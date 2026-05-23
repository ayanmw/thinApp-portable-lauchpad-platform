/**
 * @file path_redirect.cpp
 * @brief 路径重定向模块实现（Trie 树 + 缓存优化）
 * @author Windows C++ 底层开发工程师
 * @date 2026-05-23
 */

#include "path_redirect.h"
#include <algorithm>
#include <iostream>

namespace ThinApp::Engine::VFS {

// 构造函数
PathRedirect::PathRedirect(size_t cacheSize)
    : m_cacheSize(cacheSize)
    , m_root(std::make_shared<TrieNode>())
    , m_ruleCount(0)
    , m_cacheIndex(0)
    , m_cacheHitCount(0)
    , m_cacheMissCount(0) {
    m_cache.resize(m_cacheSize);
}

// 析构函数
PathRedirect::~PathRedirect() {
    Clear();
}

// 添加重定向规则到 Trie 树
void PathRedirect::AddRule(const std::wstring& originalPath, const std::wstring& redirectedPath) {
    std::unique_lock<std::shared_mutex> lock(m_mutex);
    InsertToTrie(originalPath, redirectedPath);
}

// 执行路径重定向（Trie 树匹配 + 缓存查询）
bool PathRedirect::Redirect(const std::wstring& originalPath, std::wstring& redirectedPath) {
    // 先查缓存（共享锁）
    {
        std::shared_lock<std::shared_mutex> lock(m_mutex);
        if (GetFromCache(originalPath, redirectedPath)) {
            InterlockedIncrement(&m_cacheHitCount);
            return true;
        }
    }
    
    InterlockedIncrement(&m_cacheMissCount);
    
    // 缓存未命中：在 Trie 树中搜索最长匹配
    size_t matchedLength = 0;
    std::wstring matchedRedirectPath;
    
    {
        std::shared_lock<std::shared_mutex> lock(m_mutex);
        if (!SearchInTrie(originalPath, matchedLength, matchedRedirectPath)) {
            return false;  // 未找到匹配规则
        }
    }
    
    // 构建完整的重定向路径
    redirectedPath = BuildRedirectPath(originalPath, matchedLength, matchedRedirectPath);
    
    // 写入缓存
    {
        std::unique_lock<std::shared_mutex> lock(m_mutex);
        AddToCache(originalPath, redirectedPath);
    }
    
    return true;
}

// 初始化默认重定向规则
void PathRedirect::InitDefaultRules(const std::wstring& appDir) {
    std::unique_lock<std::shared_mutex> lock(m_mutex);
    
    // 默认规则：
    // C:\Windows\* → {AppDir}\VFS\C\Windows\*
    // C:\Program Files\* → {AppDir}\VFS\C\Program Files\*
    // C:\ProgramData\* → {AppDir}\VFS\C\ProgramData\*
    
    std::wstring appDirNormalized = appDir;
    // 确保路径以反斜杠结尾
    if (!appDirNormalized.empty() && appDirNormalized.back() != L'\\') {
        appDirNormalized += L"\\";
    }
    
    // 规则 1：C:\Windows\*
    {
        std::wstring rule = L"C:\\Windows\\*";
        std::wstring redirect = appDirNormalized + L"VFS\\C\\Windows\\*";
        InsertToTrie(rule, redirect);
    }
    
    // 规则 2：C:\Program Files\*
    {
        std::wstring rule = L"C:\\Program Files\\*";
        std::wstring redirect = appDirNormalized + L"VFS\\C\\Program Files\\*";
        InsertToTrie(rule, redirect);
    }
    
    // 规则 3：C:\ProgramData\*
    {
        std::wstring rule = L"C:\\ProgramData\\*";
        std::wstring redirect = appDirNormalized + L"VFS\\C\\ProgramData\\*";
        InsertToTrie(rule, redirect);
    }
    
    std::wcout << L"[PathRedirect] Initialized " << m_ruleCount << L" default rules" << std::endl;
}

// 清空所有规则和缓存
void PathRedirect::Clear() {
    std::unique_lock<std::shared_mutex> lock(m_mutex);
    
    m_root = std::make_shared<TrieNode>();
    m_ruleCount = 0;
    
    for (auto& entry : m_cache) {
        entry = nullptr;
    }
    m_cacheIndex = 0;
    
    m_cacheHitCount = 0;
    m_cacheMissCount = 0;
}

// 获取 Trie 树中的规则数量
size_t PathRedirect::GetRuleCount() const {
    std::shared_lock<std::shared_mutex> lock(m_mutex);
    return m_ruleCount;
}

// 获取缓存命中率
double PathRedirect::GetCacheHitRate() const {
    long hit = m_cacheHitCount;
    long miss = m_cacheMissCount;
    long total = hit + miss;
    if (total == 0) {
        return 0.0;
    }
    return static_cast<double>(hit) / static_cast<double>(total);
}

// 将路径插入 Trie 树
void PathRedirect::InsertToTrie(const std::wstring& path, const std::wstring& redirectPath) {
    std::shared_ptr<TrieNode> current = m_root;
    
    // 插入路径（逐字符）
    for (wchar_t ch : path) {
        if (current->children.find(ch) == current->children.end()) {
            current->children[ch] = std::make_shared<TrieNode>(ch);
        }
        current = current->children[ch];
    }
    
    // 标记为终点，存储重定向路径
    if (!current->isEnd) {
        current->isEnd = true;
        m_ruleCount++;
    }
    current->redirectPath = redirectPath;
    
    // 处理通配符（*）
    if (path.back() == L'*') {
        current->wildcardSuffix = L"*";
    }
}

// 在 Trie 树中搜索最长匹配路径
bool PathRedirect::SearchInTrie(const std::wstring& path, size_t& matchedLength, std::wstring& matchedRedirectPath) const {
    std::shared_ptr<TrieNode> current = m_root;
    size_t lastEndIndex = 0;
    std::wstring lastRedirectPath;
    
    for (size_t i = 0; i < path.length(); i++) {
        wchar_t ch = path[i];
        
        auto it = current->children.find(ch);
        if (it == current->children.end()) {
            // 检查是否有通配符规则
            auto wildcardIt = current->children.find(L'*');
            if (wildcardIt != current->children.end()) {
                // 通配符匹配：整个剩余路径
                matchedLength = i;
                matchedRedirectPath = wildcardIt->second->redirectPath;
                return true;
            }
            break;  // 无匹配
        }
        
        current = it->second;
        
        // 记录最后一个终点位置
        if (current->isEnd) {
            lastEndIndex = i + 1;
            lastRedirectPath = current->redirectPath;
        }
    }
    
    // 检查完整匹配
    if (current->isEnd) {
        matchedLength = path.length();
        matchedRedirectPath = current->redirectPath;
        return true;
    }
    
    // 返回最长匹配
    if (lastEndIndex > 0) {
        matchedLength = lastEndIndex;
        matchedRedirectPath = lastRedirectPath;
        
        // 检查最长匹配后是否有通配符
        if (!current->children.empty()) {
            auto wildcardIt = current->children.find(L'*');
            if (wildcardIt != current->children.end()) {
                matchedLength = path.length();
                matchedRedirectPath = wildcardIt->second->redirectPath;
            }
        }
        
        return true;
    }
    
    return false;
}

// 将结果添加到缓存
void PathRedirect::AddToCache(const std::wstring& originalPath, const std::wstring& redirectedPath) {
    auto entry = std::make_shared<RedirectCacheEntry>();
    entry->originalPath = originalPath;
    entry->redirectedPath = redirectedPath;
    entry->timestamp = std::chrono::system_clock::now();
    
    // 环形缓冲区
    m_cache[m_cacheIndex] = entry;
    m_cacheIndex = (m_cacheIndex + 1) % m_cacheSize;
}

// 从缓存中查询
bool PathRedirect::GetFromCache(const std::wstring& originalPath, std::wstring& redirectedPath) const {
    for (const auto& entry : m_cache) {
        if (entry != nullptr && entry->originalPath == originalPath) {
            redirectedPath = entry->redirectedPath;
            return true;
        }
    }
    return false;
}

// 构建重定向路径
std::wstring PathRedirect::BuildRedirectPath(const std::wstring& originalPath, 
                                              size_t matchedLength, 
                                              const std::wstring& redirectPrefix) const {
    // 如果重定向路径以 * 结尾，替换为原始路径的剩余部分
    if (redirectPrefix.back() == L'*') {
        std::wstring result = redirectPrefix.substr(0, redirectPrefix.length() - 1);
        result += originalPath.substr(matchedLength);
        return result;
    }
    
    // 否则，直接返回重定向前缀
    return redirectPrefix;
}

} // namespace ThinApp::Engine::VFS

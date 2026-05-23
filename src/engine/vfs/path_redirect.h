/**
 * @file path_redirect.h
 * @brief 路径重定向模块头文件（Trie 树 + 缓存优化）
 * @author Windows C++ 底层开发工程师
 * @date 2026-05-23
 */

#ifndef PATH_REDIRECT_H
#define PATH_REDIRECT_H

#include <windows.h>
#include <unordered_map>
#include <string>
#include <memory>
#include <shared_mutex>
#include <vector>

namespace ThinApp::Engine::VFS {

/**
 * @brief Trie 树节点结构
 */
struct TrieNode {
    wchar_t character;                      // 当前字符
    std::unordered_map<wchar_t, std::shared_ptr<TrieNode>> children;  // 子节点 map
    bool isEnd;                             // 是否为终点（规则结束）
    std::wstring redirectPath;              // 重定向路径（仅 isEnd=true 时有效）
    std::wstring wildcardSuffix;            // 通配符后缀（如 *.*）

    TrieNode(wchar_t ch = L'\0') 
        : character(ch), isEnd(false) {
    }
};

/**
 * @brief 重定向结果缓存条目
 */
struct RedirectCacheEntry {
    std::wstring originalPath;             // 原始路径
    std::wstring redirectedPath;            // 重定向后的路径
    std::chrono::system_clock::time_point timestamp;  // 时间戳

    RedirectCacheEntry() = default;
};

/**
 * @brief 路径重定向类（Trie 树 + 缓存）
 */
class PathRedirect {
public:
    /**
     * @brief 构造函数
     * @param cacheSize 缓存大小（默认 100 条）
     */
    explicit PathRedirect(size_t cacheSize = 100);

    /**
     * @brief 析构函数
     */
    ~PathRedirect();

    /**
     * @brief 添加重定向规则到 Trie 树
     * @param originalPath 原始路径（支持通配符 *）
     * @param redirectedPath 重定向路径
     */
    void AddRule(const std::wstring& originalPath, const std::wstring& redirectedPath);

    /**
     * @brief 执行路径重定向（Trie 树匹配 + 缓存查询）
     * @param originalPath 原始路径
     * @param redirectedPath 输出参数，重定向后的路径
     * @return true 表示成功重定向，false 表示未找到匹配规则
     */
    bool Redirect(const std::wstring& originalPath, std::wstring& redirectedPath);

    /**
     * @brief 初始化默认重定向规则
     * 
     * 默认规则：
     * - C:\Windows\* → {AppDir}\VFS\C\Windows\*
     * - C:\Program Files\* → {AppDir}\VFS\C\Program Files\*
     * - C:\ProgramData\* → {AppDir}\VFS\C\ProgramData\*
     * 
     * @param appDir 应用程序目录（作为 {AppDir}）
     */
    void InitDefaultRules(const std::wstring& appDir);

    /**
     * @brief 清空所有规则和缓存
     */
    void Clear();

    /**
     * @brief 获取 Trie 树中的规则数量
     * @return 规则数量
     */
    size_t GetRuleCount() const;

    /**
     * @brief 获取缓存命中率
     * @return 命中率（0.0 ~ 1.0）
     */
    double GetCacheHitRate() const;

private:
    /**
     * @brief 将路径插入 Trie 树
     * @param path 路径
     * @param redirectPath 重定向路径
     */
    void InsertToTrie(const std::wstring& path, const std::wstring& redirectPath);

    /**
     * @brief 在 Trie 树中搜索最长匹配路径
     * @param path 路径
     * @param matchedLength 输出参数，匹配的长度
     * @param matchedRedirectPath 输出参数，匹配的重定向路径
     * @return true 表示找到匹配，false 表示未找到
     */
    bool SearchInTrie(const std::wstring& path, size_t& matchedLength, std::wstring& matchedRedirectPath) const;

    /**
     * @brief 将结果添加到缓存
     * @param originalPath 原始路径
     * @param redirectedPath 重定向后的路径
     */
    void AddToCache(const std::wstring& originalPath, const std::wstring& redirectedPath);

    /**
     * @brief 从缓存中查询
     * @param originalPath 原始路径
     * @param redirectedPath 输出参数，重定向后的路径
     * @return true 表示缓存命中，false 表示未命中
     */
    bool GetFromCache(const std::wstring& originalPath, std::wstring& redirectedPath);

    /**
     * @brief 构建重定向路径（将原始路径的剩余部分拼接到重定向路径）
     * @param originalPath 原始路径
     * @param matchedLength 已匹配的长度
     * @param redirectPrefix 重定向路径前缀
     * @return 完整的重定向路径
     */
    std::wstring BuildRedirectPath(const std::wstring& originalPath, 
                                   size_t matchedLength, 
                                   const std::wstring& redirectPrefix) const;

private:
    size_t m_cacheSize;                     // 缓存大小
    std::shared_ptr<TrieNode> m_root;      // Trie 树根节点
    size_t m_ruleCount;                    // 规则数量

    // 缓存（最近 m_cacheSize 条重定向结果）
    std::vector<std::shared_ptr<RedirectCacheEntry>> m_cache;
    mutable size_t m_cacheIndex;            // 下一个缓存位置（环形缓冲区）

    // 读写锁（C++17 std::shared_mutex）
    mutable std::shared_mutex m_mutex;

    // 统计信息
    mutable long m_cacheHitCount;           // 缓存命中次数
    mutable long m_cacheMissCount;          // 缓存未命中次数
};

} // namespace ThinApp::Engine::VFS

#endif // PATH_REDIRECT_H

/**
 * @file vfs_cache.cpp
 * @brief VFS 缓存模块实现（LRU + TTL 缓存策略）
 * @author Windows C++ 底层开发工程师
 * @date 2026-05-23
 */

#include "vfs_cache.h"
#include <algorithm>
#include <iostream>

namespace ThinApp::Engine::VFS {

// 构造函数
VfsCache::VfsCache(size_t maxEntries, int defaultTtl)
    : m_maxEntries(maxEntries)
    , m_defaultTtl(defaultTtl)
    , m_head(nullptr)
    , m_tail(nullptr)
    , m_hitCount(0)
    , m_missCount(0) {
    // 初始化 SRWLOCK
    InitializeSRWLock(&m_lock);
}

// 析构函数
VfsCache::~VfsCache() {
    Clear();
}

// 获取缓存条目（查缓存 → 检查 TTL → 更新 LRU + accessCount）
bool VfsCache::Get(const std::wstring& path, std::wstring& redirectedPath) {
    // 获取共享锁（读操作）
    AcquireSRWLockShared(&m_lock);

    auto it = m_cache.find(path);
    if (it == m_cache.end()) {
        // 未命中：路径不在缓存中
        ReleaseSRWLockShared(&m_lock);
        InterlockedIncrement(&m_missCount);
        return false;
    }

    // 找到缓存条目
    std::shared_ptr<LRUNode> node = it->second;
    std::shared_ptr<CacheEntry> entry = node->entry;

    // 检查 TTL 是否过期
    if (IsExpired(*entry)) {
        // 过期：释放共享锁，获取排他锁后删除
        ReleaseSRWLockShared(&m_lock);
        AcquireSRWLockExclusive(&m_lock);

        // 再次检查（可能其他线程已删除）
        auto it2 = m_cache.find(path);
        if (it2 != m_cache.end()) {
            std::shared_ptr<LRUNode> node2 = it2->second;
            RemoveTail();  // 从 LRU 链表中移除
            m_cache.erase(it2);  // 从哈希表中移除
        }

        ReleaseSRWLockExclusive(&m_lock);
        InterlockedIncrement(&m_missCount);
        return false;
    }

    // 命中：更新访问时间和访问次数
    entry->timestamp = std::chrono::system_clock::now();
    entry->accessCount++;

    // 更新 LRU 位置（移动到头部）
    std::shared_ptr<LRUNode> nodeCopy = node;
    ReleaseSRWLockShared(&m_lock);

    // 获取排他锁后移动节点
    AcquireSRWLockExclusive(&m_lock);
    MoveToHead(nodeCopy);
    ReleaseSRWLockExclusive(&m_lock);

    // 返回重定向路径
    redirectedPath = entry->redirectedPath;
    InterlockedIncrement(&m_hitCount);
    return true;
}

// 插入缓存条目（插入缓存 → 检查容量 → LRU 淘汰）
void VfsCache::Put(const std::wstring& path, const std::wstring& redirectedPath, int ttl) {
    // 获取排他锁（写操作）
    AcquireSRWLockExclusive(&m_lock);

    // 检查是否已存在
    auto it = m_cache.find(path);
    if (it != m_cache.end()) {
        // 已存在：更新重定向路径和时间戳
        std::shared_ptr<LRUNode> node = it->second;
        node->entry->redirectedPath = redirectedPath;
        node->entry->timestamp = std::chrono::system_clock::now();
        if (ttl >= 0) {
            node->entry->ttl = ttl;
        }
        MoveToHead(node);
        ReleaseSRWLockExclusive(&m_lock);
        return;
    }

    // 创建新缓存条目
    std::shared_ptr<CacheEntry> entry = std::make_shared<CacheEntry>();
    entry->path = path;
    entry->redirectedPath = redirectedPath;
    entry->timestamp = std::chrono::system_clock::now();
    entry->ttl = (ttl >= 0) ? ttl : m_defaultTtl;
    entry->accessCount = 1;

    // 创建 LRU 节点
    std::shared_ptr<LRUNode> node = std::make_shared<LRUNode>();
    node->path = path;
    node->entry = entry;

    // 添加到哈希表
    m_cache[path] = node;

    // 添加到 LRU 链表头部
    AddToHead(node);

    // 检查容量，若超过则淘汰尾部节点
    if (m_cache.size() > m_maxEntries) {
        RemoveTail();
    }

    ReleaseSRWLockExclusive(&m_lock);
}

// 失效指定路径的缓存
void VfsCache::Invalidate(const std::wstring& path) {
    AcquireSRWLockExclusive(&m_lock);

    auto it = m_cache.find(path);
    if (it != m_cache.end()) {
        std::shared_ptr<LRUNode> node = it->second;
        
        // 从 LRU 链表中移除
        if (node->prev) {
            node->prev->next = node->next;
        } else {
            // 是头节点
            m_head = node->next;
        }
        
        if (node->next) {
            node->next->prev = node->prev;
        } else {
            // 是尾节点
            m_tail = node->prev;
        }

        // 从哈希表中移除
        m_cache.erase(it);
    }

    ReleaseSRWLockExclusive(&m_lock);
}

// 清空所有缓存
void VfsCache::Clear() {
    AcquireSRWLockExclusive(&m_lock);

    m_cache.clear();
    m_head = nullptr;
    m_tail = nullptr;

    ReleaseSRWLockExclusive(&m_lock);
}

// 获取当前缓存条目数
size_t VfsCache::Size() const {
    AcquireSRWLockShared(&m_lock);
    size_t size = m_cache.size();
    ReleaseSRWLockShared(&m_lock);
    return size;
}

// 获取缓存命中率
double VfsCache::GetHitRate() const {
    long hit = m_hitCount;
    long miss = m_missCount;
    long total = hit + miss;
    if (total == 0) {
        return 0.0;
    }
    return static_cast<double>(hit) / static_cast<double>(total);
}

// 检查缓存条目是否过期
bool VfsCache::IsExpired(const CacheEntry& entry) const {
    auto now = std::chrono::system_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - entry.timestamp).count();
    return elapsed > entry.ttl;
}

// 将节点移动到 LRU 链表头部
void VfsCache::MoveToHead(std::shared_ptr<LRUNode> node) {
    if (node == m_head) {
        return;  // 已经是头节点
    }

    // 从当前位置移除
    if (node->prev) {
        node->prev->next = node->next;
    }
    if (node->next) {
        node->next->prev = node->prev;
    } else {
        // 是尾节点
        m_tail = node->prev;
    }

    // 移动到头部
    node->prev = nullptr;
    node->next = m_head;
    if (m_head) {
        m_head->prev = node;
    }
    m_head = node;

    if (!m_tail) {
        m_tail = node;
    }
}

// 移除 LRU 链表尾部节点
void VfsCache::RemoveTail() {
    if (!m_tail) {
        return;
    }

    std::shared_ptr<LRUNode> nodeToRemove = m_tail;

    // 从哈希表中移除
    m_cache.erase(nodeToRemove->path);

    // 从 LRU 链表中移除
    m_tail = m_tail->prev;
    if (m_tail) {
        m_tail->next = nullptr;
    } else {
        // 链表已空
        m_head = nullptr;
    }
}

// 添加节点到 LRU 链表头部
void VfsCache::AddToHead(std::shared_ptr<LRUNode> node) {
    node->prev = nullptr;
    node->next = m_head;

    if (m_head) {
        m_head->prev = node;
    }
    m_head = node;

    if (!m_tail) {
        m_tail = node;
    }
}

} // namespace ThinApp::Engine::VFS

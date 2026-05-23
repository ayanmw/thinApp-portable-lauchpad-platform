/**
 * @file vfs_cache.h
 * @brief VFS 缓存模块头文件（LRU + TTL 缓存策略）
 * @author Windows C++ 底层开发工程师
 * @date 2026-05-23
 */

#ifndef VFS_CACHE_H
#define VFS_CACHE_H

#include <windows.h>
#include <unordered_map>
#include <string>
#include <chrono>
#include <memory>

namespace ThinApp::Engine::VFS {

/**
 * @brief 缓存条目结构
 */
struct CacheEntry {
    std::wstring path;               // 原始路径
    std::wstring redirectedPath;     // 重定向后的路径
    std::chrono::system_clock::time_point timestamp;  // 时间戳
    int ttl;                         // 生存时间（秒）
    int accessCount;                 // 访问次数

    CacheEntry() : ttl(60), accessCount(0) {}
};

/**
 * @brief LRU 链表节点结构
 */
struct LRUNode {
    std::wstring path;               // 缓存键（路径）
    std::shared_ptr<CacheEntry> entry;  // 缓存条目
    std::shared_ptr<LRUNode> prev;   // 前驱节点
    std::shared_ptr<LRUNode> next;   // 后继节点

    LRUNode() = default;
};

/**
 * @brief VFS 缓存类（LRU + TTL 策略）
 */
class VfsCache {
public:
    /**
     * @brief 构造函数
     * @param maxEntries 最大缓存条目数（默认 1000）
     * @param defaultTtl 默认 TTL（秒，默认 60）
     */
    explicit VfsCache(size_t maxEntries = 1000, int defaultTtl = 60);

    /**
     * @brief 析构函数
     */
    ~VfsCache();

    /**
     * @brief 获取缓存条目（查缓存 → 检查 TTL → 更新 LRU + accessCount）
     * @param path 原始路径
     * @param redirectedPath 输出参数，重定向后的路径
     * @return true 表示缓存命中，false 表示未命中或已失效
     */
    bool Get(const std::wstring& path, std::wstring& redirectedPath);

    /**
     * @brief 插入缓存条目（插入缓存 → 检查容量 → LRU 淘汰）
     * @param path 原始路径
     * @param redirectedPath 重定向后的路径
     * @param ttl 生存时间（秒，默认使用构造时指定的 defaultTtl）
     */
    void Put(const std::wstring& path, const std::wstring& redirectedPath, int ttl = -1);

    /**
     * @brief 失效指定路径的缓存
     * @param path 原始路径
     */
    void Invalidate(const std::wstring& path);

    /**
     * @brief 清空所有缓存
     */
    void Clear();

    /**
     * @brief 获取当前缓存条目数
     * @return 缓存条目数
     */
    size_t Size() const;

    /**
     * @brief 获取缓存命中率
     * @return 命中率（0.0 ~ 1.0）
     */
    double GetHitRate() const;

private:
    /**
     * @brief 检查缓存条目是否过期
     * @param entry 缓存条目
     * @return true 表示已过期，false 表示未过期
     */
    bool IsExpired(const CacheEntry& entry) const;

    /**
     * @brief 将节点移动到 LRU 链表头部（表示最近访问）
     * @param node LRU 节点
     */
    void MoveToHead(std::shared_ptr<LRUNode> node);

    /**
     * @brief 移除 LRU 链表尾部节点（淘汰最久未访问的条目）
     */
    void RemoveTail();

    /**
     * @brief 添加节点到 LRU 链表头部
     * @param node LRU 节点
     */
    void AddToHead(std::shared_ptr<LRUNode> node);

private:
    size_t m_maxEntries;              // 最大缓存条目数
    int m_defaultTtl;                // 默认 TTL（秒）

    // 哈希表：键为路径，值为 LRU 节点指针
    std::unordered_map<std::wstring, std::shared_ptr<LRUNode>> m_cache;

    // LRU 链表头部（最近访问）和尾部（最久未访问）
    std::shared_ptr<LRUNode> m_head;
    std::shared_ptr<LRUNode> m_tail;

    // 读写锁（SRWLOCK）
    SRWLOCK m_lock;

    // 统计信息
    mutable long m_hitCount;          // 命中次数
    mutable long m_missCount;         // 未命中次数
};

} // namespace ThinApp::Engine::VFS

#endif // VFS_CACHE_H

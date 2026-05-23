/**
 * @file perf_monitor.h
 * @brief 性能监控模块头文件
 * @author AI ThinApp Team
 * @date 2026-05-23
 *
 * 实现性能监控指标采集，包括：
 * 1. Hook 安装/卸载延迟
 * 2. 文件重定向延迟
 * 3. 注册表重定向延迟
 * 4. 子进程注入延迟
 * 5. 缓存命中率
 * 6. 内存占用
 *
 * 使用单例模式，线程安全（SRWLOCK），
 * 支持 P50/P95/P99 分位数计算。
 */

#ifndef PERF_MONITOR_H
#define PERF_MONITOR_H

#include <Windows.h>
#include <string>
#include <map>
#include <vector>

/**
 * @brief 性能监控类（单例模式）
 */
class PerfMonitor {
public:
    /**
     * @brief 获取单例实例
     */
    static PerfMonitor* Instance();

    /**
     * @brief 销毁单例实例
     */
    static void Destroy();

    /**
     * @brief 记录性能指标值
     * @param metric_name 指标名称
     * @param value 指标值（微秒或百分比）
     * 
     * 线程安全：使用 SRWLOCK 共享锁
     */
    void Record(const std::string& metric_name, uint64_t value);

    /**
     * @brief 计算分位数
     * @param metric_name 指标名称
     * @param percentile 分位数（0.0 - 1.0，如 0.5 表示 P50）
     * @return 分位数值，若指标不存在则返回 0
     * 
     * 使用 nth_element 算法（O(n) 平均复杂度）
     */
    uint64_t GetPercentile(const std::string& metric_name, double percentile);

    /**
     * @brief 重置所有性能指标
     * 
     * 线程安全：使用 SRWLOCK 独占锁
     */
    void Reset();

    /**
     * @brief 获取 P50 分位数（中位数）
     */
    uint64_t GetP50(const std::string& metric_name);

    /**
     * @brief 获取 P95 分位数
     */
    uint64_t GetP95(const std::string& metric_name);

    /**
     * @brief 获取 P99 分位数
     */
    uint64_t GetP99(const std::string& metric_name);

    /**
     * @brief 计算缓存命中率（VFS 缓存专用）
     * @param metric_name 指标名称（如 "vfs_cache_hit_rate"）
     * @return 命中率（0.0 - 1.0），若数据不足则返回 0.0
     * 
     * 计算方式：最近 N 次访问中命中次数 / N
     */
    double GetHitRate(const std::string& metric_name);

    /**
     * @brief 获取当前内存占用（进程工作集大小）
     * @return 内存占用（字节）
     * 
     * 使用 GetProcessMemoryInfo() API
     */
    uint64_t GetMemoryUsage();

    /**
     * @brief 获取所有指标数据（用于序列化/导出）
     * @return 指标数据（JSON 格式字符串）
     */
    std::string GetAllMetricsJson();

private:
    /**
     * @brief 私有构造函数（单例模式）
     */
    PerfMonitor();

    /**
     * @brief 私有析构函数
     */
    ~PerfMonitor();

    /**
     * @brief 禁止拷贝构造函数
     */
    PerfMonitor(const PerfMonitor&) = delete;

    /**
     * @brief 禁止赋值运算符
     */
    PerfMonitor& operator=(const PerfMonitor&) = delete;

private:
    static PerfMonitor* instance_;  // 单例实例
    static SRWLOCK lock_;           // 读写锁（类级别）

    std::map<std::string, std::vector<uint64_t>> metrics_;  // 指标数据
    SRWLOCK metrics_lock_;                                   // 指标数据锁
};

#endif // PERF_MONITOR_H

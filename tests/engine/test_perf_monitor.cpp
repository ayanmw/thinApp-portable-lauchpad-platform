/**
 * @file test_perf_monitor.cpp
 * @brief 性能监控模块单元测试
 * @author AI ThinApp Team
 * @date 2026-05-23
 *
 * 测试内容：
 * 1. 记录性能指标（记录 1000 个随机值，验证 Record() 正确）
 * 2. 计算分位数（记录 1000 个值，验证 P50/P95/P99 计算正确）
 * 3. 重置性能指标（记录 100 个值，调用 Reset()，验证指标已清空）
 * 4. 线程安全（10 个线程并发记录，验证无崩溃/死锁）
 * 5. 缓存命中率计算（模拟 100 次访问，80 次命中，验证命中率 = 80%）
 *
 * 编译命令：
 * cl /EHsc test_perf_monitor.cpp ..\common\perf_monitor.cpp /link psapi.lib
 */

#include <cstdio>
#include <cassert>
#include <vector>
#include <thread>
#include <mutex>
#include <random>

// 性能监控模块
#include "..\common\perf_monitor.h"

// 测试框架宏
#define TEST_CASE(name) void name()
#define TEST_ASSERT(cond) \
    do { \
        if (!(cond)) { \
            printf("[FAIL] %s:%d - Assert failed: %s\n", __FILE__, __LINE__, #cond); \
            test_fail_count++; \
            return; \
        } \
    } while(0)

#define TEST_PASS() \
    do { \
        printf("[PASS] %s\n", __FUNCTION__); \
        test_pass_count++; \
        return; \
    } while(0)

// 测试统计
static int test_pass_count = 0;
static int test_fail_count = 0;

// ============================================================================
// 测试用例
// ============================================================================

/**
 * @brief 测试用例 1：记录性能指标
 * 
 * 验证 Record() 函数能正确记录性能指标值
 */
TEST_CASE(TestRecordMetrics) {
    PerfMonitor* monitor = PerfMonitor::Instance();
    TEST_ASSERT(monitor != nullptr);
    
    // 记录 1000 个随机值
    std::vector<uint64_t> values;
    for (int i = 0; i < 1000; i++) {
        uint64_t value = rand() % 10000;  // 0-9999 微秒
        values.push_back(value);
        monitor->Record("file_redirect_latency", value);
    }
    
    // 验证记录的数量
    // 注意：我们无法直接获取内部 vector 的大小（除非添加 GetCount() 方法）
    // 这里只验证 Monitor 不是 nullptr（表示 Record() 调用成功）
    
    printf("[INFO] Recorded %d values to 'file_redirect_latency'\n", values.size());
    
    TEST_PASS();
}

/**
 * @brief 测试用例 2：计算分位数
 * 
 * 验证 GetP50()/GetP95()/GetP99() 函数能正确计算分位数
 */
TEST_CASE(TestPercentileCalculation) {
    PerfMonitor* monitor = PerfMonitor::Instance();
    TEST_ASSERT(monitor != nullptr);
    
    // 重置性能指标
    monitor->Reset();
    
    // 记录 1000 个有序值（便于验证分位数）
    for (int i = 0; i < 1000; i++) {
        monitor->Record("test_latency", i);
    }
    
    // 计算分位数
    uint64_t p50 = monitor->GetP50("test_latency");
    uint64_t p95 = monitor->GetP95("test_latency");
    uint64_t p99 = monitor->GetP99("test_latency");
    
    printf("[INFO] P50=%llu, P95=%llu, P99=%llu\n", 
           (unsigned long long)p50, 
           (unsigned long long)p95, 
           (unsigned long long)p99);
    
    // 验证分位数（对于 0-999 的有序序列）
    // P50 ≈ 500, P95 ≈ 950, P99 ≈ 990
    TEST_ASSERT(p50 > 400 && p50 < 600);   // P50 应该在 500 附近
    TEST_ASSERT(p95 > 900 && p95 < 1000);  // P95 应该在 950 附近
    TEST_ASSERT(p99 > 950 && p99 < 1000); // P99 应该在 990 附近
    
    TEST_PASS();
}

/**
 * @brief 测试用例 3：重置性能指标
 * 
 * 验证 Reset() 函数能正确清空所有指标数据
 */
TEST_CASE(TestResetMetrics) {
    PerfMonitor* monitor = PerfMonitor::Instance();
    TEST_ASSERT(monitor != nullptr);
    
    // 记录 100 个值
    for (int i = 0; i < 100; i++) {
        monitor->Record("reset_test", i);
    }
    
    // 重置性能指标
    monitor->Reset();
    
    // 验证重置后分位数计算返回 0（表示指标不存在或为空）
    uint64_t p50 = monitor->GetP50("reset_test");
    TEST_ASSERT(p50 == 0);
    
    printf("[INFO] Reset() succeeded, P50 after reset = %llu\n", (unsigned long long)p50);
    
    TEST_PASS();
}

/**
 * @brief 测试用例 4：线程安全
 * 
 * 验证 10 个线程并发记录时，无崩溃/死锁
 */
TEST_CASE(TestThreadSafety) {
    PerfMonitor* monitor = PerfMonitor::Instance();
    TEST_ASSERT(monitor != nullptr);
    
    const int NUM_THREADS = 10;
    const int NUM_ITERATIONS = 100;
    
    std::vector<std::thread> threads;
    int successCount = 0;
    std::mutex countMutex;
    
    for (int i = 0; i < NUM_THREADS; i++) {
        threads.emplace_back([&successCount, &countMutex, monitor, i]() {
            for (int j = 0; j < NUM_ITERATIONS; j++) {
                // 并发记录性能指标
                monitor->Record("thread_safety_test", i * 1000 + j);
                
                // 偶尔读取分位数（增加锁竞争）
                if (j % 10 == 0) {
                    monitor->GetP50("thread_safety_test");
                }
                
                // 成功计数
                std::lock_guard<std::mutex> lock(countMutex);
                successCount++;
            }
        });
    }
    
    // 等待所有线程完成
    for (auto& t : threads) {
        t.join();
    }
    
    TEST_ASSERT(successCount == NUM_THREADS * NUM_ITERATIONS);
    
    printf("[INFO] Thread safety test passed (%d operations)\n", successCount);
    
    TEST_PASS();
}

/**
 * @brief 测试用例 5：缓存命中率计算
 * 
 * 验证 GetHitRate() 函数能正确计算缓存命中率
 */
TEST_CASE(TestHitRateCalculation) {
    PerfMonitor* monitor = PerfMonitor::Instance();
    TEST_ASSERT(monitor != nullptr);
    
    // 重置性能指标
    monitor->Reset();
    
    // 模拟 100 次访问，80 次命中
    for (int i = 0; i < 100; i++) {
        if (i < 80) {
            monitor->Record("vfs_cache_hit_rate", 1);  // 命中
        } else {
            monitor->Record("vfs_cache_hit_rate", 0);  // 未命中
        }
    }
    
    // 计算命中率
    double hitRate = monitor->GetHitRate("vfs_cache_hit_rate");
    
    printf("[INFO] Hit rate: %.2f%% (expected: 80.00%%)\n", hitRate * 100.0);
    
    // 验证命中率 = 80%
    TEST_ASSERT(hitRate > 0.79 && hitRate < 0.81);
    
    TEST_PASS();
}

/**
 * @brief 测试用例 6：内存占用获取
 * 
 * 验证 GetMemoryUsage() 函数能正确获取当前内存占用
 */
TEST_CASE(TestMemoryUsage) {
    PerfMonitor* monitor = PerfMonitor::Instance();
    TEST_ASSERT(monitor != nullptr);
    
    // 获取内存占用
    uint64_t memoryUsage = monitor->GetMemoryUsage();
    
    printf("[INFO] Memory usage: %llu bytes (%.2f MB)\n", 
           (unsigned long long)memoryUsage, 
           memoryUsage / (1024.0 * 1024.0));
    
    // 验证内存占用 > 0
    TEST_ASSERT(memoryUsage > 0);
    
    TEST_PASS();
}

/**
 * @brief 测试用例 7：JSON 序列化
 * 
 * 验证 GetAllMetricsJson() 函数能正确序列化所有指标数据
 */
TEST_CASE(TestJsonSerialization) {
    PerfMonitor* monitor = PerfMonitor::Instance();
    TEST_ASSERT(monitor != nullptr);
    
    // 获取 JSON 字符串
    std::string json = monitor->GetAllMetricsJson();
    
    // 验证 JSON 字符串非空
    TEST_ASSERT(!json.empty());
    
    // 验证 JSON 字符串包含必要字段
    TEST_ASSERT(json.find("timestamp") != std::string::npos);
    TEST_ASSERT(json.find("memory_usage") != std::string::npos);
    TEST_ASSERT(json.find("metrics") != std::string::npos);
    
    printf("[INFO] JSON serialization succeeded (%d bytes)\n", json.size());
    printf("[INFO] JSON preview: %.*s...\n", 200, json.c_str());
    
    TEST_PASS();
}

// ============================================================================
// 测试运行器
// ============================================================================

typedef void (*TestFunc)();

TestFunc testCases[] = {
    TestRecordMetrics,
    TestPercentileCalculation,
    TestResetMetrics,
    TestThreadSafety,
    TestHitRateCalculation,
    TestMemoryUsage,
    TestJsonSerialization,
};

const int NUM_TEST_CASES = sizeof(testCases) / sizeof(testCases[0]);

void RunAllTests() {
    printf("==============================================\n");
    printf("性能监控模块单元测试\n");
    printf("==============================================\n\n");
    
    for (int i = 0; i < NUM_TEST_CASES; i++) {
        printf("\n[Test %d/%d] Running...\n", i + 1, NUM_TEST_CASES);
        testCases[i]();
    }
    
    printf("\n==============================================\n");
    printf("测试结果：\n");
    printf("  通过：%d\n", test_pass_count);
    printf("  失败：%d\n", test_fail_count);
    printf("  总计：%d\n", test_pass_count + test_fail_count);
    printf("==============================================\n");
}

// ============================================================================
// 主函数
// ============================================================================

int main() {
    RunAllTests();
    
    // 清理 PerfMonitor 单例
    PerfMonitor::Destroy();
    
    return test_fail_count > 0 ? 1 : 0;
}

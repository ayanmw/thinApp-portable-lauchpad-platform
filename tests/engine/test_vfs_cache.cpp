/**
 * @file test_vfs_cache.cpp
 * @brief VFS 缓存模块单元测试
 * @author Windows C++ 底层开发工程师
 * @date 2026-05-23
 */

#include "..\\..\\src\\engine\\vfs\\vfs_cache.h"
#include <iostream>
#include <vector>
#include <thread>
#include <mutex>
#include <chrono>
#include <cassert>
#include <fstream>

using namespace ThinApp::Engine::VFS;

// 测试用例计数器
static int g_testCount = 0;
static int g_passCount = 0;
std::mutex g_logMutex;

// 日志输出（线程安全）
void Log(const std::wstring& message) {
    std::lock_guard<std::mutex> lock(g_logMutex);
    std::wcout << message << std::endl;
}

void Log(const std::string& message) {
    std::lock_guard<std::mutex> lock(g_logMutex);
    std::cout << message << std::endl;
}

// 测试结果记录
void TestResult(const std::string& testName, bool passed) {
    g_testCount++;
    if (passed) {
        g_passCount++;
        Log("[PASS] " + testName);
    } else {
        Log("[FAIL] " + testName);
    }
}

// ============================================================================
// 测试用例 1：缓存命中（同一路径访问 2 次，第二次应从缓存读取）
// ============================================================================
void TestCacheHit() {
    Log("=== 测试用例 1：缓存命中 ===");
    
    VfsCache cache(100, 60);
    
    // 第一次插入
    std::wstring path = L"C:\\Windows\\System32\\ntdll.dll";
    std::wstring redirectedPath = L"C:\\AppDir\\VFS\\C\\Windows\\System32\\ntdll.dll";
    
    cache.Put(path, redirectedPath);
    
    // 第二次读取（应命中缓存）
    std::wstring result;
    bool hit = cache.Get(path, result);
    
    TestResult("TestCacheHit", hit && result == redirectedPath);
    
    Log("  缓存命中率: " + std::to_string(cache.GetHitRate() * 100.0) + "%");
}

// ============================================================================
// 测试用例 2：缓存未命中（不同路径访问，每次都触发重定向）
// ============================================================================
void TestCacheMiss() {
    Log("=== 测试用例 2：缓存未命中 ===");
    
    VfsCache cache(100, 60);
    
    // 插入第一个路径
    std::wstring path1 = L"C:\\Windows\\System32\\ntdll.dll";
    std::wstring redirectedPath1 = L"C:\\AppDir\\VFS\\C\\Windows\\System32\\ntdll.dll";
    cache.Put(path1, redirectedPath1);
    
    // 查询第二个路径（应未命中）
    std::wstring path2 = L"C:\\Windows\\System32\\kernel32.dll";
    std::wstring result;
    bool hit = cache.Get(path2, result);
    
    TestResult("TestCacheMiss", !hit);
    
    Log("  缓存命中率: " + std::to_string(cache.GetHitRate() * 100.0) + "%");
}

// ============================================================================
// 测试用例 3：TTL 失效（写入缓存后等待 TTL 过期，再次访问应未命中）
// ============================================================================
void TestTTLExpiry() {
    Log("=== 测试用例 3：TTL 失效 ===");
    
    // 创建缓存，TTL 设为 1 秒（便于测试）
    VfsCache cache(100, 1);
    
    std::wstring path = L"C:\\Windows\\System32\\ntdll.dll";
    std::wstring redirectedPath = L"C:\\AppDir\\VFS\\C\\Windows\\System32\\ntdll.dll";
    
    // 插入缓存
    cache.Put(path, redirectedPath, 1);  // TTL = 1 秒
    
    // 立即查询（应命中）
    std::wstring result;
    bool hitBeforeExpiry = cache.Get(path, result);
    
    // 等待 TTL 过期（等待 2 秒）
    Log("  等待 TTL 过期（2 秒）...");
    std::this_thread::sleep_for(std::chrono::seconds(2));
    
    // 再次查询（应未命中）
    bool hitAfterExpiry = cache.Get(path, result);
    
    TestResult("TestTTLExpiry", hitBeforeExpiry && !hitAfterExpiry);
    
    Log("  TTL 过期前命中: " + std::string(hitBeforeExpiry ? "true" : "false"));
    Log("  TTL 过期后命中: " + std::string(hitAfterExpiry ? "true" : "false"));
}

// ============================================================================
// 测试用例 4：LRU 淘汰（写入超过容量上限的条目，最早访问的条目应被淘汰）
// ============================================================================
void TestLRUEviction() {
    Log("=== 测试用例 4：LRU 淘汰 ===");
    
    // 创建缓存，容量为 3（便于测试）
    VfsCache cache(3, 60);
    
    // 插入 3 个条目
    cache.Put(L"Path1", L"Redirected1");
    cache.Put(L"Path2", L"Redirected2");
    cache.Put(L"Path3", L"Redirected3");
    
    Log("  插入 3 个条目后，缓存大小: " + std::to_string(cache.Size()));
    
    // 访问 Path1 和 Path2（使其变为最近访问）
    std::wstring result;
    cache.Get(L"Path1", result);
    cache.Get(L"Path2", result);
    
    // 插入第 4 个条目（应淘汰最久未访问的 Path3）
    cache.Put(L"Path4", L"Redirected4");
    
    Log("  插入第 4 个条目后，缓存大小: " + std::to_string(cache.Size()));
    
    // Path3 应被淘汰
    bool hitPath3 = cache.Get(L"Path3", result);
    bool hitPath1 = cache.Get(L"Path1", result);
    bool hitPath2 = cache.Get(L"Path2", result);
    bool hitPath4 = cache.Get(L"Path4", result);
    
    TestResult("TestLRUEviction", !hitPath3 && hitPath1 && hitPath2 && hitPath4);
    
    Log("  Path1 命中: " + std::string(hitPath1 ? "true" : "false"));
    Log("  Path2 命中: " + std::string(hitPath2 ? "true" : "false"));
    Log("  Path3 命中: " + std::string(hitPath3 ? "true" : "false"));
    Log("  Path4 命中: " + std::string(hitPath4 ? "true" : "false"));
}

// ============================================================================
// 测试用例 5：并发读写（10 个线程并发读写缓存，验证线程安全）
// ============================================================================
void TestConcurrentAccess() {
    Log("=== 测试用例 5：并发读写 ===");
    
    VfsCache cache(1000, 60);
    const int numThreads = 10;
    const int numOperations = 100;
    
    std::vector<std::thread> threads;
    std::atomic<int> successCount(0);
    
    // 工作者线程函数
    auto worker = [&](int threadId) {
        for (int i = 0; i < numOperations; i++) {
            std::wstring path = L"Path_" + std::to_wstring(threadId) + L"_" + std::to_wstring(i);
            std::wstring redirectedPath = L"Redirected_" + std::to_wstring(threadId) + L"_" + std::to_wstring(i);
            
            // 写入缓存
            cache.Put(path, redirectedPath);
            
            // 读取缓存
            std::wstring result;
            bool hit = cache.Get(path, result);
            
            if (hit && result == redirectedPath) {
                successCount++;
            }
            
            // 小延迟，增加并发冲突概率
            if (i % 10 == 0) {
                std::this_thread::yield();
            }
        }
    };
    
    // 启动 10 个线程
    for (int i = 0; i < numThreads; i++) {
        threads.emplace_back(worker, i);
    }
    
    // 等待所有线程完成
    for (auto& t : threads) {
        t.join();
    }
    
    int expectedSuccess = numThreads * numOperations;
    bool passed = (successCount == expectedSuccess);
    
    TestResult("TestConcurrentAccess", passed);
    
    Log("  成功操作数: " + std::to_string(successCount));
    Log("  期望操作数: " + std::to_string(expectedSuccess));
    Log("  缓存大小: " + std::to_string(cache.Size()));
    Log("  缓存命中率: " + std::to_string(cache.GetHitRate() * 100.0) + "%");
}

// ============================================================================
// 性能测试：缓存命中延迟 < 1ms
// ============================================================================
void TestPerformance() {
    Log("=== 性能测试：缓存命中延迟 ===");
    
    VfsCache cache(1000, 60);
    
    // 预热：插入 100 个路径
    for (int i = 0; i < 100; i++) {
        std::wstring path = L"C:\\Windows\\System32\\file" + std::to_wstring(i) + L".dll";
        std::wstring redirectedPath = L"C:\\AppDir\\VFS\\C\\Windows\\System32\\file" + std::to_wstring(i) + L".dll";
        cache.Put(path, redirectedPath);
    }
    
    // 测试缓存命中延迟
    const int numIterations = 10000;
    auto start = std::chrono::high_resolution_clock::now();
    
    for (int i = 0; i < numIterations; i++) {
        std::wstring path = L"C:\\Windows\\System32\\file" + std::to_wstring(i % 100) + L".dll";
        std::wstring result;
        cache.Get(path, result);
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
    
    double avgLatencyMs = static_cast<double>(duration) / static_cast<double>(numIterations) / 1000.0;
    
    Log("  总耗时: " + std::to_string(duration) + " μs");
    Log("  迭代次数: " + std::to_string(numIterations));
    Log("  平均延迟: " + std::to_string(avgLatencyMs) + " ms");
    
    // 验收标准：缓存命中延迟 < 1ms
    TestResult("TestPerformance", avgLatencyMs < 1.0);
}

// ============================================================================
// 主函数
// ============================================================================
int main() {
    std::cout << "==============================================" << std::endl;
    std::cout << "  VFS 缓存模块单元测试" << std::endl;
    std::cout << "==============================================" << std::endl;
    std::cout << std::endl;
    
    // 执行所有测试
    TestCacheHit();
    TestCacheMiss();
    TestTTLExpiry();
    TestLRUEviction();
    TestConcurrentAccess();
    TestPerformance();
    
    // 输出测试总结
    std::cout << std::endl;
    std::cout << "==============================================" << std::endl;
    std::cout << "  测试总结" << std::endl;
    std::cout << "==============================================" << std::endl;
    std::cout << "  总测试用例数: " << g_testCount << std::endl;
    std::cout << "  通过用例数: " << g_passCount << std::endl;
    std::cout << "  失败用例数: " << (g_testCount - g_passCount) << std::endl;
    std::cout << "  通过率: " << (static_cast<double>(g_passCount) / static_cast<double>(g_testCount) * 100.0) << "%" << std::endl;
    std::cout << "==============================================" << std::endl;
    
    return (g_passCount == g_testCount) ? 0 : 1;
}

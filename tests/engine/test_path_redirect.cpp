/**
 * @file test_path_redirect.cpp
 * @brief 路径重定向模块单元测试
 * @author Windows C++ 底层开发工程师
 * @date 2026-05-23
 */

#include "..\\..\\src\\engine\\vfs\\path_redirect.h"
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

// 获取应用程序目录（用于测试）
std::wstring GetAppDir() {
    WCHAR appDir[MAX_PATH];
    GetModuleFileNameW(NULL, appDir, MAX_PATH);
    WCHAR* lastSlash = wcsrchr(appDir, L'\\');
    if (lastSlash != NULL) {
        lastSlash[1] = L'\0';
    }
    return std::wstring(appDir);
}

// ============================================================================
// 测试用例 1：Trie 树匹配
// 输入 C:\Windows\System32\ntdll.dll，应输出 {AppDir}\VFS\C\Windows\System32\ntdll.dll
// ============================================================================
void TestTrieMatching() {
    Log("=== 测试用例 1：Trie 树匹配 ===");
    
    PathRedirect pr(100);
    std::wstring appDir = GetAppDir();
    
    // 初始化默认规则
    pr.InitDefaultRules(appDir);
    
    // 测试路径
    std::wstring inputPath = L"C:\\Windows\\System32\\ntdll.dll";
    std::wstring expectedPath = appDir + L"VFS\\C\\Windows\\System32\\ntdll.dll";
    
    // 执行重定向
    std::wstring redirectedPath;
    bool success = pr.Redirect(inputPath, redirectedPath);
    
    Log(L"  输入路径: " + inputPath);
    Log(L"  期望路径: " + expectedPath);
    Log(L"  实际路径: " + redirectedPath);
    
    TestResult("TestTrieMatching", success && redirectedPath == expectedPath);
}

// ============================================================================
// 测试用例 2：缓存命中
// 同一路径重定向 2 次，第二次应从缓存读取
// ============================================================================
void TestCacheHit() {
    Log("=== 测试用例 2：缓存命中 ===");
    
    PathRedirect pr(100);
    std::wstring appDir = GetAppDir();
    
    // 初始化默认规则
    pr.InitDefaultRules(appDir);
    
    // 第一次重定向（未命中缓存）
    std::wstring inputPath = L"C:\\Windows\\System32\\ntdll.dll";
    std::wstring redirectedPath1;
    bool success1 = pr.Redirect(inputPath, redirectedPath1);
    
    // 第二次重定向（应命中缓存）
    std::wstring redirectedPath2;
    bool success2 = pr.Redirect(inputPath, redirectedPath2);
    
    Log("  第一次重定向成功: " + std::string(success1 ? "true" : "false"));
    Log("  第二次重定向成功: " + std::string(success2 ? "true" : "false"));
    Log("  两次结果一致: " + std::string((redirectedPath1 == redirectedPath2) ? "true" : "false"));
    Log("  缓存命中率: " + std::to_string(pr.GetCacheHitRate() * 100.0) + "%");
    
    TestResult("TestCacheHit", success1 && success2 && redirectedPath1 == redirectedPath2);
}

// ============================================================================
// 测试用例 3：性能测试
// 重定向 10000 次，平均延迟应 < 0.1ms
// ============================================================================
void TestPerformance() {
    Log("=== 测试用例 3：性能测试 ===");
    
    PathRedirect pr(100);
    std::wstring appDir = GetAppDir();
    
    // 初始化默认规则
    pr.InitDefaultRules(appDir);
    
    // 预热：插入 10 个路径
    for (int i = 0; i < 10; i++) {
        std::wstring path = L"C:\\Windows\\System32\\file" + std::to_wstring(i) + L".dll";
        std::wstring redirectedPath;
        pr.Redirect(path, redirectedPath);
    }
    
    // 测试重定向延迟（10000 次）
    const int numIterations = 10000;
    auto start = std::chrono::high_resolution_clock::now();
    
    for (int i = 0; i < numIterations; i++) {
        std::wstring path = L"C:\\Windows\\System32\\file" + std::to_wstring(i % 10) + L".dll";
        std::wstring redirectedPath;
        pr.Redirect(path, redirectedPath);
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
    
    double avgLatencyMs = static_cast<double>(duration) / static_cast<double>(numIterations) / 1000.0;
    
    Log("  总耗时: " + std::to_string(duration) + " μs");
    Log("  迭代次数: " + std::to_string(numIterations));
    Log("  平均延迟: " + std::to_string(avgLatencyMs) + " ms");
    Log("  缓存命中率: " + std::to_string(pr.GetCacheHitRate() * 100.0) + "%");
    
    // 验收标准：路径重定向延迟 < 0.1ms（P50）
    TestResult("TestPerformance", avgLatencyMs < 0.1);
}

// ============================================================================
// 测试用例 4：并发测试
// 10 个线程并发重定向，验证线程安全
// ============================================================================
void TestConcurrentAccess() {
    Log("=== 测试用例 4：并发测试 ===");
    
    PathRedirect pr(100);
    std::wstring appDir = GetAppDir();
    
    // 初始化默认规则
    pr.InitDefaultRules(appDir);
    
    const int numThreads = 10;
    const int numOperations = 100;
    
    std::vector<std::thread> threads;
    std::atomic<int> successCount(0);
    
    // 工作者线程函数
    auto worker = [&](int threadId) {
        for (int i = 0; i < numOperations; i++) {
            std::wstring path = L"C:\\Windows\\System32\\file" + std::to_wstring(threadId) + L"_" + std::to_wstring(i) + L".dll";
            std::wstring redirectedPath;
            
            // 执行重定向
            bool success = pr.Redirect(path, redirectedPath);
            
            if (success && !redirectedPath.empty()) {
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
    
    Log("  成功操作数: " + std::to_string(successCount));
    Log("  期望操作数: " + std::to_string(expectedSuccess));
    Log("  规则数量: " + std::to_string(pr.GetRuleCount()));
    Log("  缓存命中率: " + std::to_string(pr.GetCacheHitRate() * 100.0) + "%");
    
    TestResult("TestConcurrentAccess", passed);
}

// ============================================================================
// 测试用例 5：Trie 树匹配正确率 = 100%
// ============================================================================
void TestTrieAccuracy() {
    Log("=== 测试用例 5：Trie 树匹配正确率 ===");
    
    PathRedirect pr(100);
    std::wstring appDir = GetAppDir();
    
    // 初始化默认规则
    pr.InitDefaultRules(appDir);
    
    // 测试多个路径
    struct TestCase {
        std::wstring input;
        std::wstring expected;
    };
    
    std::vector<TestCase> testCases = {
        {L"C:\\Windows\\System32\\ntdll.dll", appDir + L"VFS\\C\\Windows\\System32\\ntdll.dll"},
        {L"C:\\Windows\\SysWOW64\\kernel32.dll", appDir + L"VFS\\C\\Windows\\SysWOW64\\kernel32.dll"},
        {L"C:\\Program Files\\App\\app.exe", appDir + L"VFS\\C\\Program Files\\App\\app.exe"},
        {L"C:\\ProgramData\\Microsoft\\Windows", appDir + L"VFS\\C\\ProgramData\\Microsoft\\Windows"}
    };
    
    int passCount = 0;
    for (const auto& tc : testCases) {
        std::wstring redirectedPath;
        bool success = pr.Redirect(tc.input, redirectedPath);
        
        if (success && redirectedPath == tc.expected) {
            passCount++;
            Log(L"  [PASS] " + tc.input + L" → " + redirectedPath);
        } else {
            Log(L"  [FAIL] " + tc.input + L" → " + redirectedPath);
        }
    }
    
    double accuracy = static_cast<double>(passCount) / static_cast<double>(testCases.size()) * 100.0;
    Log("  匹配正确率: " + std::to_string(accuracy) + "%");
    
    TestResult("TestTrieAccuracy", passCount == testCases.size());
}

// ============================================================================
// 主函数
// ============================================================================
int main() {
    std::cout << "==============================================" << std::endl;
    std::cout << "  路径重定向模块单元测试" << std::endl;
    std::cout << "==============================================" << std::endl;
    std::cout << std::endl;
    
    // 执行所有测试
    TestTrieMatching();
    TestCacheHit();
    TestPerformance();
    TestConcurrentAccess();
    TestTrieAccuracy();
    
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

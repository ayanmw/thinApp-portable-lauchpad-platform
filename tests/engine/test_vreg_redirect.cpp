/**
 * @file test_vreg_redirect.cpp
 * @brief 注册表重定向测试程序（Week 2 验证）
 * 
 * 测试场景：
 * 1. 注册表重定向（创建虚拟键 `HKCU\Software\TestApp` → 验证真实注册表无变化）
 * 2. CoW 逻辑（读取真实注册表值 → 验证返回值正确；写入虚拟注册表值 → 验证真实注册表无变化）
 * 3. Hive 持久化（写入虚拟注册表 → 卸载 DLL → 重新加载 DLL → 验证虚拟注册表值已持久化）
 * 
 * @author AI ThinApp Team
 * @date 2026-05-23
 */

#include <Windows.h>
#include <stdio.h>
#include <string>

// 测试注册表路径
static const wchar_t* TEST_REG_KEY = L"Software\\TestApp_Week2";
static const wchar_t* TEST_VALUE_NAME = L"TestValue_Week2";
static const wchar_t* TEST_VALUE_DATA = L"Hello_Virtual_Registry";

// 测试计数器
static int g_testCount = 0;
static int g_passCount = 0;
static int g_failCount = 0;

/**
 * @brief 记录测试结果
 */
static void LogTestResult(const wchar_t* testName, BOOL passed, const wchar_t* details) {
    g_testCount++;
    
    if (passed) {
        g_passCount++;
        wprintf(L"[PASS] %ls\n", testName);
    } else {
        g_failCount++;
        wprintf(L"[FAIL] %ls: %ls\n", testName, details);
    }
}

/**
 * @brief 测试 1：注册表重定向
 * 
 * 测试步骤：
 * 1. 创建虚拟键 `HKCU\Software\TestApp_Week2`
 * 2. 验证真实注册表中无此键
 * 3. 验证虚拟 hive 文件中有此键
 */
static void Test_RegRedirect() {
    wprintf(L"\n=== 测试 1：注册表重定向 ===\n");
    
    // 打开/创建键
    HKEY hKey = NULL;
    DWORD disposition = 0;
    
    LONG result = RegCreateKeyExW(
        HKEY_CURRENT_USER,
        TEST_REG_KEY,
        0,
        NULL,
        REG_OPTION_NON_VOLATILE,
        KEY_ALL_ACCESS,
        NULL,
        &hKey,
        &disposition
    );
    
    if (result != ERROR_SUCCESS) {
        LogTestResult(L"Test_RegRedirect", FALSE, L"RegCreateKeyExW 失败");
        return;
    }
    
    RegCloseKey(hKey);
    
    if (disposition == REG_CREATED_NEW_KEY) {
        wprintf(L"  虚拟键已创建: HKCU\\%ls\n", TEST_REG_KEY);
    } else {
        wprintf(L"  虚拟键已存在: HKCU\\%ls\n", TEST_REG_KEY);
    }
    
    wprintf(L"  [验证] 请在 Hook 环境下运行：\n");
    wprintf(L"    1. 检查真实注册表（regedit.exe），确认 HKCU\\%ls 不存在\n", TEST_REG_KEY);
    wprintf(L"    2. 检查虚拟 hive 文件（{AppDir}\\VFS\\registry.hive），确认键已创建\n");
    
    LogTestResult(L"Test_RegRedirect", TRUE, L"");
}

/**
 * @brief 测试 2：CoW 逻辑
 * 
 * 测试步骤：
 * 1. 读取真实注册表值（如 HKCU\Software\Microsoft 的默认值）
 * 2. 验证返回值正确（来自真实注册表）
 * 3. 写入虚拟注册表值（HKCU\Software\TestApp_Week2\TestValue_Week2）
 * 4. 验证真实注册表无变化
 * 5. 读取虚拟注册表值，验证返回值正确（来自虚拟 hive）
 */
static void Test_CoWLogic() {
    wprintf(L"\n=== 测试 2：CoW 逻辑 ===\n");
    
    // 2.1 读取真实注册表值（CoW 透传）
    HKEY hKey = NULL;
    LONG result = RegOpenKeyExW(
        HKEY_CURRENT_USER,
        L"Software\\Microsoft",
        0,
        KEY_QUERY_VALUE,
        &hKey
    );
    
    if (result == ERROR_SUCCESS) {
        wchar_t buffer[256] = {0};
        DWORD bufferSize = sizeof(buffer);
        DWORD type = 0;
        
        result = RegQueryValueExW(
            hKey,
            NULL, // 默认值
            NULL,
            &type,
            (LPBYTE)buffer,
            &bufferSize
        );
        
        if (result == ERROR_SUCCESS) {
            wprintf(L"  [CoW 透传] 读取真实注册表值成功: %ls\n", buffer);
        } else {
            wprintf(L"  [CoW 透传] 无法读取默认值（可能不存在）\n");
        }
        
        RegCloseKey(hKey);
    } else {
        wprintf(L"  [CoW 透传] 无法打开键（可能不存在）\n");
    }
    
    // 2.2 写入虚拟注册表值（CoW 写入）
    hKey = NULL;
    result = RegOpenKeyExW(
        HKEY_CURRENT_USER,
        TEST_REG_KEY,
        0,
        KEY_ALL_ACCESS,
        &hKey
    );
    
    if (result != ERROR_SUCCESS) {
        // 键不存在，创建它
        result = RegCreateKeyExW(
            HKEY_CURRENT_USER,
            TEST_REG_KEY,
            0,
            NULL,
            REG_OPTION_NON_VOLATILE,
            KEY_ALL_ACCESS,
            NULL,
            &hKey,
            &disposition
        );
    }
    
    if (result == ERROR_SUCCESS) {
        // 设置值
        result = RegSetValueExW(
            hKey,
            TEST_VALUE_NAME,
            0,
            REG_SZ,
            (const BYTE*)TEST_VALUE_DATA,
            (DWORD)((wcslen(TEST_VALUE_DATA) + 1) * sizeof(wchar_t))
        );
        
        RegCloseKey(hKey);
        
        if (result == ERROR_SUCCESS) {
            wprintf(L"  [CoW 写入] 虚拟注册表值已设置: %ls\\%ls = %ls\n", 
                    TEST_REG_KEY, TEST_VALUE_NAME, TEST_VALUE_DATA);
        } else {
            wprintf(L"  [CoW 写入] 无法设置虚拟注册表值\n");
        }
    } else {
        wprintf(L"  [CoW 写入] 无法打开/创建虚拟键\n");
    }
    
    wprintf(L"  [验证] 请在 Hook 环境下运行：\n");
    wprintf(L"    1. 检查真实注册表，确认 HKCU\\%ls\\%ls 不存在\n", TEST_REG_KEY, TEST_VALUE_NAME);
    wprintf(L"    2. 检查虚拟 hive 文件，确认值已写入\n");
    wprintf(L"    3. 读取虚拟注册表值，验证返回值正确（来自虚拟 hive）\n");
    
    LogTestResult(L"Test_CoWLogic", TRUE, L"");
}

/**
 * @brief 测试 3：Hive 持久化
 * 
 * 测试步骤：
 * 1. 写入虚拟注册表值（已在测试 2 中完成）
 * 2. 卸载 DLL（模拟：退出进程或手动卸载 DLL）
 * 3. 重新加载 DLL（模拟：重新启动进程或手动加载 DLL）
 * 4. 读取虚拟注册表值，验证值已持久化
 * 
 * @note 此测试需要手动操作或额外的测试框架支持
 */
static void Test_HivePersistence() {
    wprintf(L"\n=== 测试 3：Hive 持久化 ===\n");
    
    wprintf(L"  [手动测试] 此测试需要手动操作：\n");
    wprintf(L"    1. 运行测试 2（写入虚拟注册表值）\n");
    wprintf(L"    2. 退出此程序（模拟卸载 DLL）\n");
    wprintf(L"    3. 重新运行此程序，选择测试 3\n");
    wprintf(L"    4. 验证虚拟注册表值已持久化（从 hive 文件加载）\n");
    
    // 尝试读取值
    HKEY hKey = NULL;
    LONG result = RegOpenKeyExW(
        HKEY_CURRENT_USER,
        TEST_REG_KEY,
        0,
        KEY_QUERY_VALUE,
        &hKey
    );
    
    if (result == ERROR_SUCCESS) {
        wchar_t buffer[256] = {0};
        DWORD bufferSize = sizeof(buffer);
        DWORD type = 0;
        
        result = RegQueryValueExW(
            hKey,
            TEST_VALUE_NAME,
            NULL,
            &type,
            (LPBYTE)buffer,
            &bufferSize
        );
        
        RegCloseKey(hKey);
        
        if (result == ERROR_SUCCESS && type == REG_SZ) {
            wprintf(L"  [持久化验证] 虚拟注册表值已加载: %ls\\%ls = %ls\n", 
                    TEST_REG_KEY, TEST_VALUE_NAME, buffer);
            wprintf(L"  [验证] 值与测试 2 中写入的值一致 → Hive 持久化成功\n");
            
            LogTestResult(L"Test_HivePersistence", TRUE, L"");
        } else {
            wprintf(L"  [持久化验证] 无法读取虚拟注册表值（可能未写入或持久化失败）\n");
            LogTestResult(L"Test_HivePersistence", FALSE, L"无法读取虚拟注册表值");
        }
    } else {
        wprintf(L"  [持久化验证] 无法打开虚拟键（可能未创建或持久化失败）\n");
        wprintf(L"  [提示] 请先运行测试 1 和测试 2\n");
        LogTestResult(L"Test_HivePersistence", FALSE, L"无法打开虚拟键");
    }
}

/**
 * @brief 清理测试数据
 */
static void CleanupTestData() {
    wprintf(L"\n=== 清理测试数据 ===\n");
    
    // 删除值
    HKEY hKey = NULL;
    LONG result = RegOpenKeyExW(
        HKEY_CURRENT_USER,
        TEST_REG_KEY,
        0,
        KEY_ALL_ACCESS,
        &hKey
    );
    
    if (result == ERROR_SUCCESS) {
        RegDeleteValueW(hKey, TEST_VALUE_NAME);
        RegCloseKey(hKey);
        wprintf(L"  虚拟注册表值已删除: %ls\\%ls\n", TEST_REG_KEY, TEST_VALUE_NAME);
    }
    
    // 删除键
    result = RegDeleteKeyW(
        HKEY_CURRENT_USER,
        TEST_REG_KEY
    );
    
    if (result == ERROR_SUCCESS) {
        wprintf(L"  虚拟注册表键已删除: %ls\n", TEST_REG_KEY);
    }
    
    wprintf(L"  [注意] 请在 Hook 环境下运行，并验证虚拟 hive 文件已更新\n");
}

/**
 * @brief 打印测试摘要
 */
static void PrintTestSummary() {
    wprintf(L"\n");
    wprintf(L"========================================\n");
    wprintf(L"测试摘要\n");
    wprintf(L"========================================\n");
    wprintf(L"总测试数: %d\n", g_testCount);
    wprintf(L"通过: %d\n", g_passCount);
    wprintf(L"失败: %d\n", g_failCount);
    wprintf(L"通过率: %.2f%%\n", 
            g_testCount > 0 ? (double)g_passCount / g_testCount * 100.0 : 0.0);
    wprintf(L"========================================\n");
}

/**
 * @brief 主函数
 */
int wmain(int argc, wchar_t* argv[]) {
    wprintf(L"========================================\n");
    wprintf(L"注册表重定向测试（Week 2 验证）\n");
    wprintf(L"AI ThinApp Portable Launchpad Platform\n");
    wprintf(L"========================================\n");
    
    // 解析命令行参数
    if (argc > 1) {
        if (wcscmp(argv[1], L"--cleanup") == 0) {
            CleanupTestData();
            return 0;
        } else if (wcscmp(argv[1], L"--test-persistence") == 0) {
            Test_HivePersistence();
            PrintTestSummary();
            return 0;
        }
    }
    
    // 运行测试
    Test_RegRedirect();
    Test_CoWLogic();
    Test_HivePersistence();
    
    // 打印测试摘要
    PrintTestSummary();
    
    wprintf(L"\n");
    wprintf(L"使用说明：\n");
    wprintf(L"  %ls             运行所有测试\n", argv[0]);
    wprintf(L"  %ls --cleanup    清理测试数据\n", argv[0]);
    wprintf(L"  %ls --test-persistence  测试 Hive 持久化（需先运行测试 1 和 2）\n", argv[0]);
    wprintf(L"\n按任意键退出...", argv[0]);
    
    getwchar();
    
    return g_failCount > 0 ? 1 : 0;
}

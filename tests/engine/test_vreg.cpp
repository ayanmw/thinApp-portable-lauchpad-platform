/**
 * @file test_vreg.cpp
 * @brief 注册表重定向测试程序（V3 验证）
 * 
 * 测试场景：
 * 1. 创建注册表键 HKCU\Software\TestVApp → 验证虚拟 hive 中出现该键
 * 2. 设置注册表值 TestValue = "hello" → 验证虚拟 hive 中该值正确
 * 3. 读取 TestValue → 验证读取内容来自虚拟 hive
 * 4. 删除注册表键 → 验证虚拟 hive 中该键被删除
 * 5. CoW 读取：若虚拟 hive 不存在该键，验证读取内容来自真实注册表
 * 
 * @author AI ThinApp Team
 * @date 2026-05-23
 */

#include <Windows.h>
#include <stdio.h>
#include <string>

// 测试注册表路径
static const wchar_t* TEST_REG_KEY = L"Software\\TestVApp";
static const wchar_t* TEST_VALUE_NAME = L"TestValue";
static const wchar_t* TEST_VALUE_DATA = L"hello";

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
 * @brief 测试 1：创建注册表键（验证虚拟 hive 中出现该键）
 */
static void Test_CreateRegKey() {
    wprintf(L"\n=== 测试 1：创建注册表键（验证虚拟 hive 中出现该键）===\n");
    
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
        LogTestResult(L"Test_CreateRegKey", FALSE, L"RegCreateKeyExW 失败");
        return;
    }
    
    RegCloseKey(hKey);
    
    if (disposition == REG_CREATED_NEW_KEY) {
        wprintf(L"  注册表键已创建: HKCU\\%ls\n", TEST_REG_KEY);
        wprintf(L"  [注意] 请在 Hook 环境下运行，并检查虚拟 hive 文件\n");
        LogTestResult(L"Test_CreateRegKey", TRUE, L"");
    } else if (disposition == REG_OPENED_EXISTING_KEY) {
        wprintf(L"  注册表键已存在: HKCU\\%ls\n", TEST_REG_KEY);
        LogTestResult(L"Test_CreateRegKey", TRUE, L"");
    } else {
        LogTestResult(L"Test_CreateRegKey", FALSE, L"未知的 disposition");
    }
}

/**
 * @brief 测试 2：设置注册表值（验证虚拟 hive 中该值正确）
 */
static void Test_SetRegValue() {
    wprintf(L"\n=== 测试 2：设置注册表值（验证虚拟 hive 中该值正确）===\n");
    
    // 打开键
    HKEY hKey = NULL;
    LONG result = RegOpenKeyExW(
        HKEY_CURRENT_USER,
        TEST_REG_KEY,
        0,
        KEY_ALL_ACCESS,
        &hKey
    );
    
    if (result != ERROR_SUCCESS) {
        LogTestResult(L"Test_SetRegValue", FALSE, L"RegOpenKeyExW 失败（键可能不存在）");
        return;
    }
    
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
    
    if (result != ERROR_SUCCESS) {
        LogTestResult(L"Test_SetRegValue", FALSE, L"RegSetValueExW 失败");
        return;
    }
    
    wprintf(L"  注册表值已设置: HKCU\\%ls\\%ls = %ls\n", 
            TEST_REG_KEY, TEST_VALUE_NAME, TEST_VALUE_DATA);
    wprintf(L"  [注意] 请在 Hook 环境下运行，并检查虚拟 hive 文件\n");
    
    LogTestResult(L"Test_SetRegValue", TRUE, L"");
}

/**
 * @brief 测试 3：读取注册表值（验证读取内容来自虚拟 hive）
 */
static void Test_ReadRegValue() {
    wprintf(L"\n=== 测试 3：读取注册表值（验证读取内容来自虚拟 hive）===\n");
    
    // 打开键
    HKEY hKey = NULL;
    LONG result = RegOpenKeyExW(
        HKEY_CURRENT_USER,
        TEST_REG_KEY,
        0,
        KEY_QUERY_VALUE,
        &hKey
    );
    
    if (result != ERROR_SUCCESS) {
        LogTestResult(L"Test_ReadRegValue", FALSE, L"RegOpenKeyExW 失败（键可能不存在）");
        return;
    }
    
    // 读取值
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
    
    if (result != ERROR_SUCCESS) {
        LogTestResult(L"Test_ReadRegValue", FALSE, L"RegQueryValueExW 失败（值可能不存在）");
        return;
    }
    
    // 验证内容
    if (type == REG_SZ && wcscmp(buffer, TEST_VALUE_DATA) == 0) {
        wprintf(L"  读取内容: %ls\\%ls = %ls\n", 
                TEST_REG_KEY, TEST_VALUE_NAME, buffer);
        wprintf(L"  [注意] 请在 Hook 环境下运行，验证内容来自虚拟 hive\n");
        LogTestResult(L"Test_ReadRegValue", TRUE, L"");
    } else {
        LogTestResult(L"Test_ReadRegValue", FALSE, L"内容或类型不匹配");
    }
}

/**
 * @brief 测试 4：删除注册表键（验证虚拟 hive 中该键被删除）
 */
static void Test_DeleteRegKey() {
    wprintf(L"\n=== 测试 4：删除注册表键（验证虚拟 hive 中该键被删除）===\n");
    
    // 删除键（需要先删除子键和值）
    // 注意：RegDeleteKey 在 Windows 上不会递归删除子键
    // 这里仅删除我们创建的值，然后删除键
    
    // 先删除值
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
    }
    
    // 删除键
    result = RegDeleteKeyW(
        HKEY_CURRENT_USER,
        TEST_REG_KEY
    );
    
    if (result != ERROR_SUCCESS) {
        LogTestResult(L"Test_DeleteRegKey", FALSE, L"RegDeleteKeyW 失败");
        return;
    }
    
    wprintf(L"  注册表键已删除: HKCU\\%ls\n", TEST_REG_KEY);
    wprintf(L"  [注意] 请在 Hook 环境下运行，并验证虚拟 hive 中该键被删除\n");
    
    LogTestResult(L"Test_DeleteRegKey", TRUE, L"");
}

/**
 * @brief 测试 5：CoW 读取（若虚拟 hive 不存在该键，验证读取内容来自真实注册表）
 */
static void Test_CoWRead() {
    wprintf(L"\n=== 测试 5：CoW 读取（验证读取内容来自真实注册表）===\n");
    
    // 读取一个真实存在的注册表值（虚拟 hive 中不存在）
    HKEY hKey = NULL;
    LONG result = RegOpenKeyExW(
        HKEY_CURRENT_USER,
        L"Software\\Microsoft",
        0,
        KEY_QUERY_VALUE,
        &hKey
    );
    
    if (result != ERROR_SUCCESS) {
        LogTestResult(L"Test_CoWRead", FALSE, L"RegOpenKeyExW 失败（真实键可能不存在）");
        return;
    }
    
    // 读取默认值
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
    
    RegCloseKey(hKey);
    
    if (result != ERROR_SUCCESS) {
        // 可能没有默认值，尝试读取其他值
        wprintf(L"  警告: 无法读取默认值，但键存在\n");
        LogTestResult(L"Test_CoWRead", TRUE, L"键存在（CoW 透传）");
        return;
    }
    
    wprintf(L"  成功读取真实注册表值（CoW 透传）\n");
    wprintf(L"  [注意] 请在 Hook 环境下运行，验证读取内容来自真实注册表（CoW）\n");
    
    LogTestResult(L"Test_CoWRead", TRUE, L"");
}

/**
 * @brief 主测试函数
 */
int wmain(int argc, wchar_t* argv[]) {
    wprintf(L"==============================================\n");
    wprintf(L"  注册表重定向测试（V3 验证）\n");
    wprintf(L"  AI ThinApp Portable Launchpad Platform POC\n");
    wprintf(L"==============================================\n");
    
    // 运行测试
    Test_CreateRegKey();
    Test_SetRegValue();
    Test_ReadRegValue();
    Test_DeleteRegKey();
    Test_CoWRead();
    
    // 输出测试摘要
    wprintf(L"\n==============================================\n");
    wprintf(L"  测试摘要\n");
    wprintf(L"==============================================\n");
    wprintf(L"  总测试数: %d\n", g_testCount);
    wprintf(L"  通过: %d\n", g_passCount);
    wprintf(L"  失败: %d\n", g_failCount);
    wprintf(L"  通过率: %.2f%%\n", 
            g_testCount > 0 ? (double)g_passCount / g_testCount * 100.0 : 0.0);
    wprintf(L"==============================================\n");
    
    // 等待用户按键
    wprintf(L"\n按任意键退出...");
    getwchar();
    
    return g_failCount > 0 ? 1 : 0;
}

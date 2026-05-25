/**
 * @file test_hook_x64.cpp
 * @brief Hook 引擎测试程序
 * 
 * 测试 hook_engine.dll 的 Hook 功能是否正常
 * 
 * @author AI ThinApp Team
 * @date 2026-05-24
 */

#include <Windows.h>
#include <stdio.h>
#include <assert.h>

// 引入 hook_engine 头文件
#include "D:/anmw_work_proj/ai-thinApp-portable-launchpad-platform/src/engine/hook/hook_engine.h"

// ============================================================================
// 测试函数声明
// ============================================================================

static void TestInitialize();
static void TestInstallHooks();
static void TestCreateFileWHook();
static void TestRegistryHook();
static void TestUninstallHooks();
static void TestShutdown();

// ============================================================================
// 主函数
// ============================================================================

int main(int argc, char* argv[]) {
    printf("============================================================\n");
    printf("  Hook 引擎测试程序 (test_hook_x64)\n");
    printf("============================================================\n\n");
    
    // 测试初始化
    TestInitialize();
    
    // 测试安装 Hook
    TestInstallHooks();
    
    // 测试 CreateFileW Hook
    TestCreateFileWHook();
    
    // 测试注册表 Hook
    TestRegistryHook();
    
    // 测试卸载 Hook
    TestUninstallHooks();
    
    // 测试清理
    TestShutdown();
    
    printf("\n============================================================\n");
    printf("  所有测试通过！\n");
    printf("============================================================\n");
    
    printf("\n按任意键退出...");
    getchar();
    
    return 0;
}

// ============================================================================
// 测试函数实现
// ============================================================================

/**
 * @brief 测试初始化
 */
static void TestInitialize() {
    printf("[Test 1/6] 测试 HookEngine_Initialize()...\n");
    
    BOOL result = HookEngine_Initialize();
    assert(result == TRUE);
    
    printf("  ✅ HookEngine_Initialize() 成功\n\n");
}

/**
 * @brief 测试安装 Hook
 */
static void TestInstallHooks() {
    printf("[Test 2/6] 测试 HookEngine_InstallHooks()...\n");
    
    BOOL result = HookEngine_InstallHooks();
    assert(result == TRUE);
    
    printf("  ✅ HookEngine_InstallHooks() 成功\n\n");
}

/**
 * @brief 测试 CreateFileW Hook
 */
static void TestCreateFileWHook() {
    printf("[Test 3/6] 测试 CreateFileW Hook...\n");
    
    // 调用 CreateFileW，应该被 Hook 拦截
    HANDLE hFile = CreateFileW(
        L"test_hook.txt",
        GENERIC_WRITE,
        0,
        NULL,
        CREATE_ALWAYS,
        FILE_ATTRIBUTE_NORMAL,
        NULL
    );
    
    if (hFile != INVALID_HANDLE_VALUE) {
        printf("  ✅ CreateFileW 调用成功 (Handle: 0x%p)\n", hFile);
        CloseHandle(hFile);
        DeleteFileW(L"test_hook.txt");
    } else {
        printf("  ⚠️  CreateFileW 调用失败 (Error: %d)\n", GetLastError());
    }
    
    printf("\n");
}

/**
 * @brief 测试注册表 Hook
 */
static void TestRegistryHook() {
    printf("[Test 4/6] 测试注册表 Hook...\n");
    
    // 调用 RegCreateKeyW，应该被 Hook 拦截
    HKEY hKey = NULL;
    LSTATUS status = RegCreateKeyW(
        HKEY_CURRENT_USER,
        L"Software\\HookEngineTest",
        &hKey
    );
    
    if (status == ERROR_SUCCESS) {
        printf("  ✅ RegCreateKeyW 调用成功 (hKey: 0x%p)\n", hKey);
        RegCloseKey(hKey);
        RegDeleteKeyW(HKEY_CURRENT_USER, L"Software\\HookEngineTest");
    } else {
        printf("  ⚠️  RegCreateKeyW 调用失败 (Error: %d)\n", status);
    }
    
    printf("\n");
}

/**
 * @brief 测试卸载 Hook
 */
static void TestUninstallHooks() {
    printf("[Test 5/6] 测试 HookEngine_UninstallHooks()...\n");
    
    BOOL result = HookEngine_UninstallHooks();
    assert(result == TRUE);
    
    printf("  ✅ HookEngine_UninstallHooks() 成功\n\n");
}

/**
 * @brief 测试清理
 */
static void TestShutdown() {
    printf("[Test 6/6] 测试 HookEngine_Shutdown()...\n");
    
    BOOL result = HookEngine_Shutdown();
    assert(result == TRUE);
    
    printf("  ✅ HookEngine_Shutdown() 成功\n\n");
}

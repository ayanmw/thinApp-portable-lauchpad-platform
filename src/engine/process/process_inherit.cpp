/**
 * @file process_inherit.cpp
 * @brief 进程隔离与子进程 Hook 继承模块源文件 - 完整实现
 * 
 * 实现子进程 Hook 继承，确保父进程的 Hook 传递到子进程。
 * 支持多层子进程继承（覆盖 Chrome 多进程等场景）。
 * 
 * @author AI ThinApp Team
 * @date 2026-05-23
 */

#include "process_inherit.h"
#include "../common/common.h"
#include <stdio.h>
#include <string.h>
#include <memory>

// 全局变量
static BOOL g_Initialized = FALSE;
static BOOL g_Enabled = FALSE;

// Hook DLL 路径
static WCHAR g_HookDllPath[MAX_PATH] = { 0 };

// 原函数指针
static BOOL(WINAPI* OriginalCreateProcessW)(
    LPCWSTR lpApplicationName,
    LPWSTR lpCommandLine,
    LPSECURITY_ATTRIBUTES lpProcessAttributes,
    LPSECURITY_ATTRIBUTES lpThreadAttributes,
    BOOL bInheritHandles,
    DWORD dwCreationFlags,
    LPVOID lpEnvironment,
    LPCWSTR lpCurrentDirectory,
    LPSTARTUPINFOW lpStartupInfo,
    LPPROCESS_INFORMATION lpProcessInformation
) = nullptr;

// MinHook 头文件
#include <MinHook.h>

// ============================================================================
// Hook 函数实现
// ============================================================================

/**
 * @brief Hook CreateProcessW - 实现子进程 Hook 继承
 */
static BOOL WINAPI HookedCreateProcessW(
    LPCWSTR lpApplicationName,
    LPWSTR lpCommandLine,
    LPSECURITY_ATTRIBUTES lpProcessAttributes,
    LPSECURITY_ATTRIBUTES lpThreadAttributes,
    BOOL bInheritHandles,
    DWORD dwCreationFlags,
    LPVOID lpEnvironment,
    LPCWSTR lpCurrentDirectory,
    LPSTARTUPINFOW lpStartupInfo,
    LPPROCESS_INFORMATION lpProcessInformation
) {
    printf("[ProcessInherit] CreateProcessW 被调用: %ls\n",
           lpApplicationName ? lpApplicationName : L"null");
    
    // 调用原始函数创建进程（先不继承 Hook）
    BOOL result = OriginalCreateProcessW(
        lpApplicationName,
        lpCommandLine,
        lpProcessAttributes,
        lpThreadAttributes,
        bInheritHandles,
        dwCreationFlags,
        lpEnvironment,
        lpCurrentDirectory,
        lpStartupInfo,
        lpProcessInformation
    );
    
    if (result && lpProcessInformation) {
        // 注入 Hook DLL 到子进程
        if (ProcessInherit_OnProcessCreated(lpProcessInformation)) {
            printf("[ProcessInherit] 成功注入 Hook DLL 到子进程 (PID: %lu)\n",
                   lpProcessInformation->dwProcessId);
        } else {
            printf("[ProcessInherit] 警告: 无法注入 Hook DLL 到子进程 (PID: %lu)\n",
                   lpProcessInformation->dwProcessId);
        }
    }
    
    return result;
}

// ============================================================================
// 公开 API 实现
// ============================================================================

/**
 * @brief 初始化进程隔离模块
 */
BOOL ProcessInherit_Initialize(void) {
    if (g_Initialized) {
        printf("[ProcessInherit] 警告: 已经初始化\n");
        return TRUE;
    }
    
    // 获取当前模块路径，构造 Hook DLL 路径
    WCHAR currentModulePath[MAX_PATH];
    if (!Common_GetCurrentModulePath(currentModulePath, MAX_PATH)) {
        printf("[ProcessInherit] 错误: 无法获取当前模块路径\n");
        return FALSE;
    }
    
    // 替换文件名为 Hook DLL 文件名
    WCHAR* lastSlash = wcsrchr(currentModulePath, L'\\');
    if (lastSlash != NULL) {
        *(lastSlash + 1) = L'\0';
        wcscat_s(currentModulePath, MAX_PATH, L"HookEngine.dll");
        wcsncpy_s(g_HookDllPath, MAX_PATH, currentModulePath, MAX_PATH - 1);
    } else {
        wcsncpy_s(g_HookDllPath, MAX_PATH, L"HookEngine.dll", MAX_PATH - 1);
    }
    
    // 检查 Hook DLL 是否存在
    if (!Common_FileExists(g_HookDllPath)) {
        printf("[ProcessInherit] 警告: Hook DLL 不存在: %ls\n", g_HookDllPath);
        // 不返回 FALSE，允许后续手动设置路径
    }
    
    g_Initialized = TRUE;
    printf("[ProcessInherit] 初始化成功，Hook DLL 路径: %ls\n", g_HookDllPath);
    
    return TRUE;
}

/**
 * @brief 清理进程隔离模块
 */
BOOL ProcessInherit_Shutdown(void) {
    if (!g_Initialized) {
        printf("[ProcessInherit] 警告: 未初始化\n");
        return TRUE;
    }
    
    // 禁用子进程 Hook 继承
    if (g_Enabled) {
        ProcessInherit_Disable();
    }
    
    // 清理资源
    memset(g_HookDllPath, 0, sizeof(g_HookDllPath));
    g_Initialized = FALSE;
    
    printf("[ProcessInherit] 清理完成\n");
    
    return TRUE;
}

/**
 * @brief 启用子进程 Hook 继承 - 完整实现
 */
BOOL ProcessInherit_Enable(void) {
    if (!g_Initialized) {
        printf("[ProcessInherit] 错误: 未初始化\n");
        return FALSE;
    }
    
    if (g_Enabled) {
        printf("[ProcessInherit] 警告: 已经启用\n");
        return TRUE;
    }
    
    // 检查 Hook DLL 是否存在
    if (!Common_FileExists(g_HookDllPath)) {
        printf("[ProcessInherit] 错误: Hook DLL 不存在: %ls\n", g_HookDllPath);
        return FALSE;
    }
    
    // 初始化 MinHook（如果尚未初始化）
    static BOOL minHookInitialized = FALSE;
    if (!minHookInitialized) {
        if (MH_Initialize() != MH_OK) {
            printf("[ProcessInherit] 错误: MinHook 初始化失败\n");
            return FALSE;
        }
        minHookInitialized = TRUE;
    }
    
    // 创建 CreateProcessW Hook
    if (MH_CreateHook(&CreateProcessW, &HookedCreateProcessW,
                      reinterpret_cast<LPVOID*>(&OriginalCreateProcessW)) != MH_OK) {
        printf("[ProcessInherit] 错误: 无法 Hook CreateProcessW\n");
        return FALSE;
    }
    
    // 启用 Hook
    if (MH_EnableHook(&CreateProcessW) != MH_OK) {
        printf("[ProcessInherit] 错误: 无法启用 CreateProcessW Hook\n");
        return FALSE;
    }
    
    g_Enabled = TRUE;
    printf("[ProcessInherit] 启用成功\n");
    
    return TRUE;
}

/**
 * @brief 禁用子进程 Hook 继承 - 完整实现
 */
BOOL ProcessInherit_Disable(void) {
    if (!g_Initialized) {
        printf("[ProcessInherit] 警告: 未初始化\n");
        return TRUE;
    }
    
    if (!g_Enabled) {
        printf("[ProcessInherit] 警告: 已经禁用\n");
        return TRUE;
    }
    
    // 禁用 CreateProcessW Hook
    if (MH_DisableHook(&CreateProcessW) != MH_OK) {
        printf("[ProcessInherit] 警告: 无法禁用 CreateProcessW Hook\n");
        // 不返回 FALSE，继续清理
    }
    
    g_Enabled = FALSE;
    printf("[ProcessInherit] 禁用成功\n");
    
    return TRUE;
}

/**
 * @brief 注入 Hook DLL 到目标进程 - 完整实现
 */
BOOL ProcessInherit_InjectDll(HANDLE processHandle, LPCWSTR dllPath) {
    if (!g_Initialized) {
        printf("[ProcessInherit] 错误: 未初始化\n");
        return FALSE;
    }
    
    if (processHandle == NULL || processHandle == INVALID_HANDLE_VALUE) {
        printf("[ProcessInherit] 错误: 进程句柄无效\n");
        return FALSE;
    }
    
    if (dllPath == NULL || wcslen(dllPath) == 0) {
        // 使用默认 DLL 路径
        dllPath = g_HookDllPath;
    }
    
    if (!Common_FileExists(dllPath)) {
        printf("[ProcessInherit] 错误: DLL 不存在: %ls\n", dllPath);
        return FALSE;
    }
    
    printf("[ProcessInherit] 开始注入 DLL: %ls\n", dllPath);
    
    // 1. 在目标进程中分配内存
    SIZE_T dllPathSize = (wcslen(dllPath) + 1) * sizeof(WCHAR);
    LPVOID remoteMem = VirtualAllocEx(processHandle, NULL, dllPathSize,
                                      MEM_COMMIT, PAGE_READWRITE);
    if (remoteMem == NULL) {
        printf("[ProcessInherit] 错误: 无法在目标进程中分配内存 (0x%X)\n",
               GetLastError());
        return FALSE;
    }
    
    // 2. 写入 DLL 路径到目标进程
    SIZE_T bytesWritten = 0;
    if (!WriteProcessMemory(processHandle, remoteMem, dllPath,
                           dllPathSize, &bytesWritten)) {
        printf("[ProcessInherit] 错误: 无法写入 DLL 路径到目标进程 (0x%X)\n",
               GetLastError());
        VirtualFreeEx(processHandle, remoteMem, 0, MEM_RELEASE);
        return FALSE;
    }
    
    if (bytesWritten != dllPathSize) {
        printf("[ProcessInherit] 错误: 写入字节数不匹配 (%Iu != %Iu)\n",
               bytesWritten, dllPathSize);
        VirtualFreeEx(processHandle, remoteMem, 0, MEM_RELEASE);
        return FALSE;
    }
    
    // 3. 创建远程线程（执行 LoadLibraryW）
    HANDLE remoteThread = CreateRemoteThread(
        processHandle,
        NULL,
        0,
        (LPTHREAD_START_ROUTINE)&LoadLibraryW,
        remoteMem,
        0,
        NULL
    );
    
    if (remoteThread == NULL) {
        printf("[ProcessInherit] 错误: 无法创建远程线程 (0x%X)\n",
               GetLastError());
        VirtualFreeEx(processHandle, remoteMem, 0, MEM_RELEASE);
        return FALSE;
    }
    
    // 4. 等待远程线程退出
    DWORD waitResult = WaitForSingleObject(remoteThread, 10000); // 等待 10 秒
    if (waitResult == WAIT_TIMEOUT) {
        printf("[ProcessInherit] 错误: 等待远程线程超时\n");
        TerminateThread(remoteThread, 0);
        CloseHandle(remoteThread);
        VirtualFreeEx(processHandle, remoteMem, 0, MEM_RELEASE);
        return FALSE;
    }
    
    // 5. 获取模块句柄
    DWORD moduleHandle = 0;
    if (!GetExitCodeThread(remoteThread, &moduleHandle)) {
        printf("[ProcessInherit] 警告: 无法获取远程线程退出码 (0x%X)\n",
               GetLastError());
    }
    
    // 6. 清理
    CloseHandle(remoteThread);
    VirtualFreeEx(processHandle, remoteMem, 0, MEM_RELEASE);
    
    if (moduleHandle == 0) {
        printf("[ProcessInherit] 错误: DLL 注入失败，LoadLibraryW 返回 NULL (0x%X)\n",
               GetLastError());
        return FALSE;
    }
    
    printf("[ProcessInherit] DLL 注入成功，模块句柄: 0x%X\n", moduleHandle);
    
    return TRUE;
}

/**
 * @brief 处理进程创建（Hook 回调函数）- 完整实现
 */
BOOL ProcessInherit_OnProcessCreated(PROCESS_INFORMATION* processInfo) {
    if (!g_Initialized || !g_Enabled) {
        return TRUE; // 未启用，透传
    }
    
    if (processInfo == NULL) {
        printf("[ProcessInherit] 错误: 进程信息为空\n");
        return FALSE;
    }
    
    printf("[ProcessInherit] 处理新创建的进程 (PID: %lu)\n",
           processInfo->dwProcessId);
    
    // 注入 Hook DLL
    if (wcslen(g_HookDllPath) > 0) {
        return ProcessInherit_InjectDll(processInfo->hProcess, g_HookDllPath);
    }
    
    return FALSE;
}

/**
 * @brief 设置 Hook DLL 路径
 */
BOOL ProcessInherit_SetDllPath(LPCWSTR dllPath) {
    if (dllPath == NULL || wcslen(dllPath) == 0) {
        printf("[ProcessInherit] 错误: DLL 路径为空\n");
        return FALSE;
    }
    
    wcsncpy_s(g_HookDllPath, MAX_PATH, dllPath, MAX_PATH - 1);
    
    printf("[ProcessInherit] 设置 Hook DLL 路径: %ls\n", g_HookDllPath);
    
    return TRUE;
}

/**
 * @brief 获取 Hook DLL 路径
 */
LPCWSTR ProcessInherit_GetDllPath(void) {
    return g_HookDllPath;
}
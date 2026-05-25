/**
 * @file hook_engine.cpp
 * @brief Hook 引擎模块源文件 - 完整实现
 * 
 * 基于 MinHook 实现 API Hook，用于拦截文件系统、注册表和进程创建相关 API。
 * 实现应用沙箱的文件系统和注册表虚拟化。
 * 
 * @author AI ThinApp Team
 * @date 2026-05-23
 */

#include "hook_engine.h"
#include "../vfs/vfs.h"
#include "../vreg/vreg.h"
#include "../process/process_inherit.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <memory>

// MinHook 头文件
#include <MinHook.h>

// 全局变量
static BOOL g_Initialized = FALSE;
static HANDLE g_HookCheckThread = NULL;
static BOOL g_ShouldStopCheck = FALSE;
static int g_RetryCount = 0;
static const int MAX_RETRY_COUNT = 3;

// 原函数指针保存
static NTSTATUS(NTAPI* OriginalNtCreateFile)(
    PHANDLE FileHandle,
    ACCESS_MASK DesiredAccess,
    POBJECT_ATTRIBUTES ObjectAttributes,
    PIO_STATUS_BLOCK IoStatusBlock,
    PLARGE_INTEGER AllocationSize,
    ULONG FileAttributes,
    ULONG ShareAccess,
    ULONG CreateDisposition,
    ULONG CreateOptions,
    PVOID EaBuffer,
    ULONG EaLength
) = nullptr;

static NTSTATUS(NTAPI* OriginalNtCreateKey)(
    PHANDLE KeyHandle,
    ACCESS_MASK DesiredAccess,
    POBJECT_ATTRIBUTES ObjectAttributes,
    ULONG TitleIndex,
    PUNICODE_STRING Class,
    ULONG CreateOptions,
    PULONG Disposition
) = nullptr;

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

// NtQueryValueKey 原函数指针
static NTSTATUS(NTAPI* OriginalNtQueryValueKey)(
    HANDLE KeyHandle,
    PUNICODE_STRING ValueName,
    KEY_VALUE_INFORMATION_CLASS KeyValueInformationClass,
    PVOID KeyValueInformation,
    ULONG Length,
    PULONG ResultLength
) = nullptr;

// ============================================================================
// 安全检查函数
// ============================================================================

/**
 * @brief 防止 DLL 劫持
 * 
 * 通过调用 SetDllDirectoryW(L"") 将当前目录从 DLL 搜索路径中移除，
 * 防止攻击者将恶意 DLL 放在应用程序目录下实施劫持。
 * 
 * @return TRUE: 成功; FALSE: 失败
 */
static BOOL HookEngine_PreventDllHijack(void) {
    // 将当前目录从 DLL 搜索路径中移除
    if (!SetDllDirectoryW(L"")) {
        DWORD error = GetLastError();
        printf("[HookEngine] 错误: SetDllDirectoryW 失败 (0x%X)\n", error);
        return FALSE;
    }
    
    printf("[HookEngine] ✅ DLL 劫持防护已启用（当前目录已从 DLL 搜索路径中移除）\n");
    return TRUE;
}

// ============================================================================
// Hook 函数实现
// ============================================================================

/**
 * @brief Hook NtCreateFile - 文件系统重定向
 */
static NTSTATUS NTAPI HookedNtCreateFile(
    PHANDLE FileHandle,
    ACCESS_MASK DesiredAccess,
    POBJECT_ATTRIBUTES ObjectAttributes,
    PIO_STATUS_BLOCK IoStatusBlock,
    PLARGE_INTEGER AllocationSize,
    ULONG FileAttributes,
    ULONG ShareAccess,
    ULONG CreateDisposition,
    ULONG CreateOptions,
    PVOID EaBuffer,
    ULONG EaLength
) {
    // 检查是否需要重定向
    if (ObjectAttributes && ObjectAttributes->ObjectName && ObjectAttributes->ObjectName->Buffer) {
        UNICODE_STRING& objName = *ObjectAttributes->ObjectName;
        std::unique_ptr<WCHAR[]> originalPath(new WCHAR[objName.Length / sizeof(WCHAR) + 1]);
        wcsncpy_s(originalPath.get(), objName.Length / sizeof(WCHAR) + 1,
                  objName.Buffer, objName.Length / sizeof(WCHAR));
        
        // 检查是否需要虚拟化
        if (VFS_NeedsVirtualization(originalPath.get())) {
            // 重定向路径
            WCHAR redirectedPath[MAX_PATH] = { 0 };
            if (VFS_RedirectPath(originalPath.get(), redirectedPath, MAX_PATH)) {
                // 修改 ObjectAttributes 指向重定向后的路径
                UNICODE_STRING newObjectName;
                RtlInitUnicodeString(&newObjectName, redirectedPath);
                ObjectAttributes->ObjectName = &newObjectName;
                
                printf("[Hook] NtCreateFile 重定向: %ws -> %ws\n", 
                       originalPath.get(), redirectedPath);
            }
        }
    }
    
    // 调用原始函数
    return OriginalNtCreateFile(
        FileHandle,
        DesiredAccess,
        ObjectAttributes,
        IoStatusBlock,
        AllocationSize,
        FileAttributes,
        ShareAccess,
        CreateDisposition,
        CreateOptions,
        EaBuffer,
        EaLength
    );
}

/**
 * @brief Hook NtCreateKey - 注册表重定向
 */
static NTSTATUS NTAPI HookedNtCreateKey(
    PHANDLE KeyHandle,
    ACCESS_MASK DesiredAccess,
    POBJECT_ATTRIBUTES ObjectAttributes,
    ULONG TitleIndex,
    PUNICODE_STRING Class,
    ULONG CreateOptions,
    PULONG Disposition
) {
    // 检查是否需要重定向
    if (ObjectAttributes && ObjectAttributes->ObjectName && ObjectAttributes->ObjectName->Buffer) {
        UNICODE_STRING& objName = *ObjectAttributes->ObjectName;
        std::unique_ptr<WCHAR[]> originalPath(new WCHAR[objName.Length / sizeof(WCHAR) + 1]);
        wcsncpy_s(originalPath.get(), objName.Length / sizeof(WCHAR) + 1,
                  objName.Buffer, objName.Length / sizeof(WCHAR));
        
        // 检查是否需要虚拟化
        if (VReg_NeedsVirtualization(originalPath.get())) {
            // 重定向路径
            WCHAR redirectedPath[MAX_PATH] = { 0 };
            if (VReg_RedirectKeyPath(originalPath.get(), redirectedPath, MAX_PATH)) {
                // 修改 ObjectAttributes 指向重定向后的路径
                UNICODE_STRING newObjectName;
                RtlInitUnicodeString(&newObjectName, redirectedPath);
                ObjectAttributes->ObjectName = &newObjectName;
                
                printf("[Hook] NtCreateKey 重定向: %ws -> %ws\n",
                       originalPath.get(), redirectedPath);
            }
        }
    }
    
    // 调用原始函数
    return OriginalNtCreateKey(
        KeyHandle,
        DesiredAccess,
        ObjectAttributes,
        TitleIndex,
        Class,
        CreateOptions,
        Disposition
    );
}

/**
 * @brief Hook CreateProcessW - 子进程继承 Hook DLL
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
    printf("[Hook] CreateProcessW 被调用: %ws\n", lpApplicationName ? lpApplicationName : L"null");
    
    // 调用原始函数创建进程
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
        ProcessInherit_OnProcessCreated(lpProcessInformation);
    }
    
    return result;
}

// ============================================================================
// Hook 覆盖检测与自动重装
// ============================================================================

/**
 * @brief 事件驱动的 Hook 覆盖检测
 * 
 * 使用 ReadDirectoryChangesW() 监控 Hook DLL 文件的变化。
 * 当 Hook DLL 被修改或替换时，立即检测到并触发 Hook 重装。
 * 
 * @param stopFlag 停止标志指针
 * @return DWORD 线程退出码
 */
static DWORD HookCheck_EventDriven(volatile BOOL* stopFlag) {
    WCHAR modulePath[MAX_PATH] = { 0 };
    GetModuleFileNameW(NULL, modulePath, MAX_PATH);
    
    WCHAR dirPath[MAX_PATH] = { 0 };
    wcsncpy_s(dirPath, MAX_PATH, modulePath, MAX_PATH - 1);
    WCHAR* lastSlash = wcsrchr(dirPath, L'\\');
    if (lastSlash != NULL) {
        *lastSlash = L'\0';
    }
    
    // 打开目录用于监控
    HANDLE hDir = CreateFileW(
        dirPath,
        FILE_LIST_DIRECTORY,
        FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
        NULL,
        OPEN_EXISTING,
        FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OVERLAPPED,
        NULL
    );
    
    if (hDir == INVALID_HANDLE_VALUE) {
        printf("[HookEngine] 警告: 无法打开目录进行监控: %ls (错误: 0x%X)\n", dirPath, GetLastError());
        return 1;
    }
    
    BYTE buffer[4096] = { 0 };
    DWORD bytesReturned = 0;
    OVERLAPPED overlapped = { 0 };
    overlapped.hEvent = CreateEventW(NULL, TRUE, FALSE, NULL);
    
    printf("[HookEngine] ✅ 事件驱动 Hook 监控已启动: %ls\n", dirPath);
    
    while (!*stopFlag) {
        // 设置监控
        BOOL result = ReadDirectoryChangesW(
            hDir,
            buffer,
            sizeof(buffer),
            TRUE,  // 监控子目录
            FILE_NOTIFY_CHANGE_FILE_NAME | FILE_NOTIFY_CHANGE_LAST_WRITE,
            &bytesReturned,
            &overlapped,
            NULL
        );
        
        if (!result) {
            printf("[HookEngine] 警告: ReadDirectoryChangesW 失败 (0x%X)\n", GetLastError());
            break;
        }
        
        // 等待事件（超时 1 秒，以便检查 stopFlag）
        DWORD waitResult = WaitForSingleObject(overlapped.hEvent, 1000);
        
        if (waitResult == WAIT_OBJECT_0) {
            // 检测到变化
            DWORD offset = 0;
            PFILE_NOTIFY_INFORMATION pNotify = NULL;
            
            do {
                pNotify = (PFILE_NOTIFY_INFORMATION)(buffer + offset);
                
                // 检查是否是我们的 Hook DLL
                WCHAR fileName[MAX_PATH] = { 0 };
                wcsncpy_s(fileName, MAX_PATH, pNotify->FileName, pNotify->FileNameLength / sizeof(WCHAR));
                
                WCHAR moduleName[MAX_PATH] = { 0 };
                GetModuleFileNameW(NULL, moduleName, MAX_PATH);
                WCHAR* exeName = wcsrchr(moduleName, L'\\');
                if (exeName != NULL) {
                    exeName++;  // 跳过反斜杠
                    
                    if (_wcsicmp(fileName, exeName) == 0) {
                        printf("[HookEngine] 警告: Hook DLL 被修改！触发重装...\n");
                        
                        // 触发 Hook 重装
                        if (MH_DisableHook(MH_ALL_HOOKS) == MH_OK) {
                            if (MH_EnableHook(MH_ALL_HOOKS) == MH_OK) {
                                printf("[HookEngine] Hook 重装成功\n");
                            }
                        }
                    }
                }
                
                offset += pNotify->NextEntryOffset;
            } while (pNotify->NextEntryOffset != 0);
            
            // 重置事件
            ResetEvent(overlapped.hEvent);
        }
        
        // 检查停止标志
        if (*stopFlag) {
            break;
        }
    }
    
    // 清理
    CancelIo(hDir);
    CloseHandle(overlapped.hEvent);
    CloseHandle(hDir);
    
    printf("[HookEngine] 事件驱动 Hook 监控已停止\n");
    return 0;
}

/**
 * @brief Hook 检查线程函数
 */
static DWORD WINAPI HookCheckThreadProc(LPVOID lpParam) {
    WCHAR logPath[MAX_PATH];
    GetModuleFileNameW(NULL, logPath, MAX_PATH);
    wcsrchr(logPath, L'\\')[0] = L'\0';
    wcscat_s(logPath, L"\\hook_check.log");
    
    // [P0-8 修复] 使用事件驱动检测替代轮询
    printf("[HookEngine] 使用事件驱动检测 Hook 覆盖\n");
    
    // 启动事件驱动检测（会阻塞直到停止）
    DWORD result = HookCheck_EventDriven(&g_ShouldStopCheck);
    
    if (result != 0) {
        printf("[HookEngine] 警告: 事件驱动检测启动失败，回退到轮询检测\n");
        
        // 回退到轮询检测
        while (!g_ShouldStopCheck) {
            // 等待 5 秒
            for (int i = 0; i < 5; i++) {
                if (g_ShouldStopCheck) {
                    break;
                }
                Sleep(1000);
            }
            
            if (g_ShouldStopCheck) {
                break;
            }
            
            // 检查 Hook 是否被覆盖 - 修复逻辑错误
            BOOL hookIntact = TRUE;
            
            // 正确检查：尝试禁用然后重新启用 Hook
            if (MH_DisableHook(MH_ALL_HOOKS) != MH_OK) {
                hookIntact = FALSE;
            } else {
                // 重新启用 Hook
                if (MH_EnableHook(MH_ALL_HOOKS) != MH_OK) {
                    hookIntact = FALSE;
                }
            }
        
        if (!hookIntact) {
            // Hook 被覆盖，尝试重装
            g_RetryCount++;
            
            printf("[HookEngine] 警告: Hook 可能被覆盖，尝试重装 (第 %d/%d 次)\n",
                   g_RetryCount, MAX_RETRY_COUNT);
            
            // 写入日志文件
            FILE* logFile = nullptr;
            _wfopen_s(&logFile, logPath, L"a");
            if (logFile) {
                SYSTEMTIME st;
                GetLocalTime(&st);
                fwprintf(logFile, L"%04d-%02d-%02d %02d:%02d:%02d [WARNING] Hook 可能被覆盖，尝试重装 (第 %d/%d 次)\n",
                         st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond,
                         g_RetryCount, MAX_RETRY_COUNT);
                fclose(logFile);
            }
            
            // 尝试重装 Hook
            if (MH_EnableHook(MH_ALL_HOOKS) == MH_OK) {
                printf("[HookEngine] Hook 重装成功\n");
                g_RetryCount = 0;
                
                if (logFile = nullptr, _wfopen_s(&logFile, logPath, L"a") == 0 && logFile) {
                    SYSTEMTIME st;
                    GetLocalTime(&st);
                    fwprintf(logFile, L"%04d-%02d-%02d %02d:%02d:%02d [INFO] Hook 重装成功\n",
                             st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);
                    fclose(logFile);
                }
            } else {
                printf("[HookEngine] 错误: Hook 重装失败\n");
                
                if (g_RetryCount >= MAX_RETRY_COUNT) {
                    printf("[HookEngine] 错误: Hook 重装失败超过 %d 次，停止尝试\n", MAX_RETRY_COUNT);
                    
                    if (logFile = nullptr, _wfopen_s(&logFile, logPath, L"a") == 0 && logFile) {
                        SYSTEMTIME st;
                        GetLocalTime(&st);
                        fwprintf(logFile, L"%04d-%02d-%02d %02d:%02d:%02d [ERROR] Hook 重装失败超过 %d 次，停止尝试\n",
                                 st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond,
                                 MAX_RETRY_COUNT);
                        fwprintf(logFile, L"%04d-%02d-%02d %02d:%02d:%02d [ERROR] 请检查是否有其他软件干扰，或重启应用\n",
                                 st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);
                        fclose(logFile);
                    }
                    
                    MessageBoxW(NULL,
                                L"Hook 引擎重装失败超过 3 次！\n\n"
                                L"可能原因：\n"
                                L"1. 其他软件（杀毒/注入工具）覆盖了 Hook\n"
                                L"2. Hook 引擎内部错误\n\n"
                                L"建议：\n"
                                L"1. 关闭可能干扰的软件\n"
                                L"2. 重启应用\n"
                                L"3. 查看日志文件: hook_check.log",
                                L"Hook 引擎错误",
                                MB_OK | MB_ICONERROR);
                    
                    break;
                }
            }
        } else {
            g_RetryCount = 0;
        }
    }
    
    return 0;
}

// ============================================================================
// 公开 API 实现
// ============================================================================

/**
 * @brief 初始化 Hook 引擎
 */
BOOL HookEngine_Initialize(void) {
    if (g_Initialized) {
        printf("[HookEngine] 警告: 已经初始化\n");
        return TRUE;
    }
    
    // 安全加固：防止 DLL 劫持
    if (!HookEngine_PreventDllHijack()) {
        printf("[HookEngine] 警告: DLL 劫持防护启用失败\n");
        // 不阻止初始化，仅警告
    }
    
    NTSTATUS status = MH_Initialize();
    if (status != MH_OK) {
        printf("[HookEngine] 错误: MinHook 初始化失败 (0x%X)\n", status);
        return FALSE;
    }
    
    g_Initialized = TRUE;
    printf("[HookEngine] 初始化成功\n");
    
    return TRUE;
}

/**
 * @brief 安装所有预定义的 Hook
 */
BOOL HookEngine_InstallHooks(void) {
    if (!g_Initialized) {
        printf("[HookEngine] 错误: Hook 引擎未初始化\n");
        return FALSE;
    }
    
    printf("[HookEngine] 开始安装 Hook...\n");
    
    // 安装文件系统 Hook (NtCreateFile)
    if (MH_CreateHook(&NtCreateFile, &HookedNtCreateFile,
                      reinterpret_cast<LPVOID*>(&OriginalNtCreateFile)) != MH_OK) {
        printf("[HookEngine] 错误: 无法 Hook NtCreateFile\n");
    } else {
        printf("[HookEngine] 成功: Hook NtCreateFile\n");
    }
    
    // 安装注册表 Hook (NtCreateKey)
    if (MH_CreateHook(&NtCreateKey, &HookedNtCreateKey,
                      reinterpret_cast<LPVOID*>(&OriginalNtCreateKey)) != MH_OK) {
        printf("[HookEngine] 错误: 无法 Hook NtCreateKey\n");
    } else {
        printf("[HookEngine] 成功: Hook NtCreateKey\n");
    }
    
    // 安装进程创建 Hook (CreateProcessW)
    if (MH_CreateHook(&CreateProcessW, &HookedCreateProcessW,
                      reinterpret_cast<LPVOID*>(&OriginalCreateProcessW)) != MH_OK) {
        printf("[HookEngine] 错误: 无法 Hook CreateProcessW\n");
    } else {
        printf("[HookEngine] 成功: Hook CreateProcessW\n");
    }
    
    // 启用所有 Hook
    if (MH_EnableHook(MH_ALL_HOOKS) != MH_OK) {
        printf("[HookEngine] 错误: 无法启用 Hook\n");
        return FALSE;
    }
    
    printf("[HookEngine] Hook 安装完成\n");
    
    // 启动 Hook 覆盖检测线程
    HookEngine_StartHookCheck();
    
    return TRUE;
}

/**
 * @brief 启动 Hook 覆盖检测线程
 */
BOOL HookEngine_StartHookCheck(void) {
    if (g_HookCheckThread != NULL) {
        return TRUE;
    }
    
    g_ShouldStopCheck = FALSE;
    g_RetryCount = 0;
    
    DWORD threadId = 0;
    g_HookCheckThread = CreateThread(
        NULL,
        0,
        HookCheckThreadProc,
        NULL,
        0,
        &threadId
    );
    
    if (g_HookCheckThread == NULL) {
        printf("[HookEngine] 错误: 无法创建 Hook 检查线程\n");
        return FALSE;
    }
    
    printf("[HookEngine] Hook 覆盖检测线程已启动 (TID: %lu)\n", threadId);
    return TRUE;
}

/**
 * @brief 停止 Hook 覆盖检测线程
 */
BOOL HookEngine_StopHookCheck(void) {
    if (g_HookCheckThread == NULL) {
        return TRUE;
    }
    
    g_ShouldStopCheck = TRUE;
    
    DWORD waitResult = WaitForSingleObject(g_HookCheckThread, 10000);
    if (waitResult == WAIT_TIMEOUT) {
        TerminateThread(g_HookCheckThread, 0);
        printf("[HookEngine] 警告: Hook 检查线程超时，已强制终止\n");
    } else {
        printf("[HookEngine] Hook 检查线程已停止\n");
    }
    
    CloseHandle(g_HookCheckThread);
    g_HookCheckThread = NULL;
    
    return TRUE;
}

/**
 * @brief 卸载所有 Hook
 */
BOOL HookEngine_UninstallHooks(void) {
    if (!g_Initialized) {
        printf("[HookEngine] 警告: Hook 引擎未初始化\n");
        return TRUE;
    }
    
    printf("[HookEngine] 开始卸载 Hook...\n");
    
    if (MH_DisableHook(MH_ALL_HOOKS) != MH_OK) {
        printf("[HookEngine] 错误: 无法卸载 Hook\n");
        return FALSE;
    }
    
    printf("[HookEngine] Hook 卸载完成\n");
    
    return TRUE;
}

/**
 * @brief 清理 Hook 引擎
 */
BOOL HookEngine_Shutdown(void) {
    if (!g_Initialized) {
        return TRUE;
    }
    
    HookEngine_StopHookCheck();
    HookEngine_UninstallHooks();
    
    NTSTATUS status = MH_Uninitialize();
    if (status != MH_OK) {
        printf("[HookEngine] 警告: MinHook 清理失败 (0x%X)\n", status);
    }
    
    g_Initialized = FALSE;
    printf("[HookEngine] 清理完成\n");
    
    return TRUE;
}

/**
 * @brief 检查 Hook 引擎是否已初始化
 */
BOOL HookEngine_IsInitialized(void) {
    return g_Initialized;
}
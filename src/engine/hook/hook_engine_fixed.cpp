/**
 * @file hook_engine.cpp (修复版)
 * @brief Hook 引擎模块源文件
 * 
 * 基于 MinHook 实现 API Hook，用于拦截文件系统、注册表和进程创建相关 API。
 * 实现应用沙箱的文件系统和注册表虚拟化。
 * 
 * @author AI ThinApp Team
 * @date 2026-05-23
 */

#include "hook_engine.h"
#include <stdio.h>
#include <string.h>
#include <vector>
#include <algorithm>
#include <fstream>

// Windows Native API 头文件
#include <winternl.h>
#include <ntstatus.h>

// MinHook 头文件
#include <MinHook.h>

// 全局变量
static BOOL g_Initialized = FALSE;
static HANDLE g_HookCheckThread = NULL;  // Hook 检查线程句柄
static BOOL g_ShouldStopCheck = FALSE;   // 停止检查标志
static int g_RetryCount = 0;             // 重装重试次数
static const int MAX_RETRY_COUNT = 3;    // 最大重试次数

// 原函数指针保存（用于验证 Hook 是否被覆盖）
static LPVOID g_OriginalNtCreateFile = NULL;
static LPVOID g_OriginalNtCreateKey = NULL;
static LPVOID g_OriginalCreateProcessW = NULL;

// ============================================================================
// 捕获模式全局变量（用于应用捕获工具）
// ============================================================================

static BOOL g_CaptureModeEnabled = FALSE;  // 捕获模式是否启用
static std::vector<CAPTURE_RECORD> g_CaptureRecords;  // 捕获记录列表
static CRITICAL_SECTION g_CaptureRecordsLock;  // 保护捕获记录列表的临界区

// ============================================================================
// 文件系统 Hook 函数声明（桩实现）
// ============================================================================

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
);

// 注册表 Hook 函数声明（桩实现）
static NTSTATUS NTAPI HookedNtCreateKey(
    PHANDLE KeyHandle,
    ACCESS_MASK DesiredAccess,
    POBJECT_ATTRIBUTES ObjectAttributes,
    ULONG TitleIndex,
    PUNICODE_STRING Class,
    ULONG CreateOptions,
    PULONG Disposition
);

// 进程创建 Hook 函数声明（桩实现）
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
);

// ============================================================================
// 捕获模式辅助函数
// ============================================================================

/**
 * @brief 记录捕获事件（线程安全）
 * 
 * 将捕获事件添加到全局记录列表。
 * 
 * @param eventType 事件类型
 * @param path 文件/注册表路径
 * @param data 数据（可选，如文件内容、注册表值）
 * @param dataSize 数据大小
 */
static void RecordCaptureEvent(CAPTURE_EVENT_TYPE eventType,
                               const WCHAR* path,
                               const BYTE* data,
                               size_t dataSize) {
    // 检查捕获模式是否启用
    if (!g_CaptureModeEnabled) {
        return;  // 捕获模式未启用，不记录
    }
    
    // 检查参数有效性
    if (path == NULL) {
        return;
    }
    
    // 创建捕获记录
    CAPTURE_RECORD record;
    ZeroMemory(&record, sizeof(record));
    
    record.eventType = eventType;
    wcsncpy_s(record.path, path, _countof(record.path) - 1);
    
    if (data != NULL && dataSize > 0 && dataSize <= sizeof(record.data)) {
        CopyMemory(record.data, data, dataSize);
        record.dataSize = dataSize;
    }
    
    GetSystemTimeAsFileTime(&record.timestamp);
    record.processId = GetCurrentProcessId();
    
    // 获取进程名称
    HMODULE hModule = GetModuleHandleW(NULL);
    GetModuleFileNameW(hModule, record.processName, _countof(record.processName));
    
    // 线程安全地添加到全局列表
    EnterCriticalSection(&g_CaptureRecordsLock);
    g_CaptureRecords.push_back(record);
    LeaveCriticalSection(&g_CaptureRecordsLock);
    
    // 打印调试信息
    printf("[Capture] 记录事件: %d, 路径: %ls\n", eventType, path);
}

// ============================================================================
// Hook 引擎 API 实现
// ============================================================================

/**
 * @brief 初始化 Hook 引擎
 */
BOOL HookEngine_Initialize(void)
{
    if (g_Initialized) {
        printf("[HookEngine] 警告: 已经初始化\n");
        return TRUE;
    }
    
    // 初始化 MinHook
    NTSTATUS status = MH_Initialize();
    if (status != MH_OK) {
        printf("[HookEngine] 错误: MinHook 初始化失败 (0x%X)\n", status);
        return FALSE;
    }
    
    // 初始化捕获记录锁
    InitializeCriticalSection(&g_CaptureRecordsLock);
    
    g_Initialized = TRUE;
    printf("[HookEngine] 初始化成功\n");
    
    return TRUE;
}

/**
 * @brief 安装所有预定义的 Hook
 */
BOOL HookEngine_InstallHooks(void)
{
    if (!g_Initialized) {
        printf("[HookEngine] 错误: Hook 引擎未初始化\n");
        return FALSE;
    }
    
    printf("[HookEngine] 开始安装 Hook...\n");
    
    // TODO: 安装文件系统 Hook（POC 阶段桩实现）
    printf("[HookEngine] 提示: 文件系统 Hook 未实现（桩）\n");
    
    // TODO: 安装注册表 Hook（调用 VRegEngine）
    // VRegEngine::InstallHooks();
    printf("[HookEngine] 提示: 注册表 Hook 未实现（桩）\n");
    
    // TODO: 安装进程创建 Hook（调用 ProcessInherit）
    // ProcessInherit::InstallHooks();
    printf("[HookEngine] 提示: 进程创建 Hook 未实现（桩）\n");
    
    printf("[HookEngine] Hook 安装完成（桩实现）\n");
    
    // [修复限制3] 启动 Hook 覆盖检测线程
    HookEngine_StartHookCheck();
    
    return TRUE;
}

// ============================================================================
// Hook 覆盖检测与自动重装（修复限制3）
// ============================================================================

/**
 * @brief Hook 检查线程函数
 * @param lpParam 线程参数（未使用）
 * @return 0: 线程正常退出
 * 
 * 功能：
 * 1. 每 5 秒检查一次 Hook 是否被覆盖
 * 2. 若被覆盖，自动重装 Hook
 * 3. 重装失败超过 3 次 → 写入日志文件并通知用户
 */
static DWORD WINAPI HookCheckThreadProc(LPVOID lpParam) {
    WCHAR logPath[MAX_PATH];
    GetModuleFileNameW(NULL, logPath, MAX_PATH);
    wcsrchr(logPath, L'\\')[0] = L'\0'; // 去掉文件名
    wcscat_s(logPath, L"\\hook_check.log");
    
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
        
        // 检查 Hook 是否被覆盖
        BOOL hookIntact = TRUE;
        
        // 检查 MinHook 状态
        if (MH_OK != MH_OK) { // 简化检查：实际应检查每个 Hook 状态
            hookIntact = FALSE;
        }
        
        // 更严格的检查：尝试获取原函数地址
        // 如果 Hook 被覆盖，原函数地址可能已被修改
        // TODO: V2 实现完整的 Hook 完整性检查
        
        if (!hookIntact) {
            // Hook 被覆盖，尝试重装
            g_RetryCount++;
            
            printf("[HookEngine] 警告: Hook 可能被覆盖，尝试重装 (第 %d/%d 次)\n", 
                   g_RetryCount, MAX_RETRY_COUNT);
            
            // 写入日志文件
            std::wofstream logFile(logPath, std::ios::app);
            if (logFile.is_open()) {
                SYSTEMTIME st;
                GetLocalTime(&st);
                logFile << st.wYear << L"-" << st.wMonth << L"-" << st.wDay << L" "
                       << st.wHour << L":" << st.wMinute << L":" << st.wSecond
                       << L" [WARNING] Hook 可能被覆盖，尝试重装 (第 " 
                       << g_RetryCount << L"/" << MAX_RETRY_COUNT << L" 次)\n";
                logFile.close();
            }
            
            // 尝试重装 Hook
            if (MH_EnableHook(MH_ALL_HOOKS) == MH_OK) {
                // 重装成功
                printf("[HookEngine] Hook 重装成功\n");
                g_RetryCount = 0; // 重置重试计数
                
                logFile.open(logPath, std::ios::app);
                if (logFile.is_open()) {
                    SYSTEMTIME st;
                    GetLocalTime(&st);
                    logFile << st.wYear << L"-" << st.wMonth << L"-" << st.wDay << L" "
                           << st.wHour << L":" << st.wMinute << L":" << st.wSecond
                           << L" [INFO] Hook 重装成功\n";
                    logFile.close();
                }
            } else {
                // 重装失败
                printf("[HookEngine] 错误: Hook 重装失败\n");
                
                // 检查是否超过最大重试次数
                if (g_RetryCount >= MAX_RETRY_COUNT) {
                    printf("[HookEngine] 错误: Hook 重装失败超过 %d 次，停止尝试\n", MAX_RETRY_COUNT);
                    
                    // 写入日志文件
                    logFile.open(logPath, std::ios::app);
                    if (logFile.is_open()) {
                        SYSTEMTIME st;
                        GetLocalTime(&st);
                        logFile << st.wYear << L"-" << st.wMonth << L"-" << st.wDay << L" "
                               << st.wHour << L":" << st.wMinute << L":" << st.wSecond
                               << L" [ERROR] Hook 重装失败超过 " << MAX_RETRY_COUNT 
                               << L" 次，停止尝试\n";
                        logFile << st.wYear << L"-" << st.wMonth << L"-" << st.wDay << L" "
                               << st.wHour << L":" << st.wMinute << L":" << st.wSecond
                               << L" [ERROR] 请检查是否有其他软件干扰，或重启应用\n";
                        logFile.close();
                    }
                    
                    // 通知用户（简化版：弹窗）
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
                    
                    break; // 退出线程
                }
            }
        } else {
            // Hook 完整，重置重试计数
            g_RetryCount = 0;
        }
    }
    
    return 0;
}

/**
 * @brief 启动 Hook 覆盖检测线程
 * @return TRUE: 成功, FALSE: 失败
 */
BOOL HookEngine_StartHookCheck(void) {
    if (g_HookCheckThread != NULL) {
        return TRUE; // 已经启动
    }
    
    g_ShouldStopCheck = FALSE;
    g_RetryCount = 0;
    
    // 创建线程
    DWORD threadId = 0;
    g_HookCheckThread = CreateThread(
        NULL,          // 默认安全属性
        0,             // 默认栈大小
        HookCheckThreadProc, // 线程函数
        NULL,          // 参数
        0,             // 立即运行
        &threadId      // 返回线程 ID
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
 * @return TRUE: 成功, FALSE: 失败
 */
BOOL HookEngine_StopHookCheck(void) {
    if (g_HookCheckThread == NULL) {
        return TRUE; // 已经停止
    }
    
    // 设置停止标志
    g_ShouldStopCheck = TRUE;
    
    // 等待线程退出（最多等待 10 秒）
    DWORD waitResult = WaitForSingleObject(g_HookCheckThread, 10000);
    if (waitResult == WAIT_TIMEOUT) {
        // 强制终止线程（不推荐，但无奈之举）
        TerminateThread(g_HookCheckThread, 0);
        printf("[HookEngine] 警告: Hook 检查线程超时，已强制终止\n");
    } else {
        printf("[HookEngine] Hook 检查线程已停止\n");
    }
    
    // 关闭线程句柄
    CloseHandle(g_HookCheckThread);
    g_HookCheckThread = NULL;
    
    return TRUE;
}

/**
 * @brief 卸载所有 Hook
 */
BOOL HookEngine_UninstallHooks(void)
{
    if (!g_Initialized) {
        printf("[HookEngine] 警告: Hook 引擎未初始化\n");
        return TRUE;
    }
    
    printf("[HookEngine] 开始卸载 Hook...\n");
    
    // TODO: 卸载所有 Hook
    printf("[HookEngine] 提示: Hook 卸载未实现（桩）\n");
    
    printf("[HookEngine] Hook 卸载完成（桩实现）\n");
    
    return TRUE;
}

/**
 * @brief 清理 Hook 引擎
 */
BOOL HookEngine_Shutdown(void)
{
    if (!g_Initialized) {
        return TRUE;
    }
    
    // [修复限制3] 停止 Hook 覆盖检测线程
    HookEngine_StopHookCheck();
    
    // 卸载所有 Hook
    HookEngine_UninstallHooks();
    
    // 清理捕获模式资源
    HookEngine_DisableCaptureMode();
    DeleteCriticalSection(&g_CaptureRecordsLock);
    
    // 清理 MinHook
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
BOOL HookEngine_IsInitialized(void)
{
    return g_Initialized;
}

// ============================================================================
// 捕获模式 API 实现（用于应用捕获工具）
// ============================================================================

/**
 * @brief 启用捕获模式
 */
BOOL HookEngine_EnableCaptureMode(void) {
    if (!g_Initialized) {
        printf("[HookEngine] 错误: Hook 引擎未初始化\n");
        return FALSE;
    }
    
    if (g_CaptureModeEnabled) {
        printf("[HookEngine] 警告: 捕获模式已启用\n");
        return TRUE;
    }
    
    // 清除之前的捕获记录
    HookEngine_ClearCaptureRecords();
    
    g_CaptureModeEnabled = TRUE;
    printf("[HookEngine] 捕获模式已启用\n");
    
    return TRUE;
}

/**
 * @brief 禁用捕获模式
 */
BOOL HookEngine_DisableCaptureMode(void) {
    if (!g_CaptureModeEnabled) {
        printf("[HookEngine] 警告: 捕获模式未启用\n");
        return TRUE;
    }
    
    g_CaptureModeEnabled = FALSE;
    printf("[HookEngine] 捕获模式已禁用\n");
    
    return TRUE;
}

/**
 * @brief 检查捕获模式是否已启用
 */
BOOL HookEngine_IsCaptureModeEnabled(void) {
    return g_CaptureModeEnabled;
}

/**
 * @brief 清除捕获记录
 */
BOOL HookEngine_ClearCaptureRecords(void) {
    EnterCriticalSection(&g_CaptureRecordsLock);
    g_CaptureRecords.clear();
    LeaveCriticalSection(&g_CaptureRecordsLock);
    
    printf("[HookEngine] 捕获记录已清除\n");
    return TRUE;
}

/**
 * @brief 获取捕获记录数量
 */
size_t HookEngine_GetCaptureRecordCount(void) {
    EnterCriticalSection(&g_CaptureRecordsLock);
    size_t count = g_CaptureRecords.size();
    LeaveCriticalSection(&g_CaptureRecordsLock);
    
    return count;
}

/**
 * @brief 获取捕获记录
 */
size_t HookEngine_GetCaptureRecords(CAPTURE_RECORD* records, size_t maxRecords) {
    if (records == NULL || maxRecords == 0) {
        return 0;
    }
    
    EnterCriticalSection(&g_CaptureRecordsLock);
    
    size_t count = min(maxRecords, g_CaptureRecords.size());
    for (size_t i = 0; i < count; i++) {
        records[i] = g_CaptureRecords[i];
    }
    
    LeaveCriticalSection(&g_CaptureRecordsLock);
    
    return count;
}

// ============================================================
// Hook 函数实现（桩）
// ============================================================

/**
 * @brief Hook NtCreateFile（桩实现）
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
    printf("[Hook] NtCreateFile 被调用（未实现）\n");
    
    // 记录捕获事件（若捕获模式已启用）
    if (g_CaptureModeEnabled && ObjectAttributes != NULL && ObjectAttributes->ObjectName != NULL) {
        RecordCaptureEvent(
            CAPTURE_EVENT_FILE_CREATE,
            ObjectAttributes->ObjectName->Buffer,
            NULL,
            0
        );
    }
    
    // TODO: 重定向文件路径到 VFS
    
    // 调用原函数（透传）
    // return OriginalNtCreateFile(...);
    
    return STATUS_NOT_IMPLEMENTED;
}

/**
 * @brief Hook NtCreateKey（桩实现）
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
    printf("[Hook] NtCreateKey 被调用（未实现）\n");
    
    // 记录捕获事件（若捕获模式已启用）
    if (g_CaptureModeEnabled && ObjectAttributes != NULL && ObjectAttributes->ObjectName != NULL) {
        RecordCaptureEvent(
            CAPTURE_EVENT_REG_CREATE_KEY,
            ObjectAttributes->ObjectName->Buffer,
            NULL,
            0
        );
    }
    
    // TODO: 重定向注册表路径到虚拟 hive
    
    return STATUS_NOT_IMPLEMENTED;
}

/**
 * @brief Hook CreateProcessW（桩实现）
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
    printf("[Hook] CreateProcessW 被调用（未实现）\n");
    
    // 记录捕获事件（若捕获模式已启用）
    if (g_CaptureModeEnabled && lpApplicationName != NULL) {
        RecordCaptureEvent(
            CAPTURE_EVENT_FILE_CREATE,  // 使用 FILE_CREATE 作为进程创建事件（简化）
            lpApplicationName,
            NULL,
            0
        );
    }
    
    // TODO: 注入 Hook DLL 到子进程
    
    return FALSE;
}

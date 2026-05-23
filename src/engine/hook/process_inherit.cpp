/**
 * @file process_inherit.cpp
 * @brief 进程继承引擎实现 - 子进程 Hook 继承(Week 3 完善版)
 * @author AI ThinApp Team
 * @date 2026-05-23
 *
 * 基于 MinHook 实现进程创建 API Hook,
 * 拦截 CreateProcessW / ShellExecuteExW,
 * 实现子进程自动注入 Hook DLL 逻辑。
 *
 * Week 3 改进:
 * 1. 使用 CreateRemoteThreadEx 提升注入成功率
 * 2. 支持 5 层子进程继承(递归注入)
 * 3. 处理 WOW64 场景(32/64 位进程交叉注入)
 * 4. 完善的错误处理和日志记录
 * 5. 线程安全(使用 SRWLOCK 替代 std::mutex)
 */

#include "process_inherit.h"
#include <TlHelp32.h>
#include <tchar.h>
#include <fstream>
#include <sstream>
#include <iomanip>

#pragma comment(lib, "advapi32.lib")

// 进程继承模块 - 工具函数模块
// 不自行 Hook，而是由 hook_engine.cpp 调用本模块提供的函数

// Hook 引擎状态
static bool g_initialized = false;
static std::wstring g_dllPath64;  // 64 位 Hook DLL 路径
static std::wstring g_dllPath32;  // 32 位 Hook DLL 路径(WOW64 场景使用)
static SRWLOCK g_srwLock = SRWLOCK_INIT;  // 读写锁(替代 std::mutex)
static HMODULE g_currentModule = nullptr;   // 当前模块句柄

// 进程继承信息表(进程 ID -> 继承信息)
static std::map<DWORD, PROCESS_INHERIT_INFO> g_inheritInfoMap;

namespace ProcessInherit {

// ============================================================================
// 工具函数
// ============================================================================

/**
 * @brief 获取当前模块的 DLL 路径
 */
static std::wstring GetModulePath(HMODULE hModule) {
    wchar_t path[MAX_PATH];
    GetModuleFileNameW(hModule, path, MAX_PATH);
    return std::wstring(path);
}

/**
 * @brief 判断当前进程是否为 64 位进程
 * @return true: 64 位进程, false: 32 位进程
 */
bool IsCurrentProcess64Bit() {
#if defined(_WIN64)
    return true;
#else
    BOOL isWow64 = FALSE;
    IsWow64Process(GetCurrentProcess(), &isWow64);
    return isWow64 == TRUE;
#endif
}

/**
 * @brief 判断目标进程是否为 WOW64 进程(32 位进程运行在 64 位系统上)
 * @param process_handle 目标进程句柄
 * @return TRUE: 是 WOW64 进程, FALSE: 不是或失败
 */
BOOL IsWow64Process(HANDLE process_handle) {
    BOOL isWow64 = FALSE;
    if (!::IsWow64Process(process_handle, &isWow64)) {
        return FALSE;  // API 调用失败
    }
    return isWow64;
}

/**
 * @brief 判断目标进程是否为 64 位进程
 * @param process_handle 目标进程句柄
 * @return true: 64 位进程, false: 32 位进程或失败
 */
static bool IsProcess64Bit(HANDLE process_handle) {
    BOOL isWow64 = FALSE;
    if (!IsWow64Process(process_handle, &isWow64)) {
        return false;  // API 调用失败
    }
    // 如果不是 WOW64 进程,则是 64 位进程(在 64 位系统上)
    // 如果是 WOW64 进程,则是 32 位进程
    return isWow64 == FALSE;
}

/**
 * @brief 写入注入错误日志
 * @param process_id 进程 ID
 * @param error_code 错误代码
 * @param message 错误信息
 */
void WriteInjectErrorLog(DWORD process_id, DWORD error_code, const std::wstring& message) {
    // 确保 logs 目录存在
    wchar_t logPath[MAX_PATH];
    GetCurrentDirectoryW(MAX_PATH, logPath);
    wcscat_s(logPath, L"\\logs");

    // 创建 logs 目录(如果不存在)
    CreateDirectoryW(logPath, nullptr);

    wcscat_s(logPath, L"\\inject.log");

    // 获取当前时间
    SYSTEMTIME st;
    GetLocalTime(&st);

    // 格式化日志内容
    std::wstringstream wss;
    wss << L"[" << std::setfill(L'0') << std::setw(4) << st.wYear << L"-"
        << std::setw(2) << st.wMonth << L"-"
        << std::setw(2) << st.wDay << L" "
        << std::setw(2) << st.wHour << L":"
        << std::setw(2) << st.wMinute << L":"
        << std::setw(2) << st.wSecond << L"] ";
    wss << L"[PID:" << process_id << L"] ";
    wss << L"[Error:" << error_code << L"] ";
    wss << message << L"\n";

    // 写入日志文件(追加模式)
    std::wofstream logFile(logPath, std::ios::app);
    if (logFile.is_open()) {
        logFile << wss.str();
        logFile.close();
    }
}

// ============================================================================
// 进程继承信息管理
// ============================================================================

/**
 * @brief 获取进程继承信息
 */
PROCESS_INHERIT_INFO GetProcessInheritInfo(DWORD process_id) {
    AcquireSRWLockShared(&g_srwLock);

    auto it = g_inheritInfoMap.find(process_id);
    if (it != g_inheritInfoMap.end()) {
        PROCESS_INHERIT_INFO info = it->second;
        ReleaseSRWLockShared(&g_srwLock);
        return info;
    }

    ReleaseSRWLockShared(&g_srwLock);

    // 返回默认值
    PROCESS_INHERIT_INFO defaultInfo = { 0 };
    return defaultInfo;
}

/**
 * @brief 设置进程继承信息
 */
void SetProcessInheritInfo(const PROCESS_INHERIT_INFO& info) {
    AcquireSRWLockExclusive(&g_srwLock);
    g_inheritInfoMap[info.process_id] = info;
    ReleaseSRWLockExclusive(&g_srwLock);
}

/**
 * @brief 清理进程继承信息(进程退出时调用)
 */
void RemoveProcessInheritInfo(DWORD process_id) {
    AcquireSRWLockExclusive(&g_srwLock);
    g_inheritInfoMap.erase(process_id);
    ReleaseSRWLockExclusive(&g_srwLock);
}

// ============================================================================
// DLL 注入实现(使用 CreateRemoteThreadEx)
// ============================================================================

/**
 * @brief 通过远程线程注入 DLL 到目标进程(使用 CreateRemoteThreadEx)
 * @param process_handle 目标进程句柄
 * @param dllPath DLL 路径
 * @return true: 成功, false: 失败
 *
 * 注入步骤:
 * 1. 在目标进程中分配内存
 * 2. 写入 DLL 路径
 * 3. 创建远程线程执行 LoadLibraryW(使用 CreateRemoteThreadEx)
 * 4. 等待远程线程结束
 * 5. 清理资源
 */
static bool InjectDllInternal(HANDLE process_handle, const std::wstring& dllPath) {
    if (!process_handle || dllPath.empty()) {
        return false;
    }

    // 1. 在目标进程中分配内存
    SIZE_T pathSize = (dllPath.length() + 1) * sizeof(wchar_t);
    LPVOID remoteMem = VirtualAllocEx(
        process_handle,
        nullptr,
        pathSize,
        MEM_COMMIT,
        PAGE_READWRITE
    );

    if (!remoteMem) {
        DWORD error = GetLastError();
        WriteInjectErrorLog(GetProcessId(process_handle), error, L"VirtualAllocEx 失败");
        return false;
    }

    // 2. 写入 DLL 路径到目标进程内存
    SIZE_T bytesWritten = 0;
    BOOL writeResult = WriteProcessMemory(
        process_handle,
        remoteMem,
        dllPath.c_str(),
        pathSize,
        &bytesWritten
    );

    if (!writeResult || bytesWritten != pathSize) {
        DWORD error = GetLastError();
        VirtualFreeEx(process_handle, remoteMem, 0, MEM_RELEASE);
        WriteInjectErrorLog(GetProcessId(process_handle), error, L"WriteProcessMemory 失败");
        return false;
    }

    // 3. 获取 LoadLibraryW 函数地址
    HMODULE kernel32 = GetModuleHandleW(L"kernel32.dll");
    if (!kernel32) {
        DWORD error = GetLastError();
        VirtualFreeEx(process_handle, remoteMem, 0, MEM_RELEASE);
        WriteInjectErrorLog(GetProcessId(process_handle), error, L"GetModuleHandleW(kernel32.dll) 失败");
        return false;
    }

    LPTHREAD_START_ROUTINE loadLibraryAddr =
        (LPTHREAD_START_ROUTINE)GetProcAddress(kernel32, "LoadLibraryW");
    if (!loadLibraryAddr) {
        DWORD error = GetLastError();
        VirtualFreeEx(process_handle, remoteMem, 0, MEM_RELEASE);
        WriteInjectErrorLog(GetProcessId(process_handle), error, L"GetProcAddress(LoadLibraryW) 失败");
        return false;
    }

    // 4. 创建远程线程执行 LoadLibraryW(使用 CreateRemoteThreadEx)
    DWORD threadId = 0;
    HANDLE hRemoteThread = CreateRemoteThreadEx(
        process_handle,
        nullptr,  // 安全属性
        0,        // 栈大小(0 = 默认)
        loadLibraryAddr,
        remoteMem,
        0,        // 创建标志
        nullptr,  // 属性列表(可选)
        &threadId
    );

    if (!hRemoteThread) {
        DWORD error = GetLastError();
        VirtualFreeEx(process_handle, remoteMem, 0, MEM_RELEASE);
        WriteInjectErrorLog(GetProcessId(process_handle), error, L"CreateRemoteThreadEx 失败");
        return false;
    }

    // 5. 等待远程线程结束
    WaitForSingleObject(hRemoteThread, INFINITE);

    // 6. 获取线程退出码(LoadLibraryW 返回值 = DLL 模块句柄)
    DWORD exitCode = 0;
    GetExitCodeThread(hRemoteThread, &exitCode);

    // 7. 清理资源
    CloseHandle(hRemoteThread);
    VirtualFreeEx(process_handle, remoteMem, 0, MEM_RELEASE);

    // 8. 检查注入是否成功
    if (exitCode == 0) {
        DWORD error = GetLastError();
        WriteInjectErrorLog(GetProcessId(process_handle), error, L"LoadLibraryW 返回 NULL(注入失败)");
        return false;
    }

    return true;
}

/**
 * @brief 注入 Hook DLL 到目标进程(对外接口)
 * @param process_id 目标进程 ID
 * @param inherit_layer 当前继承层数(从 1 开始,最大 MAX_INHERIT_LAYER)
 * @return true: 成功, false: 失败
 *
 * 功能:
 * 1. 检查继承层数,超过 MAX_INHERIT_LAYER 则不再注入
 * 2. 判断目标进程位数(32/64 位)
 * 3. 根据父子进程位数选择对应的 Hook DLL
 * 4. 调用 InjectDllInternal() 执行注入
 * 5. 记录进程继承信息
 */
bool InjectHookDll(DWORD process_id, int inherit_layer) {
    // 检查继承层数
    if (inherit_layer > MAX_INHERIT_LAYER) {
        WriteInjectErrorLog(process_id, 0, L"继承层数超过限制(" + std::to_wstring(MAX_INHERIT_LAYER) + L" 层)");
        return false;
    }

    // 打开目标进程
    HANDLE hProcess = OpenProcess(
        PROCESS_CREATE_THREAD | PROCESS_VM_OPERATION | PROCESS_VM_WRITE | PROCESS_VM_READ,
        FALSE,
        process_id
    );

    if (!hProcess) {
        DWORD error = GetLastError();
        WriteInjectErrorLog(process_id, error, L"OpenProcess 失败");
        return false;
    }

    // 判断目标进程位数
    bool isTarget64Bit = IsProcess64Bit(hProcess);

    // 选择 DLL 路径
    std::wstring dllPath;
    if (isTarget64Bit) {
        dllPath = g_dllPath64;  // 注入 64 位 DLL
    } else {
        dllPath = g_dllPath32;  // 注入 32 位 DLL(WOW64 场景)
    }

    if (dllPath.empty()) {
        WriteInjectErrorLog(process_id, 0, L"DLL 路径为空(未初始化或缺少对应位数的 DLL)");
        CloseHandle(hProcess);
        return false;
    }

    // 执行注入
    bool result = InjectDllInternal(hProcess, dllPath);

    // 记录进程继承信息
    PROCESS_INHERIT_INFO info;
    info.process_id = process_id;
    info.parent_process_id = GetCurrentProcessId();
    info.inherit_layer = inherit_layer;
    info.inject_status = result ? TRUE : FALSE;
    info.error_code = result ? 0 : GetLastError();

    SetProcessInheritInfo(info);

    CloseHandle(hProcess);
    return result;
}

// ============================================================================
// 公开 API 实现
// ============================================================================

bool Initialize(const std::wstring& dllPath) {
    // 默认同时设置为 64 位和 32 位 DLL 路径(相同路径)
    return InitializeEx(dllPath, dllPath);
}

bool InitializeEx(const std::wstring& dllPath64, const std::wstring& dllPath32) {
    AcquireSRWLockExclusive(&g_srwLock);

    if (g_initialized) {
        ReleaseSRWLockExclusive(&g_srwLock);
        return true;  // 已经初始化
    }

    // 保存 DLL 路径
    g_dllPath64 = dllPath64;
    g_dllPath32 = dllPath32;

    // 获取当前模块句柄
    GetModuleHandleExW(
        GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS,
        (LPCWSTR)&Initialize,
        &g_currentModule
    );

    g_initialized = true;

    ReleaseSRWLockExclusive(&g_srwLock);
    return true;
}

// InstallHooks 和 UninstallHooks 已移除
// 进程继承模块不再自行 Hook，而是由 hook_engine.cpp 调用
// 保留 Initialize/Uninitialize 用于初始化/清理资源

void Uninitialize() {
    AcquireSRWLockExclusive(&g_srwLock);

    g_initialized = false;
    g_dllPath64.clear();
    g_dllPath32.clear();
    g_currentModule = nullptr;
    g_inheritInfoMap.clear();

    ReleaseSRWLockExclusive(&g_srwLock);
}

std::wstring GetCurrentDllPath() {
    if (g_currentModule) {
        return GetModulePath(g_currentModule);
    }
    return L"";
}

} // namespace ProcessInherit

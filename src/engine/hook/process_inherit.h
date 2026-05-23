/**
 * @file process_inherit.h
 * @brief 进程继承引擎头文件 - 子进程 Hook 继承（Week 3 完善版）
 * @author AI ThinApp Team
 * @date 2026-05-23
 *
 * 基于 MinHook 实现进程创建 API Hook，
 * 实现子进程自动注入 Hook DLL 逻辑。
 * 
 * Week 3 改进：
 * 1. 支持 5 层子进程继承
 * 2. 处理 WOW64 场景（32/64 位进程交叉）
 * 3. 使用 CreateRemoteThreadEx 提升注入成功率
 * 4. 完善的错误处理和日志记录
 */

#ifndef PROCESS_INHERIT_H
#define PROCESS_INHERIT_H

#include <Windows.h>
#include <winternl.h>
#include <string>
#include <map>
#include <vector>
#include <mutex>

// MinHook 头文件
#include <MinHook.h>

/**
 * @brief 最大继承层数（防止无限递归）
 */
constexpr int MAX_INHERIT_LAYER = 5;

/**
 * @brief 进程继承信息结构体
 */
struct PROCESS_INHERIT_INFO {
    DWORD process_id;       // 进程 ID
    DWORD parent_process_id; // 父进程 ID
    int inherit_layer;      // 继承层数（从 1 开始）
    BOOL inject_status;     // 注入状态（TRUE: 成功, FALSE: 失败）
    DWORD error_code;       // 错误代码（0 表示无错误）
};

/**
 * @brief 进程继承引擎命名空间
 */
namespace ProcessInherit {

/**
 * @brief 初始化进程继承引擎
 * @param dllPath Hook DLL 路径（用于注入子进程）
 * @return true: 成功, false: 失败
 */
bool Initialize(const std::wstring& dllPath);

/**
 * @brief 初始化进程继承引擎（重载，支持 32/64 位 DLL）
 * @param dllPath64 64 位 Hook DLL 路径
 * @param dllPath32 32 位 Hook DLL 路径（WOW64 场景使用）
 * @return true: 成功, false: 失败
 */
bool InitializeEx(const std::wstring& dllPath64, const std::wstring& dllPath32);

/**
 * @brief 安装所有进程创建 Hook
 * @return true: 成功, false: 失败
 */
bool InstallHooks();

/**
 * @brief 卸载所有进程创建 Hook
 * @return true: 成功, false: 失败
 */
bool UninstallHooks();

/**
 * @brief 清理进程继承引擎资源
 */
void Uninitialize();

/**
 * @brief 注入 Hook DLL 到目标进程
 * @param process_id 目标进程 ID
 * @param inherit_layer 当前继承层数（从 1 开始，最大 MAX_INHERIT_LAYER）
 * @return true: 成功, false: 失败
 * 
 * 使用 CreateRemoteThreadEx 注入，支持 WOW64 场景
 */
bool InjectHookDll(DWORD process_id, int inherit_layer);

/**
 * @brief 判断目标进程是否为 WOW64 进程（32 位进程运行在 64 位系统上）
 * @param process_handle 目标进程句柄
 * @return TRUE: 是 WOW64 进程, FALSE: 不是或失败
 */
BOOL IsWow64Process(HANDLE process_handle);

/**
 * @brief 判断当前进程是否为 64 位进程
 * @return true: 64 位进程, false: 32 位进程
 */
bool IsCurrentProcess64Bit();

/**
 * @brief 获取当前模块的 DLL 路径
 * @return DLL 路径
 */
std::wstring GetCurrentDllPath();

/**
 * @brief 获取进程继承信息
 * @param process_id 进程 ID
 * @return 进程继承信息（如果不存在则返回默认值）
 */
PROCESS_INHERIT_INFO GetProcessInheritInfo(DWORD process_id);

/**
 * @brief 设置进程继承信息
 * @param info 进程继承信息
 */
void SetProcessInheritInfo(const PROCESS_INHERIT_INFO& info);

/**
 * @brief 清理进程继承信息（进程退出时调用）
 * @param process_id 进程 ID
 */
void RemoveProcessInheritInfo(DWORD process_id);

/**
 * @brief 写入注入错误日志
 * @param process_id 进程 ID
 * @param error_code 错误代码
 * @param message 错误信息
 */
void WriteInjectErrorLog(DWORD process_id, DWORD error_code, const std::wstring& message);

} // namespace ProcessInherit

#endif // PROCESS_INHERIT_H

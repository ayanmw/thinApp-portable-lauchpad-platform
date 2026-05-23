/**
 * @file process_inherit.h
 * @brief 进程继承引擎头文件 - 子进程 Hook 继承
 * @author AI ThinApp Team
 * @date 2026-05-23
 * 
 * 基于 MinHook 实现进程创建 API Hook，
 * 实现子进程自动注入 Hook DLL 逻辑。 */

#ifndef PROCESS_INHERIT_H
#define PROCESS_INHERIT_H

#include <Windows.h>
#include <winternl.h>
#include <string>
#include <mutex>

// MinHook 头文件
#include <MinHook.h>

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
void Cleanup();

/**
 * @brief 注入 DLL 到目标进程
 * @param processHandle 目标进程句柄
 * @param dllPath DLL 路径
 * @return true: 成功, false: 失败
 */
bool InjectDll(HANDLE processHandle, const std::wstring& dllPath);

/**
 * @brief 获取当前模块的 DLL 路径
 * @return DLL 路径
 */
std::wstring GetCurrentDllPath();

} // namespace ProcessInherit

#endif // PROCESS_INHERIT_H

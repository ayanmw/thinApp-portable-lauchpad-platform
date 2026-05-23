/**
 * @file process_inherit.h
 * @brief 进程隔离与子进程 Hook 继承模块头文件
 * 
 * 实现子进程 Hook 继承，确保父进程的 Hook 传递到子进程。
 * 支持多层子进程继承（覆盖 Chrome 多进程等场景）。
 * 
 * @author AI ThinApp Team
 * @date 2026-05-23
 */

#ifndef PROCESS_INHERIT_H
#define PROCESS_INHERIT_H

#include <Windows.h>
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief 初始化进程隔离模块
 * 
 * 初始化子进程 Hook 继承功能。
 * 
 * @return TRUE: 初始化成功; FALSE: 初始化失败
 */
BOOL ProcessInherit_Initialize(void);

/**
 * @brief 清理进程隔离模块
 * 
 * 清理子进程 Hook 继承功能，释放所有资源。
 * 
 * @return TRUE: 清理成功; FALSE: 清理失败
 */
BOOL ProcessInherit_Shutdown(void);

/**
 * @brief 启用子进程 Hook 继承
 * 
 * 拦截 CreateProcess 系列 API，在子进程中注入 Hook DLL。
 * 
 * @return TRUE: 启用成功; FALSE: 启用失败
 */
BOOL ProcessInherit_Enable(void);

/**
 * @brief 禁用子进程 Hook 继承
 * 
 * 停止拦截 CreateProcess 系列 API。
 * 
 * @return TRUE: 禁用成功; FALSE: 禁用失败
 */
BOOL ProcessInherit_Disable(void);

/**
 * @brief 注入 Hook DLL 到目标进程
 * 
 * 使用远程线程注入技术将 Hook DLL 注入到目标进程。
 * 
 * @param processHandle 目标进程句柄
 * @param dllPath Hook DLL 路径（UTF-16）
 * @return TRUE: 注入成功; FALSE: 注入失败
 */
BOOL ProcessInherit_InjectDll(HANDLE processHandle, LPCWSTR dllPath);

/**
 * @brief 处理进程创建（Hook 回调函数）
 * 
 * CreateProcess Hook 回调函数，用于在子进程中注入 Hook DLL。
 * 
 * @param processInfo 新创建的进程信息
 * @return TRUE: 处理成功; FALSE: 处理失败
 */
BOOL ProcessInherit_OnProcessCreated(PROCESS_INFORMATION* processInfo);

#ifdef __cplusplus
}
#endif

#endif // PROCESS_INHERIT_H

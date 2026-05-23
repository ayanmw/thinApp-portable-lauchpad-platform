/**
 * @file hook_engine.h
 * @brief Hook 引擎模块头文件
 * 
 * 基于 MinHook 实现 API Hook，用于拦截文件系统、注册表和进程创建相关 API。
 * 实现应用沙箱的文件系统和注册表虚拟化。
 * 
 * @author AI ThinApp Team
 * @date 2026-05-23
 */

#ifndef HOOK_ENGINE_H
#define HOOK_ENGINE_H

#include <Windows.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief 初始化 Hook 引擎
 * 
 * 初始化 MinHook 库，准备 Hook 环境。
 * 
 * @return TRUE: 初始化成功; FALSE: 初始化失败
 */
BOOL HookEngine_Initialize(void);

/**
 * @brief 安装所有预定义的 Hook
 * 
 * 安装文件系统、注册表和进程创建相关的 API Hook。
 * 
 * @return TRUE: 安装成功; FALSE: 安装失败
 */
BOOL HookEngine_InstallHooks(void);

/**
 * @brief 卸载所有 Hook
 * 
 * 卸载所有已安装的 Hook，恢复原始 API 函数。
 * 
 * @return TRUE: 卸载成功; FALSE: 卸载失败
 */
BOOL HookEngine_UninstallHooks(void);

/**
 * @brief 清理 Hook 引擎
 * 
 * 卸载所有 Hook 并清理 MinHook 库。
 * 
 * @return TRUE: 清理成功; FALSE: 清理失败
 */
BOOL HookEngine_Shutdown(void);

/**
 * @brief 检查 Hook 引擎是否已初始化
 * 
 * @return TRUE: 已初始化; FALSE: 未初始化
 */
BOOL HookEngine_IsInitialized(void);

#ifdef __cplusplus
}
#endif

#endif // HOOK_ENGINE_H

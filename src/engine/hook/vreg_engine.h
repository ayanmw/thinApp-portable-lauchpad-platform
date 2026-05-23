/**
 * @file vreg_engine.h
 * @brief 虚拟注册表引擎头文件 - 注册表重定向
 * @author AI ThinApp Team
 * @date 2026-05-23
 * 
 * 基于 MinHook 实现注册表 API Hook，
 * 将目标注册表路径重定向到 VFS 虚拟 hive。 */

#ifndef VREG_ENGINE_H
#define VREG_ENGINE_H

#include <Windows.h>
#include <winternl.h>
#include <string>
#include <vector>
#include <mutex>

// MinHook 头文件
#include <MinHook.h>

/**
 * @brief 虚拟注册表引擎命名空间
 */
namespace VRegEngine {

/**
 * @brief 初始化虚拟注册表引擎
 * @param appDir 应用目录（VFS 根目录）
 * @return true: 成功, false: 失败
 */
bool Initialize(const std::wstring& appDir);

/**
 * @brief 安装所有注册表 Hook
 * @return true: 成功, false: 失败
 */
bool InstallHooks();

/**
 * @brief 卸载所有注册表 Hook
 * @return true: 成功, false: 失败
 */
bool UninstallHooks();

/**
 * @brief 清理虚拟注册表引擎资源
 */
void Cleanup();

/**
 * @brief 添加注册表重定向规则
 * @param realPath 真实注册表路径（应用视角）
 * @param vfsHivePath VFS 虚拟 hive 路径
 */
void AddRegRedirection(const std::wstring& realPath, const std::wstring& vfsHivePath);

/**
 * @brief 检查注册表路径是否需要重定向
 * @param path 原始注册表路径
 * @param redirectedPath [out] 重定向后的 hive 路径
 * @return true: 需要重定向, false: 不需要
 */
bool ShouldRedirectReg(const std::wstring& path, std::wstring& redirectedPath);

/**
 * @brief 获取虚拟 hive 文件路径
 * @return 虚拟 hive 文件路径
 */
std::wstring GetVirtualHivePath();

} // namespace VRegEngine

#endif // VREG_ENGINE_H

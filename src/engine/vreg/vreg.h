/**
 * @file vreg.h
 * @brief 虚拟注册表（VReg）模块头文件
 * 
 * 实现注册表虚拟化，将应用的注册表操作重定向到沙箱内的虚拟 hive 文件。
 * 支持 Copy-on-Write (CoW) 策略。
 * 
 * @author AI ThinApp Team
 * @date 2026-05-23
 */

#ifndef VREG_H
#define VREG_H

#include <Windows.h>
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief 初始化 VReg 模块
 * 
 * 初始化虚拟注册表模块，设置虚拟 hive 文件路径。
 * 
 * @param hivePath 虚拟 hive 文件路径（UTF-16）
 * @return TRUE: 初始化成功; FALSE: 初始化失败
 */
BOOL VReg_Initialize(LPCWSTR hivePath);

/**
 * @brief 清理 VReg 模块
 * 
 * 清理虚拟注册表模块，释放所有资源。
 * 
 * @return TRUE: 清理成功; FALSE: 清理失败
 */
BOOL VReg_Shutdown(void);

/**
 * @brief 重定向注册表路径
 * 
 * 将真实注册表路径重定向到沙箱内的虚拟 hive。
 * 例如：HKLM\Software\Test → {VirtualHive}\HKLM\Software\Test
 * 
 * @param originalPath 原始注册表路径（UTF-16）
 * @param redirectedPath 输出缓冲区，接收重定向后的路径
 * @param bufferSize 缓冲区大小（字符数）
 * @return TRUE: 重定向成功; FALSE: 重定向失败或不需要重定向
 */
BOOL VReg_RedirectKeyPath(LPCWSTR originalPath, LPWSTR redirectedPath, DWORD bufferSize);

/**
 * @brief 检查注册表项是否需要虚拟化
 * 
 * 判断指定注册表路径是否需要重定向到虚拟 hive。
 * 
 * @param path 注册表路径（UTF-16）
 * @return TRUE: 需要虚拟化; FALSE: 不需要虚拟化
 */
BOOL VReg_NeedsVirtualization(LPCWSTR path);

/**
 * @brief 创建/打开注册表键（虚拟）
 * 
 * 在虚拟 hive 中创建或打开注册表键。
 * 
 * @param hKey 父键句柄
 * @param lpSubKey 子键名称
 * @param phkResult 输出新键句柄
 * @return TRUE: 成功; FALSE: 失败
 */
BOOL VReg_CreateKey(HKEY hKey, LPCWSTR lpSubKey, PHKEY phkResult);

/**
 * @brief 设置注册表值（虚拟）
 * 
 * 在虚拟 hive 中设置注册表值。
 * 
 * @param hKey 键句柄
 * @param lpValueName 值名称
 * @param dwType 值类型
 * @param lpData 数据缓冲区
 * @param cbData 数据大小
 * @return TRUE: 成功; FALSE: 失败
 */
BOOL VReg_SetValue(HKEY hKey, LPCWSTR lpValueName, DWORD dwType, LPCVOID lpData, DWORD cbData);

/**
 * @brief 查询注册表值（CoW 逻辑）
 * 
 * 实现 Copy-on-Write 查询逻辑：
 * 1. 检查虚拟 hive 中是否存在
 * 2. 若存在，读取虚拟值
 * 3. 若不存在，透传到真实注册表
 * 
 * @param hKey 键句柄
 * @param lpValueName 值名称
 * @param lpType 输出值类型
 * @param lpData 输出数据缓冲区
 * @param lpcbData 输入输出数据大小
 * @return TRUE: 成功; FALSE: 失败
 */
BOOL VReg_QueryValue(HKEY hKey, LPCWSTR lpValueName, LPDWORD lpType, LPVOID lpData, LPDWORD lpcbData);

#ifdef __cplusplus
}
#endif

#endif // VREG_H

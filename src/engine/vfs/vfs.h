/**
 * @file vfs.h
 * @brief 虚拟文件系统（VFS）模块头文件
 * 
 * 实现文件系统虚拟化，将应用的文件操作重定向到沙箱目录。
 * 支持 Copy-on-Write (CoW) 策略。
 * 
 * @author AI ThinApp Team
 * @date 2026-05-23
 */

#ifndef VFS_H
#define VFS_H

#include <Windows.h>
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief 初始化 VFS 模块
 * 
 * 初始化虚拟文件系统模块，设置沙箱根目录。
 * 
 * @param sandboxRoot 沙箱根目录路径（UTF-16）
 * @return TRUE: 初始化成功; FALSE: 初始化失败
 */
BOOL VFS_Initialize(LPCWSTR sandboxRoot);

/**
 * @brief 清理 VFS 模块
 * 
 * 清理虚拟文件系统模块，释放所有资源。
 * 
 * @return TRUE: 清理成功; FALSE: 清理失败
 */
BOOL VFS_Shutdown(void);

/**
 * @brief 重定向文件路径
 * 
 * 将真实文件系统路径重定向到沙箱内的虚拟路径。
 * 例如：C:\Windows\Temp\test.txt → {SandboxRoot}\VFS\C\Windows\Temp\test.txt
 * 
 * @param originalPath 原始路径（UTF-16）
 * @param redirectedPath 输出缓冲区，接收重定向后的路径
 * @param bufferSize 缓冲区大小（字符数）
 * @return TRUE: 重定向成功; FALSE: 重定向失败或不需要重定向
 */
BOOL VFS_RedirectPath(LPCWSTR originalPath, LPWSTR redirectedPath, DWORD bufferSize);

/**
 * @brief 检查文件是否需要虚拟化
 * 
 * 判断指定路径是否需要重定向到沙箱。
 * 
 * @param path 文件路径（UTF-16）
 * @return TRUE: 需要虚拟化; FALSE: 不需要虚拟化
 */
BOOL VFS_NeedsVirtualization(LPCWSTR path);

/**
 * @brief 读取文件（CoW 逻辑）
 * 
 * 实现 Copy-on-Write 读取逻辑：
 * 1. 检查虚拟文件是否存在
 * 2. 若存在，读取虚拟文件
 * 3. 若不存在，透传到真实文件
 * 
 * @param path 文件路径（UTF-16）
 * @param buffer 输出缓冲区
 * @param bufferSize 缓冲区大小
 * @param bytesRead 实际读取的字节数
 * @return TRUE: 读取成功; FALSE: 读取失败
 */
BOOL VFS_ReadFile(LPCWSTR path, LPVOID buffer, DWORD bufferSize, LPDWORD bytesRead);

/**
 * @brief 写入文件（重定向到沙箱）
 * 
 * 将文件写入操作重定向到沙箱目录。
 * 
 * @param path 文件路径（UTF-16）
 * @param buffer 数据缓冲区
 * @param bufferSize 数据大小
 * @param bytesWritten 实际写入的字节数
 * @return TRUE: 写入成功; FALSE: 写入失败
 */
BOOL VFS_WriteFile(LPCWSTR path, LPCVOID buffer, DWORD bufferSize, LPDWORD bytesWritten);

#ifdef __cplusplus
}
#endif

#endif // VFS_H

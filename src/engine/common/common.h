/**
 * @file common.h
 * @brief 引擎公共工具模块头文件
 * 
 * 提供引擎各模块共用的工具函数和类型定义。
 * 包括字符串处理、路径操作、日志等功能。
 * 
 * @author AI ThinApp Team
 * @date 2026-05-23
 */

#ifndef COMMON_H
#define COMMON_H

#include <Windows.h>
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief 日志级别
 */
typedef enum {
    LOG_LEVEL_DEBUG = 0,
    LOG_LEVEL_INFO = 1,
    LOG_LEVEL_WARN = 2,
    LOG_LEVEL_ERROR = 3
} LogLevel;

/**
 * @brief 初始化公共工具模块
 * 
 * @return TRUE: 初始化成功; FALSE: 初始化失败
 */
BOOL Common_Initialize(void);

/**
 * @brief 清理公共工具模块
 * 
 * @return TRUE: 清理成功; FALSE: 清理失败
 */
BOOL Common_Shutdown(void);

/**
 * @brief 设置日志级别
 * 
 * @param level 日志级别
 */
void Common_SetLogLevel(LogLevel level);

/**
 * @brief 打印日志
 * 
 * @param level 日志级别
 * @param format 格式化字符串
 * @param ... 可变参数
 */
void Common_Log(LogLevel level, LPCWSTR format, ...);

/**
 * @brief 转换路径为规范形式
 * 
 * 将路径转换为规范形式（去除相对路径成分、统一分隔符等）。
 * 
 * @param path 输入路径（UTF-16）
 * @param normalizedPath 输出缓冲区，接收规范路径
 * @param bufferSize 缓冲区大小（字符数）
 * @return TRUE: 转换成功; FALSE: 转换失败
 */
BOOL Common_NormalizePath(LPCWSTR path, LPWSTR normalizedPath, DWORD bufferSize);

/**
 * @brief 拼接路径
 * 
 * 拼接两个路径组件，自动处理分隔符。
 * 
 * @param path1 第一个路径组件（UTF-16）
 * @param path2 第二个路径组件（UTF-16）
 * @param resultPath 输出缓冲区，接收拼接后的路径
 * @param bufferSize 缓冲区大小（字符数）
 * @return TRUE: 拼接成功; FALSE: 拼接失败
 */
BOOL Common_JoinPath(LPCWSTR path1, LPCWSTR path2, LPWSTR resultPath, DWORD bufferSize);

/**
 * @brief 检查文件是否存在
 * 
 * @param path 文件路径（UTF-16）
 * @return TRUE: 文件存在; FALSE: 文件不存在
 */
BOOL Common_FileExists(LPCWSTR path);

/**
 * @brief 创建目录（递归）
 * 
 * @param path 目录路径（UTF-16）
 * @return TRUE: 创建成功; FALSE: 创建失败
 */
BOOL Common_CreateDirectoryRecursive(LPCWSTR path);

/**
 * @brief 获取当前可执行文件路径
 * 
 * @param buffer 输出缓冲区
 * @param bufferSize 缓冲区大小（字符数）
 * @return TRUE: 获取成功; FALSE: 获取失败
 */
BOOL Common_GetCurrentModulePath(LPWSTR buffer, DWORD bufferSize);

#ifdef __cplusplus
}
#endif

#endif // COMMON_H

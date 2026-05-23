/**
 * @file common.cpp
 * @brief 引擎公共工具模块源文件
 * 
 * 提供引擎各模块共用的工具函数和类型定义。
 * 包括字符串处理、路径操作、日志等功能。
 * 
 * @author AI ThinApp Team
 * @date 2026-05-23
 */

#include "common.h"
#include <stdio.h>
#include <string.h>
#include <stdarg.h>

// 全局变量
static BOOL g_Initialized = FALSE;
static LogLevel g_LogLevel = LOG_LEVEL_INFO;

/**
 * @brief 初始化公共工具模块
 */
BOOL Common_Initialize(void)
{
    if (g_Initialized) {
        printf("[Common] 警告: 已经初始化\n");
        return TRUE;
    }
    
    // TODO: 初始化公共工具模块
    printf("[Common] 提示: 公共工具模块初始化未实现（桩）\n");
    
    g_Initialized = TRUE;
    printf("[Common] 初始化成功\n");
    
    return TRUE;
}

/**
 * @brief 清理公共工具模块
 */
BOOL Common_Shutdown(void)
{
    if (!g_Initialized) {
        printf("[Common] 警告: 未初始化\n");
        return TRUE;
    }
    
    // 清理资源
    g_Initialized = FALSE;
    
    printf("[Common] 清理完成\n");
    
    return TRUE;
}

/**
 * @brief 设置日志级别
 */
void Common_SetLogLevel(LogLevel level)
{
    g_LogLevel = level;
    printf("[Common] 日志级别设置为: %d\n", level);
}

/**
 * @brief 打印日志
 */
void Common_Log(LogLevel level, LPCWSTR format, ...)
{
    if (level < g_LogLevel) {
        return; // 日志级别不够，不打印
    }
    
    // 日志级别前缀
    LPCWSTR levelPrefix = L"";
    switch (level) {
        case LOG_LEVEL_DEBUG:
            levelPrefix = L"[DEBUG] ";
            break;
        case LOG_LEVEL_INFO:
            levelPrefix = L"[INFO] ";
            break;
        case LOG_LEVEL_WARN:
            levelPrefix = L"[WARN] ";
            break;
        case LOG_LEVEL_ERROR:
            levelPrefix = L"[ERROR] ";
            break;
    }
    
    // 打印日志级别前缀
    wprintf(L"%ls", levelPrefix);
    
    // 打印格式化消息
    va_list args;
    va_start(args, format);
    vfwprintf(stdout, format, args);
    va_end(args);
    
    wprintf(L"\n");
}

/**
 * @brief 转换路径为规范形式
 */
BOOL Common_NormalizePath(LPCWSTR path, LPWSTR normalizedPath, DWORD bufferSize)
{
    if (path == NULL || normalizedPath == NULL || bufferSize == 0) {
        printf("[Common] 错误: 参数无效\n");
        return FALSE;
    }
    
    // TODO: 实现路径规范化
    printf("[Common] 提示: 路径规范化未实现（桩）\n");
    printf("[Common]   输入路径: %ls\n", path);
    
    // 桩实现：直接复制
    wcsncpy_s(normalizedPath, bufferSize, path, bufferSize - 1);
    
    return TRUE;
}

/**
 * @brief 拼接路径
 */
BOOL Common_JoinPath(LPCWSTR path1, LPCWSTR path2, LPWSTR resultPath, DWORD bufferSize)
{
    if (path1 == NULL || path2 == NULL || resultPath == NULL || bufferSize == 0) {
        printf("[Common] 错误: 参数无效\n");
        return FALSE;
    }
    
    // TODO: 实现路径拼接
    printf("[Common] 提示: 路径拼接未实现（桩）\n");
    printf("[Common]   path1: %ls, path2: %ls\n", path1, path2);
    
    // 桩实现：简单拼接（不处理分隔符）
    wcsncpy_s(resultPath, bufferSize, path1, bufferSize - 1);
    wcsncat_s(resultPath, bufferSize, path2, bufferSize - 1 - wcslen(resultPath));
    
    return TRUE;
}

/**
 * @brief 检查文件是否存在
 */
BOOL Common_FileExists(LPCWSTR path)
{
    if (path == NULL) {
        return FALSE;
    }
    
    // 使用 GetFileAttributesW 检查文件是否存在
    DWORD attrib = GetFileAttributesW(path);
    if (attrib == INVALID_FILE_ATTRIBUTES) {
        return FALSE;
    }
    
    return TRUE;
}

/**
 * @brief 创建目录（递归）
 */
BOOL Common_CreateDirectoryRecursive(LPCWSTR path)
{
    if (path == NULL || wcslen(path) == 0) {
        printf("[Common] 错误: 路径为空\n");
        return FALSE;
    }
    
    // TODO: 实现递归创建目录
    printf("[Common] 提示: 递归创建目录未实现（桩）\n");
    printf("[Common]   路径: %ls\n", path);
    
    return FALSE;
}

/**
 * @brief 获取当前可执行文件路径
 */
BOOL Common_GetCurrentModulePath(LPWSTR buffer, DWORD bufferSize)
{
    if (buffer == NULL || bufferSize == 0) {
        printf("[Common] 错误: 参数无效\n");
        return FALSE;
    }
    
    // 获取当前模块路径
    HMODULE hModule = NULL;
    if (GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS, 
                           (LPCWSTR)&Common_GetCurrentModulePath, 
                           &hModule) == FALSE) {
        printf("[Common] 错误: 无法获取模块句柄\n");
        return FALSE;
    }
    
    DWORD result = GetModuleFileNameW(hModule, buffer, bufferSize);
    if (result == 0 || result == bufferSize) {
        printf("[Common] 错误: 无法获取模块文件名\n");
        return FALSE;
    }
    
    return TRUE;
}

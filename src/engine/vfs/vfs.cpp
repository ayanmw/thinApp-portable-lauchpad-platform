/**
 * @file vfs.cpp
 * @brief 虚拟文件系统（VFS）模块源文件 - 完整实现
 * 
 * 实现文件系统虚拟化，将应用的文件操作重定向到沙箱目录。
 * 支持 Copy-on-Write (CoW) 策略和路径缓存。
 * 
 * @author AI ThinApp Team
 * @date 2026-05-23
 */

#include "vfs.h"
#include "../common/common.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <shlwapi.h>
#include <unordered_map>
#include <list>
#include <mutex>
#include <memory>

#pragma comment(lib, "Shlwapi.lib")

// 全局变量
static WCHAR g_SandboxRoot[MAX_PATH] = { 0 };
static BOOL g_Initialized = FALSE;

// 虚拟化路径列表
static const LPCWSTR VIRTUALIZABLE_PATHS[] = {
    L"C:\\Windows\\",
    L"C:\\Program Files\\",
    L"C:\\Program Files (x86)\\",
    L"%APPDATA%\\",
    L"%LOCALAPPDATA%\\",
    L"%PROGRAMDATA%\\"
};

// ============================================================================
// VFS 缓存实现（LRU + TTL）
// ============================================================================

struct CacheEntry {
    std::wstring redirectedPath;
    DWORD timestamp;
};

static const size_t MAX_CACHE_SIZE = 1000;
static const DWORD CACHE_TTL_MS = 60000; // 1 分钟

static std::unordered_map<std::wstring, CacheEntry> g_PathCache;
static std::list<std::wstring> g_LRUList;
static std::mutex g_CacheMutex;

/**
 * @brief 检查缓存
 */
static BOOL VFS_GetFromCache(LPCWSTR originalPath, LPWSTR redirectedPath, DWORD bufferSize) {
    std::lock_guard<std::mutex> lock(g_CacheMutex);
    
    auto it = g_PathCache.find(originalPath);
    if (it != g_PathCache.end()) {
        // 检查 TTL
        if (GetTickCount() - it->second.timestamp < CACHE_TTL_MS) {
            // 缓存命中
            g_LRUList.remove(originalPath);
            g_LRUList.push_back(originalPath);
            
            wcsncpy_s(redirectedPath, bufferSize, it->second.redirectedPath.c_str(), bufferSize - 1);
            return TRUE;
        } else {
            // 缓存过期
            g_PathCache.erase(it);
            g_LRUList.remove(originalPath);
        }
    }
    
    return FALSE;
}

/**
 * @brief 写入缓存
 */
static void VFS_AddToCache(LPCWSTR originalPath, LPCWSTR redirectedPath) {
    std::lock_guard<std::mutex> lock(g_CacheMutex);
    
    CacheEntry entry;
    entry.redirectedPath = redirectedPath;
    entry.timestamp = GetTickCount();
    
    g_PathCache[originalPath] = entry;
    g_LRUList.push_back(originalPath);
    
    // 检查缓存大小
    if (g_PathCache.size() > MAX_CACHE_SIZE) {
        std::wstring oldest = g_LRUList.front();
        g_LRUList.pop_front();
        g_PathCache.erase(oldest);
    }
}

// ============================================================================
// 安全检查函数
// ============================================================================

/**
 * @brief 检查路径遍历攻击
 * 
 * 验证路径不包含 `..\` 或其他遍历模式，
 * 并确保路径在沙箱目录内。
 * 
 * @param path 待检查的路径
 * @return TRUE: 路径安全; FALSE: 检测到路径遍历攻击
 */
static BOOL VFS_PathTraversalCheck(LPCWSTR path) {
    if (!path) {
        return FALSE;
    }
    
    // 检查是否包含遍历模式
    if (wcsstr(path, L"..\\") != NULL || wcsstr(path, L"../") != NULL) {
        printf("[VFS] 安全警告: 检测到路径遍历攻击: %ls\n", path);
        return FALSE;
    }
    
    // 规范化路径
    WCHAR normalizedPath[MAX_PATH];
    if (GetFullPathNameW(path, MAX_PATH, normalizedPath, NULL) == 0) {
        return FALSE;
    }
    
    // 检查是否在沙箱根目录内
    if (g_Initialized && g_SandboxRoot[0] != L'\0') {
        size_t sandboxLen = wcslen(g_SandboxRoot);
        if (wcsncmp(normalizedPath, g_SandboxRoot, sandboxLen) != 0) {
            printf("[VFS] 安全警告: 路径超出沙箱范围: %ls\n", normalizedPath);
            return FALSE;
        }
    }
    
    return TRUE;
}

// ============================================================================
// 路径操作辅助函数
// ============================================================================

/**
 * @brief 规范化路径
 */
static BOOL VFS_NormalizePath(LPCWSTR path, LPWSTR normalizedPath, DWORD bufferSize) {
    if (!path || !normalizedPath || bufferSize == 0) {
        return FALSE;
    }
    
    // 使用 GetFullPathNameW 规范化路径
    WCHAR fullPath[MAX_PATH];
    if (GetFullPathNameW(path, MAX_PATH, fullPath, NULL) == 0) {
        return FALSE;
    }
    
    // 转换为大写以便比较
    CharUpperW(fullPath);
    
    wcsncpy_s(normalizedPath, bufferSize, fullPath, bufferSize - 1);
    return TRUE;
}

/**
 * @brief 构造 VFS 重定向路径
 * 例如：C:\Windows\Temp\test.txt -> {SandboxRoot}\VFS\C\Windows\Temp\test.txt
 */
static BOOL VFS_BuildRedirectedPath(LPCWSTR originalPath, LPWSTR redirectedPath, DWORD bufferSize) {
    if (!originalPath || !redirectedPath || bufferSize == 0) {
        return FALSE;
    }
    
    // 规范化路径
    WCHAR normalizedPath[MAX_PATH];
    if (!VFS_NormalizePath(originalPath, normalizedPath, MAX_PATH)) {
        return FALSE;
    }
    
    // 提取盘符和剩余路径
    WCHAR driveLetter[3] = { 0 };
    WCHAR remainingPath[MAX_PATH] = { 0 };
    
    if (normalizedPath[1] == L':') {
        wcsncpy_s(driveLetter, 3, normalizedPath, 2);
        wcsncpy_s(remainingPath, MAX_PATH, normalizedPath + 3, MAX_PATH - 1);
    } else {
        // UNC 路径暂不支持
        return FALSE;
    }
    
    // 构造重定向路径: {SandboxRoot}\VFS\{driveLetter}\{remainingPath}
    swprintf_s(redirectedPath, bufferSize, L"%lsVFS\\%ls\\%ls",
               g_SandboxRoot, driveLetter, remainingPath);
    
    return TRUE;
}

// ============================================================================
// 公开 API 实现
// ============================================================================

/**
 * @brief 初始化 VFS 模块
 */
BOOL VFS_Initialize(LPCWSTR sandboxRoot) {
    if (g_Initialized) {
        printf("[VFS] 警告: 已经初始化\n");
        return TRUE;
    }
    
    if (sandboxRoot == NULL || wcslen(sandboxRoot) == 0) {
        printf("[VFS] 错误: 沙箱根目录为空\n");
        return FALSE;
    }
    
    // 保存沙箱根目录
    wcsncpy_s(g_SandboxRoot, MAX_PATH, sandboxRoot, MAX_PATH - 1);
    
    // 确保路径以反斜杠结尾
    size_t len = wcslen(g_SandboxRoot);
    if (len > 0 && g_SandboxRoot[len - 1] != L'\\') {
        wcsncat_s(g_SandboxRoot, MAX_PATH, L"\\", 1);
    }
    
    // 创建 VFS 目录
    WCHAR vfsDir[MAX_PATH];
    swprintf_s(vfsDir, MAX_PATH, L"%lsVFS", g_SandboxRoot);
    CreateDirectoryW(vfsDir, NULL);
    
    g_Initialized = TRUE;
    printf("[VFS] 初始化成功，沙箱根目录: %ls\n", g_SandboxRoot);
    
    return TRUE;
}

/**
 * @brief 清理 VFS 模块
 */
BOOL VFS_Shutdown(void) {
    if (!g_Initialized) {
        printf("[VFS] 警告: 未初始化\n");
        return TRUE;
    }
    
    // 清理缓存
    {
        std::lock_guard<std::mutex> lock(g_CacheMutex);
        g_PathCache.clear();
        g_LRUList.clear();
    }
    
    memset(g_SandboxRoot, 0, sizeof(g_SandboxRoot));
    g_Initialized = FALSE;
    
    printf("[VFS] 清理完成\n");
    
    return TRUE;
}

/**
 * @brief 重定向文件路径 - 完整实现
 */
BOOL VFS_RedirectPath(LPCWSTR originalPath, LPWSTR redirectedPath, DWORD bufferSize) {
    if (!g_Initialized) {
        printf("[VFS] 错误: 未初始化\n");
        return FALSE;
    }
    
    if (originalPath == NULL || redirectedPath == NULL || bufferSize == 0) {
        printf("[VFS] 错误: 参数无效\n");
        return FALSE;
    }
    
    // 检查缓存
    if (VFS_GetFromCache(originalPath, redirectedPath, bufferSize)) {
        printf("[VFS] 缓存命中: %ls -> %ls\n", originalPath, redirectedPath);
        return TRUE;
    }
    
    // 安全检查：路径遍历攻击防护
    if (!VFS_PathTraversalCheck(originalPath)) {
        printf("[VFS] 错误: 路径遍历检查失败，拒绝访问: %ls\n", originalPath);
        return FALSE;
    }
    
    // 检查是否需要虚拟化
    if (!VFS_NeedsVirtualization(originalPath)) {
        // 不需要虚拟化，返回 FALSE（透传）
        return FALSE;
    }
    
    // 构造重定向路径
    WCHAR newRedirectedPath[MAX_PATH] = { 0 };
    if (!VFS_BuildRedirectedPath(originalPath, newRedirectedPath, MAX_PATH)) {
        return FALSE;
    }
    
    // 创建目标目录（如果不存在）
    WCHAR dirPath[MAX_PATH];
    wcsncpy_s(dirPath, MAX_PATH, newRedirectedPath, MAX_PATH - 1);
    WCHAR* lastSlash = wcsrchr(dirPath, L'\\');
    if (lastSlash != NULL) {
        *lastSlash = L'\0';
        Common_CreateDirectoryRecursive(dirPath);
    }
    
    // 写入缓存
    VFS_AddToCache(originalPath, newRedirectedPath);
    
    // 返回重定向路径
    wcsncpy_s(redirectedPath, bufferSize, newRedirectedPath, bufferSize - 1);
    
    printf("[VFS] 路径重定向: %ls -> %ls\n", originalPath, redirectedPath);
    
    return TRUE;
}

/**
 * @brief 检查文件是否需要虚拟化 - 完整实现
 */
BOOL VFS_NeedsVirtualization(LPCWSTR path) {
    if (path == NULL) {
        return FALSE;
    }
    
    // 规范化路径
    WCHAR normalizedPath[MAX_PATH];
    if (!VFS_NormalizePath(path, normalizedPath, MAX_PATH)) {
        return FALSE;
    }
    
    // 检查是否在虚拟化列表中
    for (LPCWSTR virtPath : VIRTUALIZABLE_PATHS) {
        WCHAR expandedPath[MAX_PATH];
        ExpandEnvironmentStringsW(virtPath, expandedPath, MAX_PATH);
        
        WCHAR upperExpandedPath[MAX_PATH];
        wcsncpy_s(upperExpandedPath, MAX_PATH, expandedPath, MAX_PATH - 1);
        CharUpperW(upperExpandedPath);
        
        if (wcsstr(normalizedPath, upperExpandedPath) != NULL) {
            return TRUE;
        }
    }
    
    return FALSE;
}

/**
 * @brief 读取文件（CoW 逻辑）- 完整实现
 */
BOOL VFS_ReadFile(LPCWSTR path, LPVOID buffer, DWORD bufferSize, LPDWORD bytesRead) {
    if (!g_Initialized) {
        printf("[VFS] 错误: 未初始化\n");
        return FALSE;
    }
    
    if (path == NULL || buffer == NULL || bytesRead == NULL) {
        printf("[VFS] 错误: 参数无效\n");
        return FALSE;
    }
    
    *bytesRead = 0;
    
    // 尝试重定向路径
    WCHAR redirectedPath[MAX_PATH] = { 0 };
    BOOL isRedirected = VFS_RedirectPath(path, redirectedPath, MAX_PATH);
    
    if (isRedirected && Common_FileExists(redirectedPath)) {
        // CoW 逻辑：虚拟文件存在，读取虚拟文件
        HANDLE hFile = CreateFileW(redirectedPath, GENERIC_READ, FILE_SHARE_READ,
                                   NULL, OPEN_EXISTING, 0, NULL);
        if (hFile != INVALID_HANDLE_VALUE) {
            BOOL result = ReadFile(hFile, buffer, bufferSize, bytesRead, NULL);
            CloseHandle(hFile);
            
            printf("[VFS] CoW 读取虚拟文件: %ls (%lu 字节)\n", redirectedPath, *bytesRead);
            return result;
        }
    }
    
    // 虚拟文件不存在或重定向失败，透传到真实文件
    HANDLE hFile = CreateFileW(path, GENERIC_READ, FILE_SHARE_READ,
                               NULL, OPEN_EXISTING, 0, NULL);
    if (hFile == INVALID_HANDLE_VALUE) {
        printf("[VFS] 错误: 无法打开文件: %ls\n", path);
        return FALSE;
    }
    
    BOOL result = ReadFile(hFile, buffer, bufferSize, bytesRead, NULL);
    CloseHandle(hFile);
    
    if (result) {
        printf("[VFS] 透传读取真实文件: %ls (%lu 字节)\n", path, *bytesRead);
    }
    
    return result;
}

/**
 * @brief 写入文件（重定向到沙箱）- 完整实现
 */
BOOL VFS_WriteFile(LPCWSTR path, LPCVOID buffer, DWORD bufferSize, LPDWORD bytesWritten) {
    if (!g_Initialized) {
        printf("[VFS] 错误: 未初始化\n");
        return FALSE;
    }
    
    if (path == NULL || buffer == NULL || bytesWritten == NULL) {
        printf("[VFS] 错误: 参数无效\n");
        return FALSE;
    }
    
    *bytesWritten = 0;
    
    // 尝试重定向路径
    WCHAR redirectedPath[MAX_PATH] = { 0 };
    BOOL isRedirected = VFS_RedirectPath(path, redirectedPath, MAX_PATH);
    
    if (isRedirected) {
        // 写入到虚拟文件（不修改真实文件）
        HANDLE hFile = CreateFileW(redirectedPath, GENERIC_WRITE, 0,
                                   NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
        if (hFile == INVALID_HANDLE_VALUE) {
            printf("[VFS] 错误: 无法创建虚拟文件: %ls\n", redirectedPath);
            return FALSE;
        }
        
        BOOL result = WriteFile(hFile, buffer, bufferSize, bytesWritten, NULL);
        CloseHandle(hFile);
        
        if (result) {
            printf("[VFS] 写入虚拟文件: %ls (%lu 字节)\n", redirectedPath, *bytesWritten);
        }
        
        return result;
    } else {
        // 不需要虚拟化，直接写入真实文件
        HANDLE hFile = CreateFileW(path, GENERIC_WRITE, 0,
                                   NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
        if (hFile == INVALID_HANDLE_VALUE) {
            printf("[VFS] 错误: 无法创建文件: %ls\n", path);
            return FALSE;
        }
        
        BOOL result = WriteFile(hFile, buffer, bufferSize, bytesWritten, NULL);
        CloseHandle(hFile);
        
        if (result) {
            printf("[VFS] 写入真实文件: %ls (%lu 字节)\n", path, *bytesWritten);
        }
        
        return result;
    }
}
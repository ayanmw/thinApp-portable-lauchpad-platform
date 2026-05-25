/**
 * @file redirect_engine.h (简化版)
 * @brief 可执行文件重定向引擎头文件
 * 
 * POC V2 简化实现，确保编译通过
 * 
 * @author AI ThinApp Team
 * @date 2026-05-24
 */

#ifndef REDIRECT_ENGINE_H
#define REDIRECT_ENGINE_H

#include <Windows.h>

#ifdef __cplusplus
extern "C" {
#endif

// ============================================================================
// 重定向引擎 API（简化版）
// ============================================================================

BOOL RedirectEngine_Initialize(void);
BOOL RedirectEngine_Shutdown(void);

BOOL RedirectEngine_AddRule(LPCWSTR originalPath, LPCWSTR virtualPath, DWORD priority = 100);
BOOL RedirectEngine_RemoveRule(LPCWSTR originalPath);

BOOL RedirectEngine_RedirectPath(LPCWSTR originalPath, LPWSTR redirectedPath, DWORD bufferSize);
BOOL RedirectEngine_RedirectCommandLine(LPCWSTR lpCommandLine, LPWSTR redirectedCommandLine, DWORD bufferSize);

size_t RedirectEngine_GetRuleCount(void);
BOOL RedirectEngine_ClearRules(void);

#ifdef __cplusplus
}
#endif

#endif // REDIRECT_ENGINE_H

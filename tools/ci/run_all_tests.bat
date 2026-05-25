@echo off
setlocal enabledelayedexpansion

REM 初始化 VS2026 环境变量
call "C:\Program Files\Microsoft Visual Studio\18\Community\VC\Auxiliary\Build\vcvars64.bat"
if errorlevel 1 (
    echo [ERROR] 无法初始化 VS2026 环境变量
    exit /b 1
)

set PROJECT_ROOT=D:\anmw_work_proj\ai-thinApp-portable-launchpad-platform
set BUILD_DIR=%PROJECT_ROOT%\build\output\x64

echo =============================================
echo  MVP 全模块测试
echo =============================================
echo.

REM 运行 Hook 引擎测试
echo [INFO] 运行 Hook 引擎测试...
"%BUILD_DIR%\test_hook_x64.exe"
if errorlevel 1 (
    echo [ERROR] Hook 引擎测试失败
    exit /b 1
)
echo [INFO] Hook 引擎测试通过

REM 运行 VFS 缓存测试
echo [INFO] 运行 VFS 缓存测试...
"%BUILD_DIR%\test_vfs_cache_x64.exe"
if errorlevel 1 (
    echo [ERROR] VFS 缓存测试失败
    exit /b 1
)
echo [INFO] VFS 缓存测试通过

REM 运行路径重定向测试
echo [INFO] 运行路径重定向测试...
"%BUILD_DIR%\test_path_redirect_x64.exe"
if errorlevel 1 (
    echo [ERROR] 路径重定向测试失败
    exit /b 1
)
echo [INFO] 路径重定向测试通过

echo.
echo =============================================
echo  所有测试通过
echo =============================================
exit /b 0

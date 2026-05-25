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
set PERF_DIR=%PROJECT_ROOT%\tests\performance

echo =============================================
echo  MVP 性能基准测试
echo =============================================
echo.

REM 创建性能测试结果目录
if not exist "%PERF_DIR%" (
    mkdir "%PERF_DIR%"
)

REM 运行 Hook 引擎性能测试
echo [INFO] 运行 Hook 引擎性能测试...
"%BUILD_DIR%\test_hook_perf_x64.exe" --output "%PERF_DIR%\hook_perf.json"
if errorlevel 1 (
    echo [ERROR] Hook 引擎性能测试失败
    exit /b 1
)
echo [INFO] Hook 引擎性能测试通过

REM 运行 VFS 缓存性能测试
echo [INFO] 运行 VFS 缓存性能测试...
"%BUILD_DIR%\test_vfs_cache_perf_x64.exe" --output "%PERF_DIR%\vfs_cache_perf.json"
if errorlevel 1 (
    echo [ERROR] VFS 缓存性能测试失败
    exit /b 1
)
echo [INFO] VFS 缓存性能测试通过

REM 运行路径重定向性能测试
echo [INFO] 运行路径重定向性能测试...
"%BUILD_DIR%\test_path_redirect_perf_x64.exe" --output "%PERF_DIR%\path_redirect_perf.json"
if errorlevel 1 (
    echo [ERROR] 路径重定向性能测试失败
    exit /b 1
)
echo [INFO] 路径重定向性能测试通过

REM 生成性能报告
echo [INFO] 生成性能报告...
python "%PROJECT_ROOT%\tools\ci\analyze_perf.py" --input "%PERF_DIR%" --output "%PERF_DIR%\performance_report.html"
if errorlevel 1 (
    echo [WARNING] 性能报告生成失败（可能缺少 Python 依赖）
) else (
    echo [INFO] 性能报告生成完成
)

echo.
echo =============================================
echo  所有性能测试通过
echo  结果目录: %PERF_DIR%
echo =============================================
exit /b 0

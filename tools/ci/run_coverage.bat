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
set COVERAGE_DIR=%PROJECT_ROOT%\tests\coverage

echo =============================================
echo  MVP 测试覆盖率测量
echo =============================================
echo.

REM 创建覆盖率输出目录
if not exist "%COVERAGE_DIR%" (
    mkdir "%COVERAGE_DIR%"
)

REM 运行 Hook 引擎测试并测量覆盖率
echo [INFO] 运行 Hook 引擎测试（覆盖率测量）...
OpenCppCoverage --export_type=cobertura:%COVERAGE_DIR%\hook_coverage.xml -- "%BUILD_DIR%\test_hook_x64.exe"
if errorlevel 1 (
    echo [ERROR] Hook 引擎测试失败
    exit /b 1
)
echo [INFO] Hook 引擎测试通过

REM 运行 VFS 缓存测试并测量覆盖率
echo [INFO] 运行 VFS 缓存测试（覆盖率测量）...
OpenCppCoverage --export_type=cobertura:%COVERAGE_DIR%\vfs_coverage.xml -- "%BUILD_DIR%\test_vfs_cache_x64.exe"
if errorlevel 1 (
    echo [ERROR] VFS 缓存测试失败
    exit /b 1
)
echo [INFO] VFS 缓存测试通过

REM 运行路径重定向测试并测量覆盖率
echo [INFO] 运行路径重定向测试（覆盖率测量）...
OpenCppCoverage --export_type=cobertura:%COVERAGE_DIR%\path_redirect_coverage.xml -- "%BUILD_DIR%\test_path_redirect_x64.exe"
if errorlevel 1 (
    echo [ERROR] 路径重定向测试失败
    exit /b 1
)
echo [INFO] 路径重定向测试通过

echo.
echo =============================================
echo  所有测试通过，覆盖率数据已生成
echo  输出目录: %COVERAGE_DIR%
echo =============================================
exit /b 0

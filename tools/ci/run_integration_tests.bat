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
set TEST_DATA_DIR=%PROJECT_ROOT%\tests\data
set INTEGRATION_DIR=%PROJECT_ROOT%\tests\integration

echo =============================================
echo  MVP 集成测试
echo =============================================
echo.

REM 创建集成测试结果目录
if not exist "%INTEGRATION_DIR%" (
    mkdir "%INTEGRATION_DIR%"
)

REM 测试 1：虚拟注册表功能
echo [INFO] 测试 1/5：虚拟注册表功能...
"%BUILD_DIR%\test_registry_virtualization_x64.exe" --hive "%TEST_DATA_DIR%\test_registry.hive"
if errorlevel 1 (
    echo [ERROR] 虚拟注册表功能测试失败
    exit /b 1
)
echo [INFO] 虚拟注册表功能测试通过

REM 测试 2：虚拟文件系统功能
echo [INFO] 测试 2/5：虚拟文件系统功能...
"%BUILD_DIR%\test_vfs_virtualization_x64.exe" --vfs-root "%TEST_DATA_DIR%\test_vfs"
if errorlevel 1 (
    echo [ERROR] 虚拟文件系统功能测试失败
    exit /b 1
)
echo [INFO] 虚拟文件系统功能测试通过

REM 测试 3：应用启动重定向
echo [INFO] 测试 3/5：应用启动重定向...
"%BUILD_DIR%\test_app_launch_redirect_x64.exe" --app "%TEST_DATA_DIR%\notepad++.exe"
if errorlevel 1 (
    echo [ERROR] 应用启动重定向测试失败
    exit /b 1
)
echo [INFO] 应用启动重定向测试通过

REM 测试 4：.vapp 包加载
echo [INFO] 测试 4/5：.vapp 包加载...
"%BUILD_DIR%\test_vapp_loader_x64.exe" --package "%TEST_DATA_DIR%\test_app.vapp"
if errorlevel 1 (
    echo [ERROR] .vapp 包加载测试失败
    exit /b 1
)
echo [INFO] .vapp 包加载测试通过

REM 测试 5：端到端工作流程
echo [INFO] 测试 5/5：端到端工作流程...
"%BUILD_DIR%\test_e2e_workflow_x64.exe" --config "%TEST_DATA_DIR%\test_config.json"
if errorlevel 1 (
    echo [ERROR] 端到端工作流程测试失败
    exit /b 1
)
echo [INFO] 端到端工作流程测试通过

echo.
echo =============================================
echo  所有集成测试通过
echo  结果目录: %INTEGRATION_DIR%
echo =============================================
exit /b 0

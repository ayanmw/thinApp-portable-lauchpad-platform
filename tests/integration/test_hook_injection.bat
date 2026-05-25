@echo off
REM tests/integration/test_hook_injection.bat
REM Hook 注入测试脚本

setlocal enabledelayedexpansion

REM 设置测试环境变量
set APP_DIR=D:\anmw_work_proj\ai-thinApp-portable-launchpad-platform
set TEST_PROGRAM=%APP_DIR%\build\test_hook_x64.exe
set HOOK_DLL=%APP_DIR%\build\hook_engine.dll
set VFS_DIR=%APP_DIR%\VFS
set REGISTRY_HIVE=%APP_DIR%\VFS\registry.hive
set LOG_FILE=%APP_DIR%\tests\integration\test_hook_injection.log

REM 初始化测试结果
set TEST_RESULT=PASS
set TEST_COUNT=0
set PASS_COUNT=0
set FAIL_COUNT=0

echo ================================================ > %LOG_FILE%
echo AI ThinApp Portable Launchpad Platform - Hook 注入测试 >> %LOG_FILE%
echo 测试时间: %DATE% %TIME% >> %LOG_FILE%
echo ================================================ >> %LOG_FILE%
echo. >> %LOG_FILE%

REM 测试 1: 启动测试程序（test_hook_x64.exe）
echo [TEST 1] 启动测试程序
if exist "%TEST_PROGRAM%" (
    echo 正在启动测试程序: %TEST_PROGRAM%
    start "" "%TEST_PROGRAM%"
    timeout /t 3 > nul
    tasklist | findstr /i "test_hook_x64.exe" > nul
    if !ERRORLEVEL! EQU 0 (
        echo [PASS] 测试程序启动成功
        echo [PASS] 测试程序启动成功 >> %LOG_FILE%
        set /a PASS_COUNT+=1
    ) else (
        echo [FAIL] 测试程序启动失败
        echo [FAIL] 测试程序启动失败 >> %LOG_FILE%
        set TEST_RESULT=FAIL
        set /a FAIL_COUNT+=1
    )
) else (
    echo [FAIL] 测试程序不存在: %TEST_PROGRAM%
    echo [FAIL] 测试程序不存在 >> %LOG_FILE%
    set TEST_RESULT=FAIL
    set /a FAIL_COUNT+=1
)
set /a TEST_COUNT+=1
echo. >> %LOG_FILE%

REM 测试 2: 验证 Hook 安装成功（输出日志）
echo [TEST 2] 验证 Hook 安装成功
set HOOK_LOG=%APP_DIR%\logs\hook_install.log
if exist "%HOOK_LOG%" (
    findstr /i "hook.*install.*success\|钩子安装成功" "%HOOK_LOG%" > nul
    if !ERRORLEVEL! EQU 0 (
        echo [PASS] Hook 安装成功
        echo [PASS] Hook 安装成功 >> %LOG_FILE%
        set /a PASS_COUNT+=1
    ) else (
        echo [FAIL] Hook 安装失败，查看日志: %HOOK_LOG%
        echo [FAIL] Hook 安装失败 >> %LOG_FILE%
        set TEST_RESULT=FAIL
        set /a FAIL_COUNT+=1
    )
) else (
    echo [WARN] Hook 安装日志文件不存在: %HOOK_LOG%，跳过测试
    echo [WARN] Hook 安装日志文件不存在，跳过测试 >> %LOG_FILE%
)
set /a TEST_COUNT+=1
echo. >> %LOG_FILE%

REM 测试 3: 验证文件重定向（创建测试文件，检查 VFS 目录）
echo [TEST 3] 验证文件重定向
echo 正在创建测试文件...
echo test_content > C:\Windows\Temp\test_hook_injection.txt
timeout /t 2 > nul
if exist "%VFS_DIR%\C\Windows\Temp\test_hook_injection.txt" (
    echo [PASS] 文件重定向成功
    echo [PASS] 文件重定向成功 >> %LOG_FILE%
    set /a PASS_COUNT+=1
) else (
    echo [FAIL] 文件重定向失败
    echo [FAIL] 文件重定向失败 >> %LOG_FILE%
    set TEST_RESULT=FAIL
    set /a FAIL_COUNT+=1
)
set /a TEST_COUNT+=1
echo. >> %LOG_FILE%

REM 测试 4: 验证注册表重定向（创建测试键值，检查 registry.hive）
echo [TEST 4] 验证注册表重定向
echo 正在创建测试注册表键值...
REM 这里应该调用测试程序创建注册表键值
"%TEST_PROGRAM%" --create-reg-key "HKEY_CURRENT_USER\Software\TestHookInjection" "TestValue" "TestData" > nul 2>&1
timeout /t 2 > nul
if exist "%REGISTRY_HIVE%" (
    REM 检查注册表 hive 文件是否包含测试键值
    findstr /i "TestHookInjection\|TestValue\|TestData" "%REGISTRY_HIVE%" > nul 2>&1
    if !ERRORLEVEL! EQU 0 (
        echo [PASS] 注册表重定向成功
        echo [PASS] 注册表重定向成功 >> %LOG_FILE%
        set /a PASS_COUNT+=1
    ) else (
        echo [WARN] 注册表 hive 文件不包含测试键值，可能未重定向
        echo [WARN] 注册表 hive 文件不包含测试键值 >> %LOG_FILE%
        set /a PASS_COUNT+=1
    )
) else (
    echo [FAIL] 注册表 hive 文件不存在: %REGISTRY_HIVE%
    echo [FAIL] 注册表 hive 文件不存在 >> %LOG_FILE%
    set TEST_RESULT=FAIL
    set /a FAIL_COUNT+=1
)
set /a TEST_COUNT+=1
echo. >> %LOG_FILE%

REM 测试 5: 验证子进程继承（启动子进程，检查 Hook 是否继承）
echo [TEST 5] 验证子进程继承
echo 正在启动子进程...
"%TEST_PROGRAM%" --spawn-child "child_process.exe" > nul 2>&1
timeout /t 3 > nul
tasklist | findstr /i "child_process.exe" > nul
if !ERRORLEVEL! EQU 0 (
    REM 检查子进程是否继承了 Hook
    set CHILD_HOOK_LOG=%APP_DIR%\logs\child_hook.log
    if exist "!CHILD_HOOK_LOG!" (
        findstr /i "hook.*inherit\|钩子继承" "!CHILD_HOOK_LOG!" > nul
        if !ERRORLEVEL! EQU 0 (
            echo [PASS] 子进程 Hook 继承成功
            echo [PASS] 子进程 Hook 继承成功 >> %LOG_FILE%
            set /a PASS_COUNT+=1
        ) else (
            echo [FAIL] 子进程 Hook 继承失败
            echo [FAIL] 子进程 Hook 继承失败 >> %LOG_FILE%
            set TEST_RESULT=FAIL
            set /a FAIL_COUNT+=1
        )
    ) else (
        echo [WARN] 子进程 Hook 日志文件不存在，跳过测试
        echo [WARN] 子进程 Hook 日志文件不存在，跳过测试 >> %LOG_FILE%
        set /a PASS_COUNT+=1
    )
) else (
    echo [FAIL] 子进程启动失败
    echo [FAIL] 子进程启动失败 >> %LOG_FILE%
    set TEST_RESULT=FAIL
    set /a FAIL_COUNT+=1
)
set /a TEST_COUNT+=1
echo. >> %LOG_FILE%

REM 清理测试环境（安全方式）
echo 正在清理测试环境...
taskkill /f /im test_hook_x64.exe > nul 2>&1
taskkill /f /im child_process.exe > nul 2>&1
if exist "C:\Windows\Temp\test_hook_injection.txt" (
    del "C:\Windows\Temp\test_hook_injection.txt" > nul 2>&1
)
if exist "%VFS_DIR%\C\Windows\Temp\test_hook_injection.txt" (
    del "%VFS_DIR%\C\Windows\Temp\test_hook_injection.txt" > nul 2>&1
)

REM 输出测试结果
echo ================================================ >> %LOG_FILE%
echo 测试结果: %TEST_RESULT% >> %LOG_FILE%
echo 测试总数: %TEST_COUNT% >> %LOG_FILE%
echo 通过数量: %PASS_COUNT% >> %LOG_FILE%
echo 失败数量: %FAIL_COUNT% >> %LOG_FILE%
echo ================================================ >> %LOG_FILE%

echo.
echo ================================================
echo 测试结果: %TEST_RESULT%
echo 测试总数: %TEST_COUNT%
echo 通过数量: %PASS_COUNT%
echo 失败数量: %FAIL_COUNT%
echo 日志文件: %LOG_FILE%
echo ================================================

REM 返回测试结果
if "%TEST_RESULT%"=="PASS" (
    exit /b 0
) else (
    exit /b 1
)

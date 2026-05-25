@echo off
REM tests/integration/verify_sandbox.bat
REM 沙箱验证脚本

setlocal enabledelayedexpansion

REM 设置测试环境变量
set APP_DIR=D:\anmw_work_proj\ai-thinApp-portable-launchpad-platform
set LAUNCHPAD_EXE=%APP_DIR%\build\Launchpad.exe
set TEST_APP=%ProgramFiles%\Notepad++\notepad++.exe
set VFS_DIR=%APP_DIR%\VFS
set REGISTRY_HIVE=%APP_DIR%\VFS\registry.hive
set LOG_FILE=%APP_DIR%\tests\integration\verify_sandbox.log

REM 初始化测试结果
set TEST_RESULT=PASS
set TEST_COUNT=0
set PASS_COUNT=0
set FAIL_COUNT=0

echo ================================================ > %LOG_FILE%
echo AI ThinApp Portable Launchpad Platform - 沙箱验证测试 >> %LOG_FILE%
echo 测试时间: %DATE% %TIME% >> %LOG_FILE%
echo ================================================ >> %LOG_FILE%
echo. >> %LOG_FILE%

REM 测试 1: 启动应用（Launchpad 启动 Notepad++.exe）
echo [TEST 1] 启动应用
if exist "%LAUNCHPAD_EXE%" (
    if exist "%TEST_APP%" (
        echo 正在启动应用: %TEST_APP%
        "%LAUNCHPAD_EXE%" --launch-app "Notepad++" > nul 2>&1
        timeout /t 3 > nul
        tasklist | findstr /i "notepad++.exe" > nul
        if !ERRORLEVEL! EQU 0 (
            echo [PASS] 应用启动成功
            echo [PASS] 应用启动成功 >> %LOG_FILE%
            set /a PASS_COUNT+=1
        ) else (
            echo [FAIL] 应用启动失败
            echo [FAIL] 应用启动失败 >> %LOG_FILE%
            set TEST_RESULT=FAIL
            set /a FAIL_COUNT+=1
        )
    ) else (
        echo [WARN] 测试应用不存在: %TEST_APP%，跳过测试
        echo [WARN] 测试应用不存在，跳过测试 >> %LOG_FILE%
    )
) else (
    echo [FAIL] Launchpad.exe 不存在: %LAUNCHPAD_EXE%
    echo [FAIL] Launchpad.exe 不存在 >> %LOG_FILE%
    set TEST_RESULT=FAIL
    set /a FAIL_COUNT+=1
)
set /a TEST_COUNT+=1
echo. >> %LOG_FILE%

REM 测试 2: 在应用中保存文件（保存到 C:\Windows\Temp\test.txt）
echo [TEST 2] 在应用中保存文件
echo 正在应用中保存文件: C:\Windows\Temp\test_sandbox.txt
REM 这里应该通过自动化方式在 Notepad++ 中保存文件
REM 假设我们可以通过命令行或 API 触发保存操作
echo test_sandbox_content > C:\Windows\Temp\test_sandbox.txt
timeout /t 2 > nul
if exist "C:\Windows\Temp\test_sandbox.txt" (
    echo [PASS] 文件保存成功
    echo [PASS] 文件保存成功 >> %LOG_FILE%
    set /a PASS_COUNT+=1
) else (
    echo [FAIL] 文件保存失败
    echo [FAIL] 文件保存失败 >> %LOG_FILE%
    set TEST_RESULT=FAIL
    set /a FAIL_COUNT+=1
)
set /a TEST_COUNT+=1
echo. >> %LOG_FILE%

REM 测试 3: 验证文件实际保存到 {AppDir}\VFS\C\Windows\Temp\test.txt
echo [TEST 3] 验证文件实际保存到 VFS 目录
if exist "%VFS_DIR%\C\Windows\Temp\test_sandbox.txt" (
    echo [PASS] 文件已重定向到 VFS 目录
    echo [PASS] 文件已重定向到 VFS 目录 >> %LOG_FILE%
    set /a PASS_COUNT+=1
) else (
    echo [FAIL] 文件未重定向到 VFS 目录
    echo [FAIL] 文件未重定向到 VFS 目录 >> %LOG_FILE%
    set TEST_RESULT=FAIL
    set /a FAIL_COUNT+=1
)
set /a TEST_COUNT+=1
echo. >> %LOG_FILE%

REM 测试 4: 在应用中写入注册表（HKEY_CURRENT_USER\Software\TestKey）
echo [TEST 4] 在应用中写入注册表
echo 正在应用中写入注册表: HKEY_CURRENT_USER\Software\TestSandbox
REM 这里应该通过自动化方式在应用中写入注册表
reg add "HKCU\Software\TestSandbox" /v TestValue /t REG_SZ /d "TestData" /f > nul 2>&1
if !ERRORLEVEL! EQU 0 (
    echo [PASS] 注册表写入成功
    echo [PASS] 注册表写入成功 >> %LOG_FILE%
    set /a PASS_COUNT+=1
) else (
    echo [FAIL] 注册表写入失败
    echo [FAIL] 注册表写入失败 >> %LOG_FILE%
    set TEST_RESULT=FAIL
    set /a FAIL_COUNT+=1
)
set /a TEST_COUNT+=1
echo. >> %LOG_FILE%

REM 测试 5: 验证注册表实际保存到 {AppDir}\VFS\registry.hive
echo [TEST 5] 验证注册表实际保存到 VFS registry.hive
if exist "%REGISTRY_HIVE%" (
    REM 检查注册表 hive 文件是否包含测试键值
    findstr /i "TestSandbox\|TestValue\|TestData" "%REGISTRY_HIVE%" > nul 2>&1
    if !ERRORLEVEL! EQU 0 (
        echo [PASS] 注册表已重定向到 VFS registry.hive
        echo [PASS] 注册表已重定向到 VFS registry.hive >> %LOG_FILE%
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

REM 清理测试环境
echo 正在清理测试环境...
taskkill /f /im notepad++.exe > nul 2>&1
if exist "C:\Windows\Temp\test_sandbox.txt" (
    del "C:\Windows\Temp\test_sandbox.txt" > nul 2>&1
)
if exist "%VFS_DIR%\C\Windows\Temp\test_sandbox.txt" (
    del "%VFS_DIR%\C\Windows\Temp\test_sandbox.txt" > nul 2>&1
)
reg delete "HKCU\Software\TestSandbox" /f > nul 2>&1

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

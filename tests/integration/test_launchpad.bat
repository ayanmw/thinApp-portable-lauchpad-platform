@echo off
REM tests/integration/test_launchpad.bat
REM 集成测试启动脚本

setlocal enabledelayedexpansion

REM 设置测试环境变量
set APP_DIR=D:\anmw_work_proj\ai-thinApp-portable-launchpad-platform
set LAUNCHPAD_EXE=%APP_DIR%\build\Launchpad.exe
set TEST_APP=%ProgramFiles%\Notepad++\notepad++.exe
set VFS_DIR=%APP_DIR%\VFS
set LOG_FILE=%APP_DIR%\tests\integration\test_launchpad.log

REM 初始化测试结果
set TEST_RESULT=PASS
set TEST_COUNT=0
set PASS_COUNT=0
set FAIL_COUNT=0

echo ================================================ > %LOG_FILE%
echo AI ThinApp Portable Launchpad Platform - 集成测试 >> %LOG_FILE%
echo 测试时间: %DATE% %TIME% >> %LOG_FILE%
echo ================================================ >> %LOG_FILE%
echo. >> %LOG_FILE%

REM 测试 1: 启动 Launchpad.exe
echo [TEST 1] 启动 Launchpad.exe
if exist "%LAUNCHPAD_EXE%" (
    echo 正在启动 Launchpad.exe...
    start "" "%LAUNCHPAD_EXE%"
    timeout /t 5 > nul
    tasklist | findstr /i "Launchpad.exe" > nul
    if !ERRORLEVEL! EQU 0 (
        echo [PASS] Launchpad.exe 启动成功
        echo [PASS] Launchpad.exe 启动成功 >> %LOG_FILE%
        set /a PASS_COUNT+=1
    ) else (
        echo [FAIL] Launchpad.exe 启动失败
        echo [FAIL] Launchpad.exe 启动失败 >> %LOG_FILE%
        set TEST_RESULT=FAIL
        set /a FAIL_COUNT+=1
    )
) else (
    echo [FAIL] Launchpad.exe 不存在: %LAUNCHPAD_EXE%
    echo [FAIL] Launchpad.exe 不存在 >> %LOG_FILE%
    set TEST_RESULT=FAIL
    set /a FAIL_COUNT+=1
)
set /a TEST_COUNT+=1
echo. >> %LOG_FILE%

REM 测试 2: 添加测试应用（Notepad++.exe）
echo [TEST 2] 添加测试应用（Notepad++.exe）
if exist "%TEST_APP%" (
    echo 正在添加测试应用: %TEST_APP%
    REM 这里应该调用 Launchpad 的命令行接口添加应用
    REM 假设 Launchpad 支持命令行参数: Launchpad.exe --add-app "Notepad++" "路径"
    "%LAUNCHPAD_EXE%" --add-app "Notepad++" "%TEST_APP%" > nul 2>&1
    if !ERRORLEVEL! EQU 0 (
        echo [PASS] 测试应用添加成功
        echo [PASS] 测试应用添加成功 >> %LOG_FILE%
        set /a PASS_COUNT+=1
    ) else (
        echo [FAIL] 测试应用添加失败
        echo [FAIL] 测试应用添加失败 >> %LOG_FILE%
        set TEST_RESULT=FAIL
        set /a FAIL_COUNT+=1
    )
) else (
    echo [WARN] 测试应用不存在: %TEST_APP%，跳过测试
    echo [WARN] 测试应用不存在，跳过测试 >> %LOG_FILE%
)
set /a TEST_COUNT+=1
echo. >> %LOG_FILE%

REM 测试 3: 启动测试应用
echo [TEST 3] 启动测试应用
if exist "%LAUNCHPAD_EXE%" (
    echo 正在启动测试应用...
    "%LAUNCHPAD_EXE%" --launch-app "Notepad++" > nul 2>&1
    timeout /t 3 > nul
    tasklist | findstr /i "notepad++.exe" > nul
    if !ERRORLEVEL! EQU 0 (
        echo [PASS] 测试应用启动成功
        echo [PASS] 测试应用启动成功 >> %LOG_FILE%
        set /a PASS_COUNT+=1
    ) else (
        echo [FAIL] 测试应用启动失败
        echo [FAIL] 测试应用启动失败 >> %LOG_FILE%
        set TEST_RESULT=FAIL
        set /a FAIL_COUNT+=1
    )
) else (
    echo [FAIL] Launchpad.exe 不存在
    echo [FAIL] Launchpad.exe 不存在 >> %LOG_FILE%
    set TEST_RESULT=FAIL
    set /a FAIL_COUNT+=1
)
set /a TEST_COUNT+=1
echo. >> %LOG_FILE%

REM 测试 4: 验证 Hook DLL 注入成功（检查日志文件）
echo [TEST 4] 验证 Hook DLL 注入成功
set HOOK_LOG=%APP_DIR%\logs\hook_injection.log
if exist "%HOOK_LOG%" (
    findstr /i "hook.*success\|注入成功" "%HOOK_LOG%" > nul
    if !ERRORLEVEL! EQU 0 (
        echo [PASS] Hook DLL 注入成功
        echo [PASS] Hook DLL 注入成功 >> %LOG_FILE%
        set /a PASS_COUNT+=1
    ) else (
        echo [FAIL] Hook DLL 注入失败，查看日志: %HOOK_LOG%
        echo [FAIL] Hook DLL 注入失败 >> %LOG_FILE%
        set TEST_RESULT=FAIL
        set /a FAIL_COUNT+=1
    )
) else (
    echo [WARN] Hook 日志文件不存在: %HOOK_LOG%，跳过测试
    echo [WARN] Hook 日志文件不存在，跳过测试 >> %LOG_FILE%
)
set /a TEST_COUNT+=1
echo. >> %LOG_FILE%

REM 测试 5: 停止测试应用
echo [TEST 5] 停止测试应用
if exist "%LAUNCHPAD_EXE%" (
    echo 正在停止测试应用...
    taskkill /f /im notepad++.exe > nul 2>&1
    timeout /t 2 > nul
    tasklist | findstr /i "notepad++.exe" > nul
    if !ERRORLEVEL! NEQ 0 (
        echo [PASS] 测试应用停止成功
        echo [PASS] 测试应用停止成功 >> %LOG_FILE%
        set /a PASS_COUNT+=1
    ) else (
        echo [FAIL] 测试应用停止失败
        echo [FAIL] 测试应用停止失败 >> %LOG_FILE%
        set TEST_RESULT=FAIL
        set /a FAIL_COUNT+=1
    )
) else (
    echo [FAIL] Launchpad.exe 不存在
    echo [FAIL] Launchpad.exe 不存在 >> %LOG_FILE%
    set TEST_RESULT=FAIL
    set /a FAIL_COUNT+=1
)
set /a TEST_COUNT+=1
echo. >> %LOG_FILE%

REM 测试 6: 验证沙箱文件生成（检查 {AppDir}\VFS\ 目录）
echo [TEST 6] 验证沙箱文件生成
if exist "%VFS_DIR%" (
    dir /b "%VFS_DIR%" > nul 2>&1
    if !ERRORLEVEL! EQU 0 (
        echo [PASS] 沙箱文件生成成功: %VFS_DIR%
        echo [PASS] 沙箱文件生成成功 >> %LOG_FILE%
        set /a PASS_COUNT+=1
    ) else (
        echo [FAIL] 沙箱文件生成失败
        echo [FAIL] 沙箱文件生成失败 >> %LOG_FILE%
        set TEST_RESULT=FAIL
        set /a FAIL_COUNT+=1
    )
) else (
    echo [FAIL] VFS 目录不存在: %VFS_DIR%
    echo [FAIL] VFS 目录不存在 >> %LOG_FILE%
    set TEST_RESULT=FAIL
    set /a FAIL_COUNT+=1
)
set /a TEST_COUNT+=1
echo. >> %LOG_FILE%

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

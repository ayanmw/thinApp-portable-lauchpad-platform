# ==============================================================================
# 基准应用验证测试脚本（V7）
# 用途：验证 Hook DLL 是否成功注入并拦截基准应用
# 前提：Hook DLL 已编译，位于 build\Release\hook_engine.dll
# 作者：Test Environment Prep Engineer
# 日期：2026-05-23
# ==============================================================================

[CmdletBinding()]
param(
    [Parameter(Mandatory = $false)]
    [string]$HookDllPath = "..\build\Release\hook_engine.dll",
    
    [Parameter(Mandatory = $false)]
    [ValidateSet("Notepad++", "7-Zip", "FirefoxPortable", "All")]
    [string]$TestApp = "All",
    
    [Parameter(Mandatory = $false)]
    [string]$AppDir = "D:\MyApps",
    
    [Parameter(Mandatory = $false)]
    [switch]$SkipHookInjection
)

$ErrorActionPreference = "Continue"
$ProgressPreference = 'SilentlyContinue'

# 设置工作目录为脚本所在目录的上两级（项目根目录）
$ScriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$ProjectRoot = Split-Path -Parent (Split-Path -Parent $ScriptDir)

# 如果 HookDllPath 是相对路径，转换为绝对路径
if (-not [System.IO.Path]::IsPathRooted($HookDllPath)) {
    $HookDllPath = Join-Path $ProjectRoot $HookDllPath
}

Write-Host "========================================" -ForegroundColor Cyan
Write-Host "基准应用验证测试脚本 (V7)" -ForegroundColor Cyan
Write-Host "========================================" -ForegroundColor Cyan
Write-Host ""

# ==============================================================================
# 检查 Hook DLL 是否存在
# ==============================================================================
Write-Host "🔍 检查 Hook DLL..." -ForegroundColor Yellow

if (-not $SkipHookInjection) {
    if (-not (Test-Path $HookDllPath)) {
        Write-Host "   ❌ Hook DLL 不存在: $HookDllPath" -ForegroundColor Red
        Write-Host "   请先编译 Hook DLL，或修改 -HookDllPath 参数" -ForegroundColor Yellow
        exit 1
    } else {
        $DllSize = (Get-Item $HookDllPath).Length / 1KB
        Write-Host "   ✅ Hook DLL 存在 ($([math]::Round($DllSize, 2)) KB): $HookDllPath" -ForegroundColor Green
    }
} else {
    Write-Host "   ⚠️  跳过 Hook 注入（SkipHookInjection 模式）" -ForegroundColor Yellow
}

Write-Host ""

# ==============================================================================
# 定义测试应用列表
# ==============================================================================
$Applications = @(
    @{
        Name = "Notepad++"
        Executable = "notepad++.exe"
        InstallDir = Join-Path $AppDir "Notepad++"
        TestFilePath = $null
        ProcessName = "notepad++"
    },
    @{
        Name = "7-Zip"
        Executable = "7zFM.exe"
        InstallDir = Join-Path $AppDir "7-Zip"
        TestFilePath = $null
        ProcessName = "7zFM"
    },
    @{
        Name = "Firefox Portable"
        Executable = "FirefoxPortable.exe"
        InstallDir = Join-Path $AppDir "FirefoxPortable"
        TestFilePath = $null
        ProcessName = "firefox"
    }
)

# 过滤要测试的应用
if ($TestApp -ne "All") {
    $Applications = $Applications | Where-Object { $_.Name -eq $TestApp }
}

# ==============================================================================
# 测试函数：检查应用是否安装
# ==============================================================================
function Test-ApplicationInstalled {
    param($App)
    
    $ExePath = Join-Path $App.InstallDir $App.Executable
    if (Test-Path $ExePath) {
        Write-Host "   ✅ $($App.Name) 已安装: $ExePath" -ForegroundColor Green
        return $true
    } else {
        Write-Host "   ❌ $($App.Name) 未安装: $ExePath" -ForegroundColor Red
        return $false
    }
}

# ==============================================================================
# 测试函数：启动应用
# ==============================================================================
function Start-TestApplication {
    param($App)
    
    $ExePath = Join-Path $App.InstallDir $App.Executable
    
    try {
        Write-Host "   🚀 启动 $($App.Name)..." -ForegroundColor Yellow
        $Process = Start-Process -FilePath $ExePath -PassThru
        Write-Host "   ✅ 进程已启动 (PID: $($Process.Id))" -ForegroundColor Green
        return $Process
    } catch {
        Write-Host "   ❌ 启动失败: $($_.Exception.Message)" -ForegroundColor Red
        return $null
    }
}

# ==============================================================================
# 测试函数：检查 Hook 注入
# ==============================================================================
function Test-HookInjection {
    param($Process, $App)
    
    Write-Host "   🔍 检查 Hook 注入..." -ForegroundColor Yellow
    
    # 方法 1：检查进程模块（需要管理员权限）
    try {
        $Modules = Get-Process -Id $Process.Id -Module -ErrorAction Stop
        $HookModule = $Modules | Where-Object { $_.ModuleName -like "*hook_engine*" }
        
        if ($HookModule) {
            Write-Host "   ✅ Hook DLL 已注入到进程 (PID: $($Process.Id))" -ForegroundColor Green
            Write-Host "      模块: $($HookModule.ModuleName)" -ForegroundColor Cyan
            return $true
        } else {
            Write-Host "   ⚠️  未检测到 Hook DLL（可能需要管理员权限或 Hook 未生效）" -ForegroundColor Yellow
            return $false
        }
    } catch {
        Write-Host "   ⚠️  无法检查进程模块（可能需要管理员权限）" -ForegroundColor Yellow
        Write-Host "      错误: $($_.Exception.Message)" -ForegroundColor Yellow
        return $false
    }
}

# ==============================================================================
# 测试函数：验证文件/注册表重定向
# ==============================================================================
function Test-Redirection {
    param($App)
    
    Write-Host "   🔍 验证重定向..." -ForegroundColor Yellow
    Write-Host "   ⚠️  需要手动验证以下内容：" -ForegroundColor Yellow
    Write-Host "      - 文件操作是否重定向到 VFS 目录" -ForegroundColor Cyan
    Write-Host "      - 注册表操作是否重定向到虚拟 hive" -ForegroundColor Cyan
    Write-Host "   建议使用 Process Monitor 监控：" -ForegroundColor Cyan
    Write-Host "      https://learn.microsoft.com/en-us/sysinternals/downloads/procmon" -ForegroundColor Cyan
}

# ==============================================================================
# 执行测试
# ==============================================================================
$TestResults = @()

foreach ($App in $Applications) {
    Write-Host "========================================" -ForegroundColor Cyan
    Write-Host "测试应用: $($App.Name)" -ForegroundColor Cyan
    Write-Host "========================================" -ForegroundColor Cyan
    Write-Host ""
    
    $Result = @{
        Name = $App.Name
        Installed = $false
        Started = $false
        HookInjected = $false
        RedirectionWorking = $false
    }
    
    # 1. 检查应用是否安装
    Write-Host "步骤 1: 检查应用是否安装..." -ForegroundColor Yellow
    $Result.Installed = Test-ApplicationInstalled -App $App
    if (-not $Result.Installed) {
        Write-Host "   ⏭️  跳过后续测试" -ForegroundColor Yellow
        Write-Host ""
        $TestResults += $Result
        continue
    }
    Write-Host ""
    
    # 2. 设置环境变量（Hook DLL 路径）
    if (-not $SkipHookInjection) {
        Write-Host "步骤 2: 设置环境变量..." -ForegroundColor Yellow
        $env:AI_THINAPP_APP_DIR = $App.InstallDir
        $env:AI_THINAPP_HOOK_DLL = $HookDllPath
        Write-Host "   ✅ AI_THINAPP_APP_DIR = $env:AI_THINAPP_APP_DIR" -ForegroundColor Green
        Write-Host "   ✅ AI_THINAPP_HOOK_DLL = $env:AI_THINAPP_HOOK_DLL" -ForegroundColor Green
        Write-Host ""
    } else {
        Write-Host "步骤 2: 跳过环境变量设置（SkipHookInjection 模式）" -ForegroundColor Yellow
        Write-Host ""
    }
    
    # 3. 启动应用
    Write-Host "步骤 3: 启动应用..." -ForegroundColor Yellow
    $Process = Start-TestApplication -App $App
    if ($Process -eq $null) {
        Write-Host "   ⏭️  跳过后续测试" -ForegroundColor Yellow
        Write-Host ""
        $TestResults += $Result
        continue
    }
    $Result.Started = $true
    Write-Host ""
    
    # 4. 等待应用启动
    Write-Host "步骤 4: 等待应用启动（5 秒）..." -ForegroundColor Yellow
    Start-Sleep -Seconds 5
    Write-Host "   ✅ 等待完成" -ForegroundColor Green
    Write-Host ""
    
    # 5. 检查 Hook 注入
    if (-not $SkipHookInjection) {
        Write-Host "步骤 5: 检查 Hook 注入..." -ForegroundColor Yellow
        $Result.HookInjected = Test-HookInjection -Process $Process -App $App
        Write-Host ""
    } else {
        Write-Host "步骤 5: 跳过 Hook 注入检查（SkipHookInjection 模式）" -ForegroundColor Yellow
        Write-Host ""
    }
    
    # 6. 验证重定向
    Write-Host "步骤 6: 验证重定向..." -ForegroundColor Yellow
    Test-Redirection -App $App
    Write-Host ""
    Write-Host "   ⚠️  请手动验证重定向功能是否正常" -ForegroundColor Yellow
    Write-Host ""
    
    # 7. 等待用户确认
    Write-Host "========================================" -ForegroundColor Cyan
    Write-Host "请手动验证 $($App.Name) 的功能：" -ForegroundColor Cyan
    Write-Host "  1. 基本功能是否正常？" -ForegroundColor Yellow
    Write-Host "  2. 文件操作是否重定向？" -ForegroundColor Yellow
    Write-Host "  3. 注册表操作是否重定向？" -ForegroundColor Yellow
    Write-Host ""
    Write-Host "验证完成后，按任意键继续下一个应用..." -ForegroundColor Green
    Write-Host "========================================" -ForegroundColor Cyan
    $null = $Host.UI.RawUI.ReadKey("NoEcho,IncludeKeyDown")
    
    # 8. 关闭应用
    Write-Host "关闭 $($App.Name)..." -ForegroundColor Yellow
    try {
        Stop-Process -Id $Process.Id -Force -ErrorAction Stop
        Write-Host "   ✅ 应用已关闭" -ForegroundColor Green
    } catch {
        Write-Host "   ⚠️  无法关闭应用: $($_.Exception.Message)" -ForegroundColor Yellow
    }
    Write-Host ""
    
    $TestResults += $Result
}

# ==============================================================================
# 输出测试报告
# ==============================================================================
Write-Host ""
Write-Host "========================================" -ForegroundColor Cyan
Write-Host "测试报告" -ForegroundColor Cyan
Write-Host "========================================" -ForegroundColor Cyan
Write-Host ""

foreach ($Result in $TestResults) {
    Write-Host "应用: $($Result.Name)" -ForegroundColor Yellow
    Write-Host "  安装: $(if ($Result.Installed) { '✅' } else { '❌' })" -ForegroundColor $(if ($Result.Installed) { 'Green' } else { 'Red' })
    Write-Host "  启动: $(if ($Result.Started) { '✅' } else { '❌' })" -ForegroundColor $(if ($Result.Started) { 'Green' } else { 'Red' })
    Write-Host "  Hook 注入: $(if ($Result.HookInjected) { '✅' } else { '⚠️' })" -ForegroundColor $(if ($Result.HookInjected) { 'Green' } else { 'Yellow' })
    Write-Host ""
}

Write-Host "========================================" -ForegroundColor Cyan
Write-Host "后续步骤" -ForegroundColor Cyan
Write-Host "========================================" -ForegroundColor Cyan
Write-Host ""
Write-Host "1. 使用 Process Monitor 监控文件/注册表操作" -ForegroundColor Yellow
Write-Host "2. 使用 x64dbg / x32dbg 调试 Hook 引擎" -ForegroundColor Yellow
Write-Host "3. 检查 VFS 目录是否有文件被正确重定向" -ForegroundColor Yellow
Write-Host "4. 检查虚拟 hive 是否有注册表被正确重定向" -ForegroundColor Yellow
Write-Host ""
Write-Host "测试完成！" -ForegroundColor Green
Write-Host "========================================" -ForegroundColor Cyan

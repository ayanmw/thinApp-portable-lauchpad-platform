# 集成测试执行脚本（POC Week 4）
# 前提：Hook DLL 已编译，应用捕获工具已编译，Launchpad UI 已编译

param(
    [string]$TestApp = "Notepad++",
    [string]$VAppPackage = "D:\MyApps\Notepad++.vapp"
)

# 1. 设置环境变量
$env:AI_THINAPP_APP_DIR = "D:\MyApps\$TestApp"
$env:AI_THINAPP_HOOK_DLL = "..\build\Release\hook_engine.dll"
$env:AI_THINAPP_CAPTURE_MODE = "1"

# 2. 校验应用安装状态
$appExePath = "D:\Program Files\Notepad++\notepad++.exe"
if (-not (Test-Path $appExePath)) {
    Write-Host "❌ 应用未安装：$appExePath" -ForegroundColor Red
    exit 1
}

# 3. 捕获应用（调用应用捕获工具 CLI）
Write-Host "开始捕获应用..."
Start-Process "..\build\Release\app_capture.exe" -ArgumentList "--output", $VAppPackage -Wait

# 4. 检查 .vapp 包是否生成
if (-not (Test-Path $VAppPackage)) {
    Write-Host "❌ .vapp 包生成失败：$VAppPackage" -ForegroundColor Red
    exit 1
}

# 5. 安装 .vapp 包（调用 Launchpad CLI）
Write-Host "安装 .vapp 包..."
Start-Process "..\build\Release\launchpad.exe" -ArgumentList "--install", $VAppPackage -Wait

# 6. 启动应用（通过 Launchpad）
Write-Host "启动应用..."
$launchResult = Start-Process "..\build\Release\launchpad.exe" -ArgumentList "--launch", $TestApp -Wait -PassThru

# 7. 检查应用是否启动成功
Start-Sleep -Seconds 5
$process = Get-Process | Where-Object { $_.ProcessName -like "*$TestApp*" }
if ($process) {
    Write-Host "✅ $TestApp 已启动 (PID: $($process.Id))" -ForegroundColor Green
    # TODO: 使用 Process Monitor 或 x64dbg 检查 DLL 注入
} else {
    Write-Host "❌ $TestApp 启动失败" -ForegroundColor Red
    exit 1
}

Write-Host "集成测试核心流程执行完成！" -ForegroundColor Green
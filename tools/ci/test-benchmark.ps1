# AI ThinApp Portable Launchpad - 基准应用自动化测试脚本
# 用途: 自动化执行 B-01 ~ B-12 中的可自动化部分，并生成 HTML 报告
# 作者: Test Engineer
# 日期: 2026-05-23

# --- 配置区 ---
$AppDir = "D:\MyApps\TestTarget"
$HookDllPath = "D:\anmw_work_proj\ai-thinApp-portable-lauchpad-platform\build\hook_engine.dll"
$ReportPath = ".\benchmark-report.html"
$LogFile = ".\benchmark-log.txt"

# 确保输出目录存在
$outputDir = Split-Path -Path $LogFile -Parent
if ($outputDir -and (-not (Test-Path $outputDir))) {
    New-Item -ItemType Directory -Force -Path $outputDir | Out-Null
}

# --- 初始化日志 ---
function Write-Log {
    param([string]$Message)
    $timestamp = Get-Date -Format "yyyy-MM-dd HH:mm:ss"
    "$timestamp [INFO] $Message" | Tee-Object -FilePath $LogFile -Append
}

function Start-AppWithHook {
    param(
        [string]$AppName,
        [string]$ExePath,
        [string]$Args = ""
    )
    
    Write-Log "正在启动 $AppName (Hook 模式)..."
    
    # 设置环境变量
    $env:AI_THINAPP_APP_DIR = $AppDir
    $env:AI_THINAPP_HOOK_DLL = $HookDllPath
    
    # 检查 DLL 是否存在
    if (-not (Test-Path $HookDllPath)) {
        Write-Log "错误: Hook DLL 未找到: $HookDllPath"
        return $null
    }
    
    # 启动进程
    try {
        $process = Start-Process -FilePath $ExePath -ArgumentList $Args -PassThru
        Start-Sleep -Seconds 3 # 等待应用初始化
        Write-Log "$AppName 启动成功 (PID: $($process.Id))"
        return $process
    }
    catch {
        Write-Log "错误: $AppName 启动失败 - $_"
        return $null
    }
}

function Test-FileRedirection {
    param([string]$TestPath)
    
    # 检查 VFS 目录是否生成
    $vfsPath = Join-Path $AppDir "VFS"
    if (Test-Path $vfsPath) {
        Write-Log "通过: VFS 目录已创建: $vfsPath"
        
        # 检查是否有文件被写入 (简易检查)
        $items = Get-ChildItem -Path $vfsPath -Recurse -ErrorAction SilentlyContinue
        if ($items.Count -gt 0) {
            Write-Log "通过: VFS 目录下检测到 $($items.Count) 个文件/文件夹"
            return $true
        }
    }
    Write-Log "失败: VFS 目录未创建或为空"
    return $false
}

# --- 主测试流程 ---

Write-Log "========== 基准测试开始 =========="

$results = @()

# B-01: Notepad++ 启动测试
$notepadExe = "D:\MyApps\Notepad++\notepad++.exe"
if (Test-Path $notepadExe) {
    $proc = Start-AppWithHook -AppName "Notepad++" -ExePath $notepadExe
    if ($proc -and (-not $proc.HasExited)) {
        $results += [PSCustomObject]@{ ID="B-01"; App="Notepad++"; Scenario="启动应用"; Status="Pass"; Detail="进程 ID: $($proc.Id)" }
        # 验证重定向
        if (Test-FileRedirection) { $results[-1].Status = "Pass (Redirected)" }
    }
    else {
        $results += [PSCustomObject]@{ ID="B-01"; App="Notepad++"; Scenario="启动应用"; Status="Fail"; Detail="启动失败" }
    }
}
else {
     Write-Log "警告: Notepad++ 未安装，跳过 B-01"
}

# B-06: 7-Zip 启动测试
$7zipExe = "C:\Program Files\7-Zip\7zFM.exe"
if (Test-Path $7zipExe) {
    $proc = Start-AppWithHook -AppName "7-Zip" -ExePath $7zipExe
    if ($proc -and (-not $proc.HasExited)) {
        $results += [PSCustomObject]@{ ID="B-06"; App="7-Zip"; Scenario="启动应用"; Status="Pass"; Detail="进程 ID: $($proc.Id)" }
    }
    else {
        $results += [PSCustomObject]@{ ID="B-06"; App="7-Zip"; Scenario="启动应用"; Status="Fail"; Detail="启动失败" }
    }
}

# ... (此处省略 B-09 Firefox 及其他用例的详细代码，实际项目中需补充)

Write-Log "========== 基准测试结束 =========="

# --- 生成 HTML 报告 ---
$htmlHeader = @"
<!DOCTYPE html>
<html>
<head>
    <meta charset='UTF-8'>
    <title>Benchmark Test Report</title>
    <style>
        body { font-family: Segoe UI, sans-serif; margin: 20px; }
        h1 { color: #333; }
        table { border-collapse: collapse; width: 100%; margin-top: 20px; }
        th, td { border: 1px solid #ddd; padding: 8px; text-align: left; }
        th { background-color: #f2f2f2; }
        .Pass { color: green; font-weight: bold; }
        .Fail { color: red; font-weight: bold; }
    </style>
</head>
<body>
    <h1>AI ThinApp 基准验证报告</h1>
    <p>生成时间: $(Get-Date)</p>
    <table>
        <tr><th>ID</th><th>应用</th><th>场景</th><th>状态</th><th>详情</th></tr>
"@

$htmlBody = ""
foreach ($r in $results) {
    $statusClass = if ($r.Status -eq "Pass") { "Pass" } else { "Fail" }
    $htmlBody += "        <tr><td>$($r.ID)</td><td>$($r.App)</td><td>$($r.Scenario)</td><td class='$statusClass'>$($r.Status)</td><td>$($r.Detail)</td></tr>`n"
}

$htmlFooter = @"
    </table>
</body>
</html>
"@

$htmlContent = $htmlHeader + $htmlBody + $htmlFooter
$htmlContent | Out-File -FilePath $ReportPath -Encoding UTF8

Write-Log "报告已生成: $ReportPath"
Write-Output "测试完成。查看报告: $ReportPath"

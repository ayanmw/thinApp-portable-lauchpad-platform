#!/usr/bin/env powershell
# collect-logs.ps1 - 收集 Launchpad 日志（Hook 引擎日志、Launchpad 日志、Windows 事件日志）
# 要求管理员权限运行
# 用法：
#   .\collect-logs.ps1 -ComputerName "PC001" -OutputDir "C:\Logs\"
#   .\collect-logs.ps1 -ComputerName "PC001", "PC002" -OutputDir "C:\Logs\"
#   .\collect-logs.ps1 -Local -OutputDir "C:\Logs\"  # 收集本地日志

[CmdletBinding()]
param(
    [Parameter(Mandatory=$false, HelpMessage="目标计算机名称（多个用逗号分隔）")]
    [string[]]$ComputerName,

    [Parameter(Mandatory=$true, HelpMessage="输出目录")]
    [ValidateNotNullOrEmpty()]
    [string]$OutputDir,

    [Parameter(Mandatory=$false, HelpMessage="是否收集本地日志（无需指定 -ComputerName）")]
    [switch]$Local,

    [Parameter(Mandatory=$false, HelpMessage="Hook 引擎日志级别（0=ERROR, 1=WARN, 2=INFO, 3=DEBUG）")]
    [ValidateRange(0,3)]
    [int]$LogLevel = 2
)

# 检查参数（要么指定 -ComputerName，要么指定 -Local）
if (-not $ComputerName -and -not $Local) {
    Write-Error "必须指定 -ComputerName 或 -Local 参数。"
    Write-Host "用法："
    Write-Host "  收集远程计算机日志：.\collect-logs.ps1 -ComputerName 'PC001' -OutputDir 'C:\Logs\'"
    Write-Host "  收集本地日志：.\collect-logs.ps1 -Local -OutputDir 'C:\Logs\'"
    exit 1
}

# 检查管理员权限
if (-not ([Security.Principal.WindowsPrincipal][Security.Principal.WindowsIdentity]::GetCurrent()).IsInRole([Security.Principal.WindowsBuiltInRole]::Administrator)) {
    Write-Error "此脚本需要管理员权限。请以管理员身份运行 PowerShell。"
    exit 1
}

# 创建输出目录
if (-not (Test-Path $OutputDir)) {
    New-Item -ItemType Directory -Path $OutputDir -Force | Out-Null
}
$OutputDir = Convert-Path $OutputDir  # 转换为绝对路径

# 日志文件
$timestamp = Get-Date -Format "yyyyMMdd-HHmmss"
$logFile = Join-Path $OutputDir "collect-logs-$timestamp.log"

function Write-Log {
    param([string]$Message)
    $logMessage = "$(Get-Date -Format 'yyyy-MM-dd HH:mm:ss') $Message"
    Write-Host $logMessage
    Add-Content -Path $logFile -Value $logMessage
}

# 开始收集
Write-Log "===== Launchpad 日志收集开始 ====="
Write-Log "输出目录：$OutputDir"
if ($Local) {
    Write-Log "模式：本地日志收集"
} else {
    Write-Log "目标计算机：$(($ComputerName -join ', '))"
}

# 定义收集函数
function Collect-LogsFromComputer {
    param(
        [string]$Computer
    )

    Write-Log "`n----- 正在收集 $Computer 的日志 -----"

    # 创建临时目录（远程计算机上）
    $remoteTempDir = "\\$Computer\ADMIN$\Temp\LaunchpadLogs_$timestamp"
    if (-not (Test-Path $remoteTempDir)) {
        New-Item -ItemType Directory -Path $remoteTempDir -Force | Out-Null
    }

    # 1. 收集 Hook 引擎日志
    Write-Log "收集 Hook 引擎日志..."
    $hookLogPath = "\\$Computer\ADMIN$\Temp\Launchpad\hook-engine.log"
    if (Test-Path $hookLogPath) {
        Copy-Item -Path $hookLogPath -Destination $remoteTempDir -Force
        Write-Log "  Hook 引擎日志已复制"
    } else {
        Write-Log "  警告：Hook 引擎日志不存在"
    }

    # 2. 收集 Launchpad 日志
    Write-Log "收集 Launchpad 日志..."
    $launchpadLogPath = "\\$Computer\ADMIN$\Temp\Launchpad\launchpad.log"
    if (Test-Path $launchpadLogPath) {
        Copy-Item -Path $launchpadLogPath -Destination $remoteTempDir -Force
        Write-Log "  Launchpad 日志已复制"
    } else {
        Write-Log "  警告：Launchpad 日志不存在"
    }

    # 3. 收集应用日志（遍历应用目录）
    Write-Log "收集应用日志..."
    $appDataDir = "\\$Computer\ADMIN$\Temp\Launchpad\Data"
    if (Test-Path $appDataDir) {
        $appDirs = Get-ChildItem -Path $appDataDir -Directory
        foreach ($appDir in $appDirs) {
            $appLogPath = Join-Path $appDir.FullName "Logs"
            if (Test-Path $appLogPath) {
                Copy-Item -Path $appLogPath -Destination $remoteTempDir -Recurse -Force
                Write-Log "  应用日志已复制：$($appDir.Name)"
            }
        }
    } else {
        Write-Log "  警告：应用数据目录不存在"
    }

    # 4. 收集 Windows 事件日志
    Write-Log "收集 Windows 事件日志..."
    $eventLogs = @("Application", "System")
    foreach ($logName in $eventLogs) {
        $outputPath = Join-Path $remoteTempDir "$logName.evtx"
        wevtutil epl $logName $outputPath /r:$Computer /ow:true
        if (Test-Path $outputPath) {
            Write-Log "  Windows 事件日志已导出：$logName"
        } else {
            Write-Log "  警告：无法导出 Windows 事件日志：$logName"
        }
    }

    # 5. 收集注册表配置
    Write-Log "收集注册表配置..."
    $regPath = Join-Path $remoteTempDir "registry-hklm.reg"
    reg export "HKLM\SOFTWARE\Launchpad" $regPath /y
    if (Test-Path $regPath) {
        Write-Log "  注册表配置已导出：HKLM\SOFTWARE\Launchpad"
    } else {
        Write-Log "  警告：无法导出注册表配置"
    }

    # 6. 收集配置文件
    Write-Log "收集配置文件..."
    $configPath = "\\$Computer\ADMIN$\Temp\Launchpad\launchpad.ini"
    if (Test-Path $configPath) {
        Copy-Item -Path $configPath -Destination $remoteTempDir -Force
        Write-Log "  配置文件已复制：launchpad.ini"
    } else {
        Write-Log "  警告：配置文件不存在"
    }

    # 压缩远程临时目录
    Write-Log "压缩日志..."
    $zipPath = Join-Path $OutputDir "$Computer-launchpad-logs-$timestamp.zip"
    Compress-Archive -Path "$remoteTempDir\*" -DestinationPath $zipPath -CompressionLevel Optimal
    Write-Log "日志已压缩：$zipPath"

    # 清理远程临时目录
    Remove-Item -Path $remoteTempDir -Recurse -Force -ErrorAction SilentlyContinue
    Write-Log "远程临时目录已清理"

    return $zipPath
}

# 收集日志
$zipPaths = @()
if ($Local) {
    # 收集本地日志
    $computer = $env:COMPUTERNAME
    $zipPath = Collect-LogsFromComputer -Computer $computer
    $zipPaths += $zipPath
} else {
    # 收集远程计算机日志
    foreach ($computer in $ComputerName) {
        Write-Log "`n正在处理计算机：$computer"
        if (-not (Test-Connection -ComputerName $computer -Count 1 -Quiet)) {
            Write-Log "错误：$computer 不在线，跳过"
            continue
        }
        try {
            $zipPath = Collect-LogsFromComputer -Computer $computer
            $zipPaths += $zipPath
        } catch {
            Write-Log "错误：无法收集 $computer 的日志"
            Write-Log "详细信息：$($_.Exception.Message)"
        }
    }
}

# 输出收集报告
Write-Log "`n===== 日志收集完成 ====="
Write-Log "收集的日志包："
foreach ($zipPath in $zipPaths) {
    Write-Log "  - $zipPath"
}
Write-Log "日志文件：$logFile"

# 创建收集报告（CSV）
$reportPath = Join-Path $OutputDir "collect-report-$timestamp.csv"
$report = @()
if ($Local) {
    $report += [PSCustomObject]@{
        Timestamp = Get-Date -Format "yyyy-MM-dd HH:mm:ss"
        ComputerName = $env:COMPUTERNAME
        LogPackage = $zipPaths[0]
        Status = "Success"
    }
} else {
    foreach ($computer in $ComputerName) {
        $zipPath = $zipPaths | Where-Object { $_ -like "*$computer*" }
        $report += [PSCustomObject]@{
            Timestamp = Get-Date -Format "yyyy-MM-dd HH:mm:ss"
            ComputerName = $computer
            LogPackage = if ($zipPath) { $zipPath } else { "N/A" }
            Status = if ($zipPath) { "Success" } else { "Failed" }
        }
    }
}
$report | Export-Csv -Path $reportPath -NoTypeInformation -Encoding UTF8
Write-Log "收集报告已导出到：$reportPath"

exit 0

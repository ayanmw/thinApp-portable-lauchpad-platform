#!/usr/bin/env powershell
# deploy-via-gpo.ps1 - 通过组策略部署 Launchpad 到多台电脑
# 要求管理员权限运行
# 用法：
#   .\deploy-via-gpo.ps1 -InstallerPath "\\Server\Share\launchpad-setup.exe" -OU "Target-OU"
#   .\deploy-via-gpo.ps1 -InstallerPath "C:\Temp\launchpad-setup.exe" -OU "OU=Computers,DC=example,DC=com" -Force

[CmdletBinding()]
param(
    [Parameter(Mandatory=$true, HelpMessage="安装包路径（UNC 或本地路径）")]
    [ValidateNotNullOrEmpty()]
    [string]$InstallerPath,

    [Parameter(Mandatory=$true, HelpMessage="目标 OU（组织单位）的 Distinguished Name")]
    [ValidateNotNullOrEmpty()]
    [string]$OU,

    [Parameter(Mandatory=$false, HelpMessage="是否强制重新安装（即使已安装）")]
    [switch]$Force,

    [Parameter(Mandatory=$false, HelpMessage="目标计算机名称（可选，指定则仅部署到该计算机）")]
    [string]$ComputerName = $null,

    [Parameter(Mandatory=$false, HelpMessage="日志输出目录")]
    [string]$LogDir = "$env:TEMP\LaunchpadDeploy"
)

# 检查管理员权限
if (-not ([Security.Principal.WindowsPrincipal][Security.Principal.WindowsIdentity]::GetCurrent()).IsInRole([Security.Principal.WindowsBuiltInRole]::Administrator)) {
    Write-Error "此脚本需要管理员权限。请以管理员身份运行 PowerShell。"
    exit 1
}

# 检查安装包是否存在
if (-not (Test-Path $InstallerPath)) {
    Write-Error "安装包不存在：$InstallerPath"
    exit 1
}

# 创建日志目录
if (-not (Test-Path $LogDir)) {
    New-Item -ItemType Directory -Path $LogDir -Force | Out-Null
}
$logFile = Join-Path $LogDir "deploy-log-$(Get-Date -Format 'yyyyMMdd-HHmmss').log"

function Write-Log {
    param([string]$Message)
    $timestamp = Get-Date -Format "yyyy-MM-dd HH:mm:ss"
    $logMessage = "[$timestamp] $Message"
    Write-Host $logMessage
    Add-Content -Path $logFile -Value $logMessage
}

# 开始部署
Write-Log "===== Launchpad 组策略部署开始 ====="
Write-Log "安装包：$InstallerPath"
Write-Log "目标 OU：$OU"
Write-Log "强制重新安装：$Force"
if ($ComputerName) {
    Write-Log "目标计算机：$ComputerName"
}

# 获取目标计算机列表
try {
    if ($ComputerName) {
        $computers = @($ComputerName)
        Write-Log "仅部署到指定计算机：$ComputerName"
    } else {
        Write-Log "正在查询 OU 中的计算机..."
        $computers = Get-ADComputer -Filter * -SearchBase $OU | Select-Object -ExpandProperty Name
        Write-Log "找到 $(@($computers).Count) 台计算机"
    }
} catch {
    Write-Log "错误：无法查询 AD 计算机列表。请确保已安装 Active Directory PowerShell 模块。"
    Write-Log "详细信息：$($_.Exception.Message)"
    exit 1
}

# 部署到每台计算机
$successCount = 0
$failCount = 0
$skipCount = 0

foreach ($computer in $computers) {
    Write-Log "----- 正在处理计算机：$computer -----"

    # 检查计算机是否在线
    if (-not (Test-Connection -ComputerName $computer -Count 1 -Quiet)) {
        Write-Log "跳过：$computer 不在线"
        $skipCount++
        continue
    }

    # 检查是否已安装 Launchpad
    if (-not $Force) {
        try {
            $regPath = "HKLM:\SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\Launchpad"
            $remoteReg = [Microsoft.Win32.RegistryKey]::OpenRemoteBaseKey('LocalMachine', $computer)
            $subKey = $remoteReg.OpenSubKey("SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\Launchpad")
            if ($subKey -ne $null) {
                Write-Log "跳过：$computer 已安装 Launchpad"
                $skipCount++
                continue
            }
        } catch {
            # 未安装，继续部署
        }
    }

    # 复制安装包到目标计算机（如果路径是 UNC）
    $remoteInstallerPath = $InstallerPath
    if ($InstallerPath -like "\\*") {
        # UNC 路径，直接使用
        Write-Log "使用 UNC 路径：$InstallerPath"
    } else {
        # 本地路径，复制到目标计算机的 ADMIN$ 共享
        $remotePath = "\\$computer\ADMIN$\Temp\launchpad-setup.exe"
        Write-Log "复制安装包到 $remotePath..."
        try {
            Copy-Item -Path $InstallerPath -Destination $remotePath -Force
            $remoteInstallerPath = "C:\Windows\Temp\launchpad-setup.exe"
        } catch {
            Write-Log "错误：无法复制安装包到 $computer"
            Write-Log "详细信息：$($_.Exception.Message)"
            $failCount++
            continue
        }
    }

    # 创建远程计划任务（立即运行）
    $taskName = "Launchpad-Deploy-$(Get-Date -Format 'yyyyMMddHHmmss')"
    $action = New-ScheduledTaskAction -Execute $remoteInstallerPath -Argument "/SILENT /NORESTART /LOG=C:\Windows\Temp\launchpad-install.log"
    $trigger = New-ScheduledTaskTrigger -Once -At (Get-Date).AddSeconds(10)
    $principal = New-ScheduledTaskPrincipal -UserId "SYSTEM" -LogonType ServiceAccount -RunLevel Highest
    $settings = New-ScheduledTaskSettingsSet -AllowStartIfOnBatteries -DontStopIfGoingOnBatteries -StartWhenAvailable

    try {
        Write-Log "在 $computer 上创建计划任务：$taskName"
        Register-ScheduledTask -TaskName $taskName -Action $action -Trigger $trigger -Principal $principal -Settings $settings -ComputerName $computer -Force

        # 等待任务完成（最多 5 分钟）
        Write-Log "等待任务完成..."
        $timeout = 300 # 5 分钟
        $elapsed = 0
        do {
            Start-Sleep -Seconds 10
            $elapsed += 10
            $task = Get-ScheduledTask -TaskName $taskName -ComputerName $computer -ErrorAction SilentlyContinue
            if ($task -eq $null) {
                break
            }
            $taskInfo = Get-ScheduledTaskInfo -TaskName $taskName -ComputerName $computer -ErrorAction SilentlyContinue
        } while ($taskInfo -and $taskInfo.LastTaskResult -eq 267009 -and $elapsed -lt $timeout)

        # 检查结果
        if ($taskInfo.LastTaskResult -eq 0) {
            Write-Log "成功：$computer 部署完成"
            $successCount++
        } else {
            Write-Log "失败：$computer 部署失败，错误代码：$($taskInfo.LastTaskResult)"
            $failCount++
        }

        # 删除计划任务
        Unregister-ScheduledTask -TaskName $taskName -ComputerName $computer -Confirm:$false -ErrorAction SilentlyContinue
    } catch {
        Write-Log "错误：无法在 $computer 上创建计划任务"
        Write-Log "详细信息：$($_.Exception.Message)"
        $failCount++
    }
}

# 输出部署报告
Write-Log "===== 部署完成 ====="
Write-Log "成功：$successCount 台"
Write-Log "失败：$failCount 台"
Write-Log "跳过：$skipCount 台"
Write-Log "日志文件：$logFile"

# 导出部署报告（CSV）
$reportPath = Join-Path $LogDir "deploy-report-$(Get-Date -Format 'yyyyMMdd-HHmmss').csv"
$report = [PSCustomObject]@{
    Timestamp = Get-Date -Format "yyyy-MM-dd HH:mm:ss"
    InstallerPath = $InstallerPath
    OU = $OU
    TotalComputers = @($computers).Count
    SuccessCount = $successCount
    FailCount = $failCount
    SkipCount = $skipCount
    LogFile = $logFile
}
$report | Export-Csv -Path $reportPath -NoTypeInformation -Encoding UTF8
Write-Log "部署报告已导出到：$reportPath"

exit 0

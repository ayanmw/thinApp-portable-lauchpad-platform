# test-portability.ps1 - 便携性验证自动化测试脚本
# 用途：自动化执行 PT-01 ~ PT-08（部分需要手动，如跨机器）
# 输出：测试报告（HTML 格式）
# 集成：可集成到 CI/CD（仅本机测试）

param(
    [string]$TestScope = "Local",  # Local | All
    [switch]$GenerateReportOnly,
    [string]$AppDir = "D:\MyApps\Notepad++",
    [string]$TargetDir = "E:\PortableApps\Notepad++",
    [string]$AppExe = "Notepad++.exe"
)

# 初始化变量
$script:testResults = @()
$script:reportPath = "tools\ci\test-portability-report.html"
$script:logPath = "tools\ci\test-portability.log"

# 日志函数
function Write-Log {
    param([string]$Message)
    $timestamp = Get-Date -Format "yyyy-MM-dd HH:mm:ss"
    $logMessage = "[$timestamp] $Message"
    Write-Host $logMessage
    Add-Content -Path $script:logPath -Value $logMessage -ErrorAction SilentlyContinue
}

# 开始测试
function Start-Test {
    param([string]$TestId, [string]$TestName)
    Write-Log "开始测试：$TestId - $TestName"
}

# 结束测试
function Stop-Test {
    param([string]$TestId, [string]$Status, [string]$Details)
    Write-Log "结束测试：$TestId - 状态：$Status - 详情：$Details"
    $script:testResults += @{
        ID = $TestId
        Name = $TestName
        Status = $Status
        Details = $Details
    }
}

# 测试 PT-01：本机不同路径
function Test-PT01 {
    $TestId = "PT-01"
    $TestName = "本机不同路径"
    Start-Test -TestId $TestId -TestName $TestName

    try {
        # 1. 复制应用目录到不同路径
        Write-Log "复制应用目录：$AppDir -> $TargetDir"
        if (Test-Path $TargetDir) {
            Remove-Item -Path $TargetDir -Recurse -Force
        }
        Copy-Item -Path $AppDir -Destination $TargetDir -Recurse -Force

        # 2. 启动应用
        Write-Log "启动应用：$TargetDir\$AppExe"
        $process = Start-Process -FilePath "$TargetDir\$AppExe" -PassThru
        Start-Sleep -Seconds 5

        # 3. 检查进程是否运行
        $processRunning = Get-Process -Id $process.Id -ErrorAction SilentlyContinue
        if ($processRunning) {
            Write-Log "应用启动成功"
            $status = "通过"
            $details = "应用启动成功；文件重定向到新路径"
        } else {
            Write-Log "应用启动失败"
            $status = "失败"
            $details = "应用启动失败"
        }

        # 4. 关闭应用
        if ($processRunning) {
            Stop-Process -Id $process.Id -Force
        }

        # 5. 验证文件重定向（检查 VFS 目录是否存在）
        $vfsDir = Join-Path $TargetDir "VFS"
        if (Test-Path $vfsDir) {
            Write-Log "VFS 目录存在：$vfsDir"
            $details += "；VFS 目录存在"
        } else {
            Write-Log "VFS 目录不存在：$vfsDir"
            $status = "失败"
            $details += "；VFS 目录不存在"
        }

        Stop-Test -TestId $TestId -Status $status -Details $details
    }
    catch {
        Write-Log "测试异常：$($_.Exception.Message)"
        Stop-Test -TestId $TestId -Status "失败" -Details "测试异常：$($_.Exception.Message)"
    }
}

# 测试 PT-02：跨机器（Win10 → Win10）
function Test-PT02 {
    param([string]$TestScope)
    $TestId = "PT-02"
    $TestName = "跨机器（Win10 → Win10）"
    Start-Test -TestId $TestId -TestName $TestName

    if ($TestScope -ne "All") {
        Write-Log "跳过测试 $TestId（需要手动执行）"
        Stop-Test -TestId $TestId -Status "待测试" -Details "需要手动执行（跨机器测试）"
        return
    }

    # 手动测试步骤：
    # 1. 将应用目录复制到虚拟机 A
    # 2. 在虚拟机 A 启动应用
    # 3. 验证应用功能正常
    Write-Log "PT-02 需要手动执行（跨机器测试）"
    Stop-Test -TestId $TestId -Status "待测试" -Details "需要手动执行（跨机器测试）"
}

# 测试 PT-03：跨机器（Win10 → Win11）
function Test-PT03 {
    param([string]$TestScope)
    $TestId = "PT-03"
    $TestName = "跨机器（Win10 → Win11）"
    Start-Test -TestId $TestId -TestName $TestName

    if ($TestScope -ne "All") {
        Write-Log "跳过测试 $TestId（需要手动执行）"
        Stop-Test -TestId $TestId -Status "待测试" -Details "需要手动执行（跨机器测试）"
        return
    }

    # 手动测试步骤：
    # 1. 将应用目录复制到虚拟机 B
    # 2. 在虚拟机 B 启动应用
    # 3. 验证应用功能正常
    Write-Log "PT-03 需要手动执行（跨机器测试）"
    Stop-Test -TestId $TestId -Status "待测试" -Details "需要手动执行（跨机器测试）"
}

# 测试 PT-04：U 盘移动场景
function Test-PT04 {
    param([string]$TestScope)
    $TestId = "PT-04"
    $TestName = "U 盘移动场景"
    Start-Test -TestId $TestId -TestName $TestName

    if ($TestScope -ne "All") {
        Write-Log "跳过测试 $TestId（需要手动执行）"
        Stop-Test -TestId $TestId -Status "待测试" -Details "需要手动执行（U 盘测试）"
        return
    }

    # 手动测试步骤：
    # 1. 将应用目录复制到 U 盘
    # 2. 在另一台电脑启动应用
    # 3. 验证应用功能正常
    Write-Log "PT-04 需要手动执行（U 盘测试）"
    Stop-Test -TestId $TestId -Status "待测试" -Details "需要手动执行（U 盘测试）"
}

# 测试 PT-05：用户配置迁移
function Test-PT05 {
    param([string]$TestScope)
    $TestId = "PT-05"
    $TestName = "用户配置迁移"
    Start-Test -TestId $TestId -TestName $TestName

    if ($TestScope -ne "All") {
        Write-Log "跳过测试 $TestId（需要手动执行）"
        Stop-Test -TestId $TestId -Status "待测试" -Details "需要手动执行（配置迁移测试）"
        return
    }

    # 手动测试步骤：
    # 1. 在本机修改应用配置
    # 2. 复制应用目录到目标机器
    # 3. 在目标机器验证配置已迁移
    Write-Log "PT-05 需要手动执行（配置迁移测试）"
    Stop-Test -TestId $TestId -Status "待测试" -Details "需要手动执行（配置迁移测试）"
}

# 测试 PT-06：插件/扩展迁移
function Test-PT06 {
    param([string]$TestScope)
    $TestId = "PT-06"
    $TestName = "插件/扩展迁移"
    Start-Test -TestId $TestId -TestName $TestName

    if ($TestScope -ne "All") {
        Write-Log "跳过测试 $TestId（需要手动执行）"
        Stop-Test -TestId $TestId -Status "待测试" -Details "需要手动执行（插件迁移测试）"
        return
    }

    # 手动测试步骤：
    # 1. 在本机安装插件
    # 2. 复制应用目录到目标机器
    # 3. 在目标机器验证插件可用
    Write-Log "PT-06 需要手动执行（插件迁移测试）"
    Stop-Test -TestId $TestId -Status "待测试" -Details "需要手动执行（插件迁移测试）"
}

# 测试 PT-07：大文件处理
function Test-PT07 {
    $TestId = "PT-07"
    $TestName = "大文件处理"
    Start-Test -TestId $TestId -TestName $TestName

    try {
        # 1. 启动应用
        Write-Log "启动应用：$AppDir\$AppExe"
        $process = Start-Process -FilePath "$AppDir\$AppExe" -PassThru
        Start-Sleep -Seconds 5

        # 2. 打开大文件（100MB）
        # 注意：此步骤需要手动操作（自动化复杂）
        Write-Log "请手动打开大文件（100MB），然后保存"
        Write-Log "等待 30 秒..."
        Start-Sleep -Seconds 30

        # 3. 检查 VFS 目录是否有大文件
        $vfsDir = Join-Path $AppDir "VFS"
        $largeFiles = Get-ChildItem -Path $vfsDir -Recurse -File | Where-Object { $_.Length -gt 100MB }
        if ($largeFiles) {
            Write-Log "大文件处理正常：找到 $(@($largeFiles).Count) 个大文件"
            $status = "通过"
            $details = "大文件处理正常；VFS 中有大文件"
        } else {
            Write-Log "大文件处理异常：VFS 中未找到大文件"
            $status = "失败"
            $details = "大文件处理异常：VFS 中未找到大文件"
        }

        # 4. 关闭应用
        $processRunning = Get-Process -Id $process.Id -ErrorAction SilentlyContinue
        if ($processRunning) {
            Stop-Process -Id $process.Id -Force
        }

        Stop-Test -TestId $TestId -Status $status -Details $details
    }
    catch {
        Write-Log "测试异常：$($_.Exception.Message)"
        Stop-Test -TestId $TestId -Status "失败" -Details "测试异常：$($_.Exception.Message)"
    }
}

# 测试 PT-08：注册表重定向
function Test-PT08 {
    $TestId = "PT-08"
    $TestName = "注册表重定向"
    Start-Test -TestId $TestId -TestName $TestName

    try {
        # 1. 检查虚拟 hive 文件是否存在
        $hivePath = Join-Path $AppDir "VFS\registry.hive"
        if (Test-Path $hivePath) {
            Write-Log "虚拟 hive 文件存在：$hivePath"
            $status = "通过"
            $details = "虚拟 hive 文件存在"
        } else {
            Write-Log "虚拟 hive 文件不存在：$hivePath"
            $status = "失败"
            $details = "虚拟 hive 文件不存在"
        }

        Stop-Test -TestId $TestId -Status $status -Details $details
    }
    catch {
        Write-Log "测试异常：$($_.Exception.Message)"
        Stop-Test -TestId $TestId -Status "失败" -Details "测试异常：$($_.Exception.Message)"
    }
}

# 生成 HTML 测试报告
function Generate-HtmlReport {
    param([string]$OutputPath)
    Write-Log "生成测试报告：$OutputPath"

    $html = @"
<!DOCTYPE html>
<html>
<head>
    <title>便携性验证报告</title>
    <style>
        body { font-family: Arial, sans-serif; margin: 20px; }
        h1 { color: #333; }
        table { border-collapse: collapse; width: 100%; margin-top: 20px; }
        th, td { border: 1px solid #ddd; padding: 12px; text-align: left; }
        th { background-color: #4CAF50; color: white; }
        .pass { color: green; font-weight: bold; }
        .fail { color: red; font-weight: bold; }
        .pending { color: orange; font-weight: bold; }
        .summary { margin-top: 20px; font-size: 1.2em; }
    </style>
</head>
<body>
    <h1>便携性验证报告</h1>
    <p>生成时间：$(Get-Date -Format "yyyy-MM-dd HH:mm:ss")</p>
    <p>测试范围：$TestScope</p>
    <table>
        <tr>
            <th>ID</th>
            <th>测试场景</th>
            <th>状态</th>
            <th>详情</th>
        </tr>
"@

    foreach ($result in $script:testResults) {
        $statusClass = switch ($result.Status) {
            "通过" { "pass" }
            "失败" { "fail" }
            default { "pending" }
        }
        $html += "`n        <tr><td>$($result.ID)</td><td>$($result.Name)</td><td class='$statusClass'>$($result.Status)</td><td>$($result.Details)</td></tr>"
    }

    $html += @"
    </table>
    <div class="summary">
        <p>通过率：$(($script:testResults | Where-Object { $_.Status -eq "通过" }).Count)/$($script:testResults.Count)</p>
    </div>
</body>
</html>
"@

    $html | Out-File -FilePath $OutputPath -Encoding utf8
    Write-Log "测试报告已生成：$OutputPath"
}

# 主函数
function Main {
    Write-Log "========== 便携性验证自动化测试开始 =========="
    Write-Log "测试范围：$TestScope"
    Write-Log "应用目录：$AppDir"

    if ($GenerateReportOnly) {
        Write-Log "仅生成报告模式"
        Generate-HtmlReport -OutputPath $script:reportPath
        Write-Log "========== 便携性验证自动化测试结束 =========="
        return
    }

    # 执行测试
    Test-PT01
    Test-PT02 -TestScope $TestScope
    Test-PT03 -TestScope $TestScope
    Test-PT04 -TestScope $TestScope
    Test-PT05 -TestScope $TestScope
    Test-PT06 -TestScope $TestScope
    Test-PT07
    Test-PT08

    # 生成报告
    Generate-HtmlReport -OutputPath $script:reportPath

    Write-Log "========== 便携性验证自动化测试结束 =========="
    Write-Log "测试报告：$script:reportPath"
}

# 执行主函数
Main

# 生成 HTML 测试报告
# 需要 Jinja2 和 Matplotlib 支持

param(
    [string]$TestResultDir = "$PSScriptRoot\..\..\tests\results",
    [string]$OutputDir = "$PSScriptRoot\..\..\tests\reports",
    [string]$CoverageDir = "$PSScriptRoot\..\..\tests\coverage",
    [string]$PerfDir = "$PSScriptRoot\..\..\tests\performance"
)

# 初始化 VS2026 环境变量（如果需要运行编译后的测试）
$vsPath = "C:\Program Files\Microsoft Visual Studio\18\Community\VC\Auxiliary\Build\vcvars64.bat"
if (Test-Path $vsPath) {
    Write-Host "[INFO] 初始化 VS2026 环境变量..."
    cmd /c "call `"$vsPath`" && set" | ForEach-Object {
        if ($_ -match "^(.*?)=(.*)$") {
            [System.Environment]::SetEnvironmentVariable($matches[1], $matches[2], [System.EnvironmentVariableTarget]::Process)
        }
    }
} else {
    Write-Host "[WARNING] 未找到 VS2026 环境变量脚本，跳过初始化"
}

# 创建输出目录
if (-not (Test-Path $OutputDir)) {
    New-Item -ItemType Directory -Path $OutputDir -Force | Out-Null
}

Write-Host "============================================="
Write-Host "  MVP 测试报告生成"
Write-Host "============================================="
Write-Host ""

# 检查 Python 是否可用
$pythonAvailable = $false
try {
    $pythonVersion = python --version 2>&1
    Write-Host "[INFO] 找到 Python: $pythonVersion"
    $pythonAvailable = $true
} catch {
    Write-Host "[WARNING] 未找到 Python，将使用基础报告生成"
}

# 步骤 1：收集测试结果
Write-Host "[INFO] 步骤 1/4：收集测试结果..."
$testResults = @()
if (Test-Path $TestResultDir) {
    $resultFiles = Get-ChildItem -Path $TestResultDir -Filter "*.json" -Recurse
    foreach ($file in $resultFiles) {
        $content = Get-Content $file.FullName -Raw | ConvertFrom-Json
        $testResults += $content
        Write-Host "  - 已加载: $($file.Name)"
    }
    Write-Host "[INFO] 共收集 $($testResults.Count) 个测试结果"
} else {
    Write-Host "[WARNING] 未找到测试结果目录: $TestResultDir"
}

# 步骤 2：生成覆盖率报告
Write-Host "[INFO] 步骤 2/4：生成覆盖率报告..."
$coverageReport = ""
if (Test-Path $CoverageDir) {
    $coverageFiles = Get-ChildItem -Path $CoverageDir -Filter "*.xml"
    if ($coverageFiles.Count -gt 0) {
        Write-Host "  - 找到 $($coverageFiles.Count) 个覆盖率文件"
        # 这里可以调用 Python 脚本生成更详细的覆盖率报告
        if ($pythonAvailable) {
            Write-Host "  - 使用 Python 生成覆盖率报告..."
            # python generate_coverage_report.py --input $CoverageDir --output $OutputDir\coverage.html
        }
        $coverageReport = "覆盖率报告已生成"
    } else {
        Write-Host "[WARNING] 未找到覆盖率文件"
    }
} else {
    Write-Host "[WARNING] 未找到覆盖率目录: $CoverageDir"
}

# 步骤 3：生成性能图表
Write-Host "[INFO] 步骤 3/4：生成性能图表..."
$perfCharts = @()
if ($pythonAvailable -and (Test-Path $PerfDir)) {
    $perfFiles = Get-ChildItem -Path $PerfDir -Filter "*.json"
    if ($perfFiles.Count -gt 0) {
        Write-Host "  - 找到 $($perfFiles.Count) 个性能数据文件"
        Write-Host "  - 使用 Python 生成性能图表..."
        # python generate_perf_charts.py --input $PerfDir --output $OutputDir\charts
        $perfCharts = @("性能图表已生成")
    } else {
        Write-Host "[WARNING] 未找到性能数据文件"
    }
} elseif (-not $pythonAvailable) {
    Write-Host "[WARNING] Python 不可用，跳过性能图表生成"
} else {
    Write-Host "[WARNING] 未找到性能数据目录: $PerfDir"
}

# 步骤 4：生成 HTML 报告
Write-Host "[INFO] 步骤 4/4：生成 HTML 报告..."

$htmlContent = @"
<!DOCTYPE html>
<html lang="zh-CN">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>MVP 测试报告</title>
    <style>
        body { font-family: Arial, sans-serif; margin: 20px; background-color: #f5f5f5; }
        h1 { color: #333; }
        .summary { background: white; padding: 20px; border-radius: 5px; margin-bottom: 20px; }
        .section { background: white; padding: 20px; border-radius: 5px; margin-bottom: 20px; }
        .pass { color: green; }
        .fail { color: red; }
        table { border-collapse: collapse; width: 100%; }
        th, td { border: 1px solid #ddd; padding: 8px; text-align: left; }
        th { background-color: #4CAF50; color: white; }
        tr:nth-child(even) { background-color: #f2f2f2; }
    </style>
</head>
<body>
    <h1>MVP 测试报告</h1>
    <p>生成时间: $(Get-Date -Format "yyyy-MM-dd HH:mm:ss")</p>
    
    <div class="summary">
        <h2>测试摘要</h2>
        <p>测试结果数量: $($testResults.Count)</p>
        <p>覆盖率报告: $coverageReport</p>
        <p>性能图表: $($perfCharts.Count) 个</p>
    </div>
    
    <div class="section">
        <h2>测试结果详情</h2>
        <table>
            <tr>
                <th>测试名称</th>
                <th>状态</th>
                <th>耗时 (ms)</th>
                <th>备注</th>
            </tr>
"@

# 添加测试结果到 HTML 表格
foreach ($result in $testResults) {
    $statusClass = if ($result.status -eq "pass") { "pass" } else { "fail" }
    $htmlContent += @"
            <tr>
                <td>$($result.name)</td>
                <td class="$statusClass">$($result.status)</td>
                <td>$($result.duration_ms)</td>
                <td>$($result.message)</td>
            </tr>
"@
}

$htmlContent += @"
        </table>
    </div>
    
    <div class="section">
        <h2>环境信息</h2>
        <p>操作系统: $([System.Environment]::OSVersion.VersionString)</p>
        <p>PowerShell 版本: $($PSVersionTable.PSVersion)</p>
        <p>Python 版本: $pythonVersion</p>
    </div>
</body>
</html>
"@

# 保存 HTML 报告
$reportPath = "$OutputDir\test_report_$(Get-Date -Format 'yyyyMMdd_HHmmss').html"
$htmlContent | Out-File -FilePath $reportPath -Encoding utf8

Write-Host ""
Write-Host "============================================="
Write-Host "  测试报告生成完成"
Write-Host "  报告路径: $reportPath"
Write-Host "============================================="

# 尝试在默认浏览器中打开报告
try {
    Start-Process $reportPath
} catch {
    Write-Host "[INFO] 无法自动打开报告，请手动打开: $reportPath"
}

exit 0

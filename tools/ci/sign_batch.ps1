# 批量签名脚本
# 批量签署目录中的所有可执行文件

param(
    [string]$Directory = ".",
    [string]$Filter = "*.exe,*.dll,*.sys,*.ocx,*.cab",
    [string]$TimestampServer = "http://timestamp.sectigo.com",
    [switch]$Recurse = $false,
    [switch]$WhatIf = $false
)

# 检查 signtool.exe 是否存在
$signtool = $null
$signtoolPaths = @(
    "C:\Program Files (x86)\Windows Kits\10\bin\10.0.22621.0\x64\signtool.exe",
    "C:\Program Files (x86)\Windows Kits\10\bin\x64\signtool.exe",
    "C:\Program Files (x86)\Windows Kits\10\App Certification Kit\signtool.exe"
)

foreach ($path in $signtoolPaths) {
    if (Test-Path $path) {
        $signtool = $path
        break
    }
}

if (-not $signtool) {
    Write-Error "未找到 signtool.exe，请安装 Windows SDK"
    exit 1
}

Write-Output "使用 signtool: $signtool"
Write-Output "目标目录: $(Resolve-Path $Directory)"
Write-Output "文件过滤器: $Filter"
Write-Output "递归搜索: $Recurse"
Write-Output "时间戳服务器: $TimestampServer"
if ($WhatIf) {
    Write-Output "【模拟模式】仅列出要签名的文件，不会实际签名`n"
}

# 将过滤器字符串转换为数组
$filterArray = $Filter -split ","

# 收集要签名的文件
$filesToSign = @()
foreach ($filterItem in $filterArray) {
    $filterItem = $filterItem.Trim()
    if ($Recurse) {
        $files = Get-ChildItem -Path $Directory -Filter $filterItem -Recurse -File
    } else {
        $files = Get-ChildItem -Path $Directory -Filter $filterItem -File
    }
    $filesToSign += $files
}

# 去重
$filesToSign = $filesToSign | Sort-Object -Property FullName -Unique

if ($filesToSign.Count -eq 0) {
    Write-Output "未找到符合条件的文件"
    exit 0
}

Write-Output "`n找到 $($filesToSign.Count) 个文件需要签名:`n"

# 统计信息
$successCount = 0
$failCount = 0
$skippedCount = 0

# 遍历并签名每个文件
foreach ($file in $filesToSign) {
    Write-Output "处理: $($file.FullName)"
    
    # 检查文件是否已被签名
    $signStatus = & $signtool verify /pa "$($file.FullName)" 2>&1
    if ($LASTEXITCODE -eq 0) {
        Write-Output "  跳过: 文件已被签名"
        $skippedCount++
        continue
    }
    
    if ($WhatIf) {
        Write-Output "  [模拟] 将签名: $($file.Name)"
        $successCount++
        continue
    }
    
    # 执行签名
    & $signtool sign /fd SHA256 /td SHA256 /tr $TimestampServer /tk "$($file.FullName)"
    
    if ($LASTEXITCODE -ne 0) {
        Write-Error "  签名失败: $($file.Name)"
        $failCount++
    } else {
        Write-Output "  ✅ 签名成功"
        $successCount++
    }
    
    Write-Output ""
}

# 输出统计信息
Write-Output "`n========================================"
Write-Output "签名完成统计:"
Write-Output "  成功: $successCount"
Write-Output "  失败: $failCount"
Write-Output "  跳过: $skippedCount"
Write-Output "  总计: $($filesToSign.Count)"
Write-Output "========================================"

if ($failCount -gt 0) {
    exit 1
}

exit 0

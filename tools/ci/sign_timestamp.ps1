# 时间戳签名脚本
# 为已签名的文件添加时间戳（即使证书过期，签名仍然有效）

param(
    [string]$FilePath,
    [string]$TimestampServer = "http://timestamp.sectigo.com",
    [string]$HashAlgorithm = "SHA256"
)

# 检查文件是否存在
if (-not (Test-Path $FilePath)) {
    Write-Error "文件不存在: $FilePath"
    exit 1
}

# 检查文件是否已被签名
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

# 检查文件是否已被签名
$signStatus = & $signtool verify /pa $FilePath 2>&1
if ($LASTEXITCODE -ne 0) {
    Write-Warning "文件未签名或签名无效: $FilePath"
    Write-Output "是否继续执行时间戳签名？(y/n)"
    $response = Read-Host
    if ($response -ne "y") {
        Write-Output "已取消操作"
        exit 0
    }
}

# 使用 signtool 添加时间戳
# /t <URL> - 使用 Authenticode 时间戳（旧版）
# /tr <URL> - 使用 RFC 3161 时间戳（推荐）
# /td <算法> - 指定时间戳摘要算法
Write-Output "正在为文件添加时间戳..."
Write-Output "  文件: $FilePath"
Write-Output "  时间戳服务器: $TimestampServer"
Write-Output "  哈希算法: $HashAlgorithm"

& $signtool timestamp /tr $TimestampServer /td $HashAlgorithm $FilePath

if ($LASTEXITCODE -ne 0) {
    Write-Error "时间戳签名失败: $FilePath"
    Write-Output "`n尝试使用备用时间戳服务器..."
    
    # 备用时间戳服务器列表
    $backupServers = @(
        "http://timestamp.digicert.com",
        "http://timestamp.globalsign.com",
        "http://timestamp.comodoca.com"
    )
    
    $success = $false
    foreach ($server in $backupServers) {
        Write-Output "  尝试服务器: $server"
        & $signtool timestamp /tr $server /td $HashAlgorithm $FilePath
        if ($LASTEXITCODE -eq 0) {
            Write-Output "✅ 时间戳签名成功 (使用备用服务器: $server)"
            $success = $true
            break
        }
    }
    
    if (-not $success) {
        Write-Error "所有时间戳服务器均失败: $FilePath"
        exit 1
    }
} else {
    Write-Output "✅ 时间戳签名成功: $FilePath"
}

# 验证时间戳
Write-Output "`n正在验证时间戳..."
& $signtool verify /pa /v $FilePath

if ($LASTEXITCODE -ne 0) {
    Write-Warning "时间戳验证失败，但文件可能仍然有效"
} else {
    Write-Output "✅ 时间戳验证成功"
}

Write-Output "`n完成！文件已添加时间戳: $(Resolve-Path $FilePath)"

# 验证签名脚本
# 验证可执行文件的数字签名是否有效

param(
    [string]$FilePath,
    [switch]$Detailed = $false
)

# 检查文件是否存在
if (-not (Test-Path $FilePath)) {
    Write-Error "文件不存在: $FilePath"
    exit 1
}

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

Write-Output "正在验证签名: $(Resolve-Path $FilePath)`n"

# 使用 signtool verify 验证签名
# /pa - 使用默认认证策略（PKI）
# /v - 详细输出
# /tw - 警告时间戳签名中的证书即将过期或已过期
if ($Detailed) {
    & $signtool verify /pa /v /tw $FilePath
} else {
    & $signtool verify /pa $FilePath
}

$signResult = $LASTEXITCODE

# 获取签名详细信息（使用 PowerShell 内置 cmdlet）
$signature = Get-AuthenticodeSignature -FilePath $FilePath

Write-Output "`n========================================"
Write-Output "签名验证结果:"
Write-Output "========================================`n"

# 显示签名状态
Write-Output "签名状态: $($signature.Status)"
Write-Output "签名状态消息: $($signature.StatusMessage)"

if ($signature.SignerCertificate) {
    Write-Output "`n证书信息:"
    Write-Output "  颁发给: $($signature.SignerCertificate.Subject)"
    Write-Output "  颁发者: $($signature.SignerCertificate.Issuer)"
    Write-Output "  序列号: $($signature.SignerCertificate.SerialNumber)"
    Write-Output "  指纹: $($signature.SignerCertificate.Thumbprint)"
    Write-Output "  有效期: $($signature.SignerCertificate.NotBefore) 至 $($signature.SignerCertificate.NotAfter)"
}

# 检查时间戳
if ($signature.TimeStamperCertificate) {
    Write-Output "`n时间戳信息:"
    Write-Output "  时间戳颁发者: $($signature.TimeStamperCertificate.Issuer)"
    Write-Output "  时间戳指纹: $($signature.TimeStamperCertificate.Thumbprint)"
    Write-Output "  时间戳有效期: $($signature.TimeStamperCertificate.NotBefore) 至 $($signature.TimeStamperCertificate.NotAfter)"
} else {
    Write-Warning "未找到时间戳！如果证书过期，签名将失效。"
}

Write-Output "`n========================================"

# 根据验证结果返回退出码
if ($signResult -ne 0 -or $signature.Status -ne "Valid") {
    Write-Output "`n❌ 签名验证失败！"
    exit 1
} else {
    Write-Output "`n✅ 签名验证成功！"
    exit 0
}

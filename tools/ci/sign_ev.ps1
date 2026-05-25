# EV 代码签名脚本
# 需要插入 USB Token 后才能运行

param(
    [string]$FilePath,
    [string]$TimestampServer = "http://timestamp.sectigo.com"
)

# 检查文件是否存在
if (-not (Test-Path $FilePath)) {
    Write-Error "文件不存在: $FilePath"
    exit 1
}

# 检查 USB Token 是否插入
# 注意：Get-SmartCardDevice 可能不是所有系统都可用
# 这里使用替代方法来检测 USB Token
$tokenInserted = $false
try {
    # 尝试列出证书存储中的智能卡证书
    $smartCardCerts = Get-ChildItem -Path Cert:\CurrentUser\My | Where-Object { $_.HasPrivateKey -eq $true }
    if ($smartCardCerts.Count -gt 0) {
        $tokenInserted = $true
    }
} catch {
    # 如果无法检测，假设 Token 已插入（用户需要手动确认）
    Write-Warning "无法自动检测 USB Token，请手动确认 Token 已插入"
    $tokenInserted = $true
}

if (-not $tokenInserted) {
    Write-Error "未检测到 USB Token，请插入 EV 证书 Token"
    exit 1
}

# 使用 signtool.exe 签署文件
# 注意：signtool.exe 通常随 Windows SDK 安装
# 常见路径：
# - C:\Program Files (x86)\Windows Kits\10\bin\10.0.22621.0\x64\signtool.exe
# - C:\Program Files (x86)\Windows Kits\10\App Certification Kit\signtool.exe
$signtoolPaths = @(
    "C:\Program Files (x86)\Windows Kits\10\bin\10.0.22621.0\x64\signtool.exe",
    "C:\Program Files (x86)\Windows Kits\10\bin\x64\signtool.exe",
    "C:\Program Files (x86)\Windows Kits\10\App Certification Kit\signtool.exe"
)

$signtool = $null
foreach ($path in $signtoolPaths) {
    if (Test-Path $path) {
        $signtool = $path
        break
    }
}

if (-not $signtool) {
    Write-Error "未找到 signtool.exe，请安装 Windows SDK`n常见安装方式：`n1. 安装 Visual Studio 时勾选""Windows 10 SDK""或""Windows 11 SDK""`n2. 独立下载 Windows SDK：https://developer.microsoft.com/en-us/windows/downloads/windows-sdk/"
    exit 1
}

Write-Output "使用 signtool: $signtool"

# 签署文件（使用 SHA256 算法）
# /fd SHA256 - 指定文件摘要算法为 SHA256
# /td SHA256 - 指定时间戳摘要算法为 SHA256
# /tr $TimestampServer - 使用 RFC 3161 时间戳服务器
# /tk - 使用智能卡（Token）中的证书签名
# 注意：如果 USB Token 中有多个证书，可能需要使用 /sha1 指定证书指纹
& $signtool sign /fd SHA256 /td SHA256 /tr $TimestampServer /tk $FilePath

if ($LASTEXITCODE -ne 0) {
    Write-Error "签名失败: $FilePath"
    exit 1
}

Write-Output "✅ 签名成功: $FilePath"
Write-Output "   时间戳服务器: $TimestampServer"
Write-Output "   文件路径: $(Resolve-Path $FilePath)"

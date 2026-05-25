# 创建自签名证书（仅用于测试）
# 用于 MVP 测试阶段，不推荐用于生产环境

param(
    [string]$CertName = "AI ThinApp Portable Launchpad (Test)",
    [int]$ValidityDays = 365,
    [string]$ExportPath = "",
    [string]$Password = ""
)

# 创建自签名证书
# -Type CodeSigningCert - 指定证书类型为代码签名
# -Subject - 证书主题（CN=通用名称）
# -CertStoreLocation - 证书存储位置
# -NotAfter - 证书过期时间
Write-Output "正在创建自签名证书..."
Write-Output "  名称: $CertName"
Write-Output "  有效期: $ValidityDays 天`n"

try {
    $Cert = New-SelfSignedCertificate -Type CodeSigningCert -Subject "CN=$CertName" `
        -CertStoreLocation "Cert:\CurrentUser\My" `
        -NotAfter (Get-Date).AddDays($ValidityDays) `
        -KeySpec Signature `
        -KeyUsage DigitalSignature `
        -TextExtension @("2.5.29.37={text}1.3.6.1.5.5.7.3.3")  # Code Signing EKU

    Write-Output "✅ 自签名证书已创建:"
    Write-Output "  名称: $CertName"
    Write-Output "  指纹: $($Cert.Thumbprint)"
    Write-Output "  有效期: $(Get-Date) 至 $(Get-Date).AddDays($ValidityDays)"
    Write-Output "  存储位置: Cert:\CurrentUser\My\$($Cert.Thumbprint)"

} catch {
    Write-Error "创建自签名证书失败: $_"
    exit 1
}

# 如果指定了导出路径，则导出证书（包含私钥）
if ($ExportPath -ne "") {
    Write-Output "`n正在导出证书..."
    
    if ($Password -eq "") {
        $Password = Read-Host "请输入导出密码（用于保护私钥）" -AsSecureString
    } else {
        $Password = ConvertTo-SecureString -String $Password -Force -AsPlainText
    }
    
    try {
        # 导出为 PFX 文件（包含私钥）
        Export-PfxCertificate -Cert $Cert -FilePath $ExportPath -Password $Password
        Write-Output "✅ 证书已导出: $ExportPath"
        Write-Output "  注意：请妥善保管 PFX 文件和密码！"
        
        # 同时导出为 CER 文件（仅公钥，可安全分发）
        $cerPath = [System.IO.Path]::ChangeExtension($ExportPath, ".cer")
        Export-Certificate -Cert $Cert -FilePath $cerPath
        Write-Output "✅ 公钥证书已导出: $cerPath"
        Write-Output "  可以将此文件分发给用户，让他们安装到""受信任的根证书颁发机构""存储区"
        
    } catch {
        Write-Error "导出证书失败: $_"
    }
}

Write-Output "`n⚠️ 重要提示:"
Write-Output "  1. 自签名证书会被杀毒软件标记为未知发布者"
Write-Output "  2. 用户需要手动安装证书到""受信任的根证书颁发机构""存储区"
Write-Output "  3. 仅用于内部测试和开发，不可用于生产环境"
Write-Output "  4. Windows SmartScreen 仍然会显示警告（除非用户手动信任证书）"

Write-Output "`n如何使用此证书签名:"
Write-Output "  1. 查看证书指纹: Get-ChildItem Cert:\CurrentUser\My"
Write-Output "  2. 使用证书签名: signtool sign /fd SHA256 /sha1 $($Cert.Thumbprint) /tr http://timestamp.sectigo.com <文件路径>"
Write-Output "  3. 或指定证书存储位置: signtool sign /fd SHA256 /sm /s My /n \"$CertName\" /tr http://timestamp.sectigo.com <文件路径>"

Write-Output "`n如何信任此证书（测试用）:"
Write-Output "  1. 导出 CER 文件（公钥证书）"
Write-Output "  2. 双击 CER 文件 → 安装证书"
Write-Output "  3. 选择""当前用户"" → ""将所有证书放入以下存储"" → ""受信任的根证书颁发机构"""
Write-Output "  4. 完成安装后，此证书签名的文件将被信任"

exit 0

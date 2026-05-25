# Microsoft Store 应用提交脚本
# 作者: MVP 应用商店上线工程师
# 日期: 2026-05-24
# 说明: 使用 Microsoft Store API 自动提交应用

param(
    [Parameter(Mandatory=$true)]
    [string]$AppxPath,           # 应用包路径 (.msix 或 .appx)
    
    [Parameter(Mandatory=$true)]
    [string]$ClientId,           # Azure AD 应用程序客户端 ID
    
    [Parameter(Mandatory=$true)]
    [string]$ClientSecret,       # Azure AD 应用程序客户端密钥
    
    [Parameter(Mandatory=$true)]
    [string]$TenantId,          # Azure AD 租户 ID
    
    [Parameter(Mandatory=$true)]
    [string]$AppId               # Microsoft Store 应用 ID
)

# 检查应用包文件是否存在
if (-not (Test-Path $AppxPath)) {
    Write-Error "❌ 应用包文件不存在: $AppxPath"
    exit 1
}

Write-Output "🚀 开始提交应用到 Microsoft Store..."
Write-Output "📦 应用包: $AppxPath"
Write-Output "🆔 应用 ID: $AppId"

# 获取访问令牌
function Get-AccessToken {
    param(
        [string]$ClientId,
        [string]$ClientSecret,
        [string]$TenantId
    )
    
    Write-Output "🔑 正在获取访问令牌..."
    
    $Body = @{
        grant_type = "client_credentials"
        client_id = $ClientId
        client_secret = $ClientSecret
        resource = "https://graph.microsoft.com"
    }
    
    try {
        $TokenResponse = Invoke-RestMethod -Uri "https://login.microsoftonline.com/$TenantId/oauth2/token" `
            -Method Post -Body $Body -ErrorAction Stop
        
        Write-Output "✅ 访问令牌获取成功"
        return $TokenResponse.access_token
    }
    catch {
        Write-Error "❌ 获取访问令牌失败: $_"
        exit 1
    }
}

# 上传应用包
function Upload-AppPackage {
    param(
        [string]$AccessToken,
        [string]$AppxPath
    )
    
    Write-Output "📤 正在上传应用包..."
    
    $Headers = @{
        "Authorization" = "Bearer $AccessToken"
        "Content-Type" = "application/json"
    }
    
    # 读取应用包并转换为 Base64
    $AppxBytes = [System.IO.File]::ReadAllBytes($AppxPath)
    $AppxBase64 = [Convert]::ToBase64String($AppxBytes)
    
    $Body = @{
        "appBundleUri" = $AppxBase64
    } | ConvertTo-Json
    
    try {
        $Response = Invoke-RestMethod -Uri "https://graph.microsoft.com/v1.0/me/drive/items/root:/$($AppxPath | Split-Path -Leaf):/content" `
            -Method Put -Headers $Headers -Body $Body -ErrorAction Stop
        
        Write-Output "✅ 应用包上传成功: $($Response.webUrl)"
        return $Response.id
    }
    catch {
        Write-Error "❌ 应用包上传失败: $_"
        exit 1
    }
}

# 提交应用审核
function Submit-AppReview {
    param(
        [string]$AccessToken,
        [string]$AppId,
        [string]$BundleId
    )
    
    Write-Output "📝 正在提交应用审核..."
    
    $Headers = @{
        "Authorization" = "Bearer $AccessToken"
        "Content-Type" = "application/json"
    }
    
    $Body = @{
        "appId" = $AppId
        "bundleId" = $BundleId
    } | ConvertTo-Json
    
    try {
        $Response = Invoke-RestMethod -Uri "https://graph.microsoft.com/v1.0/me/drive/items/root:/submissions:/children" `
            -Method Post -Headers $Headers -Body $Body -ErrorAction Stop
        
        Write-Output "✅ 应用审核已提交: $($Response.id)"
        Write-Output "⏳ 审核周期通常为 24-48 小时，请耐心等待"
        return $Response.id
    }
    catch {
        Write-Error "❌ 应用审核提交失败: $_"
        exit 1
    }
}

# 主流程
try {
    # 步骤 1: 获取访问令牌
    $AccessToken = Get-AccessToken -ClientId $ClientId -ClientSecret $ClientSecret -TenantId $TenantId
    
    # 步骤 2: 上传应用包
    $BundleId = Upload-AppPackage -AccessToken $AccessToken -AppxPath $AppxPath
    
    # 步骤 3: 提交审核
    $SubmissionId = Submit-AppReview -AccessToken $AccessToken -AppId $AppId -BundleId $BundleId
    
    Write-Output ""
    Write-Output "🎉 应用提交完成！"
    Write-Output "📋 提交 ID: $SubmissionId"
    Write-Output "🔗 可在 Microsoft Partner Center 查看审核状态"
    Write-Output ""
    Write-Output "📌 后续步骤:"
    Write-Output "   1. 登录 https://partner.microsoft.com/dashboard"
    Write-Output "   2. 查看应用审核状态"
    Write-Output "   3. 如被拒绝，根据反馈修复问题并重新提交"
}
catch {
    Write-Error "❌ 提交过程中发生错误: $_"
    exit 1
}

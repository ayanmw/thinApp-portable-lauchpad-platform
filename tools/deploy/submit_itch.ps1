# Itch.io 应用提交脚本
# 作者: MVP 应用商店上线工程师
# 日期: 2026-05-24
# 说明: 使用 Itch.io API 和 butler 工具自动提交应用

param(
    [Parameter(Mandatory=$true)]
    [string]$AppPath,            # 应用包路径 (.zip 或 .exe)
    
    [Parameter(Mandatory=$true)]
    [string]$ItchUsername,       # Itch.io 用户名
    
    [Parameter(Mandatory=$true)]
    [string]$ItchGameName,       # Itch.io 游戏/应用名称 (URL slug)
    
    [string]$ButlerPath = "C:\Program Files\Itch\butler.exe",  # butler 工具路径
    
    [string]$Channel = "win64",  # 发布频道 (win64, win32, mac, linux)
    
    [string]$Version = "1.0.0"  # 版本号
)

# 检查应用包文件是否存在
if (-not (Test-Path $AppPath)) {
    Write-Error "❌ 应用包文件不存在: $AppPath"
    exit 1
}

# 检查 butler 工具是否存在
if (-not (Test-Path $ButlerPath)) {
    Write-Error "❌ 未找到 butler 工具: $ButlerPath"
    Write-Output ""
    Write-Output "💡 安装 butler:"
    Write-Output "   1. 下载: https://itch.io/docs/butler/"
    Write-Output "   2. 安装后放置在: $ButlerPath"
    Write-Output "   3. 或使用 scoop/Chocolatey 安装: scoop install butler"
    exit 1
}

Write-Output "🚀 开始提交应用到 Itch.io..."
Write-Output "📦 应用包: $AppPath"
Write-Output "👤 用户名: $ItchUsername"
Write-Output "🎮 应用名称: $ItchGameName"
Write-Output "📡 频道: $Channel"
Write-Output "🔖 版本: $Version"
Write-Output ""

# 登录 Itch.io (使用 butler)
function Connect-Itch {
    param(
        [string]$ButlerPath
    )
    
    Write-Output "🔑 正在登录 Itch.io..."
    Write-Output "⚠️  首次使用需要在浏览器中授权"
    Write-Output ""
    
    $LoginProcess = Start-Process -FilePath $ButlerPath `
        -ArgumentList "login" `
        -Wait -NoNewWindow -PassThru
    
    if ($LoginProcess.ExitCode -ne 0) {
        Write-Error "❌ Itch.io 登录失败"
        exit 1
    }
    
    Write-Output "✅ Itch.io 登录成功"
}

# 上传应用包
function Upload-AppPackage {
    param(
        [string]$ButlerPath,
        [string]$AppPath,
        [string]$ItchUsername,
        [string]$ItchGameName,
        [string]$Channel,
        [string]$Version
    )
    
    Write-Output "📤 正在上传应用包..."
    Write-Output "   目标: $ItchUsername/$ItchGameName:$Channel"
    Write-Output ""
    
    $UploadProcess = Start-Process -FilePath $ButlerPath `
        -ArgumentList "push", "$AppPath", "$ItchUsername/$ItchGameName:$Channel", "--userversion", "$Version" `
        -Wait -NoNewWindow -PassThru
    
    if ($UploadProcess.ExitCode -ne 0) {
        Write-Error "❌ 应用包上传失败"
        exit 1
    }
    
    Write-Output "✅ 应用包上传成功"
}

# 更新应用信息 (可选，建议在网页端操作)
function Update-AppInfo {
    param(
        [string]$ItchUsername,
        [string]$ItchGameName
    )
    
    Write-Output ""
    Write-Output "📝 应用信息更新 (建议在网页端操作)"
    Write-Output ""
    Write-Output "📋 需要更新的信息:"
    Write-Output "   - 应用名称"
    Write-Output "   - 应用描述 (短 + 长)"
    Write-Output "   - 截图和视频"
    Write-Output "   - 定价 (建议免费 + 可选捐赠)"
    Write-Output "   - 系统需求"
    Write-Output ""
    Write-Output "🔗 访问: https://itch.io/dashboard"
}

# 主流程
try {
    # 步骤 1: 登录 Itch.io
    Connect-Itch -ButlerPath $ButlerPath
    
    # 步骤 2: 上传应用包
    Upload-AppPackage -ButlerPath $ButlerPath -AppPath $AppPath -ItchUsername $ItchUsername `
        -ItchGameName $ItchGameName -Channel $Channel -Version $Version
    
    # 步骤 3: 提示更新应用信息
    Update-AppInfo -ItchUsername $ItchUsername -ItchGameName $ItchGameName
    
    Write-Output ""
    Write-Output "🎉 应用提交完成！"
    Write-Output "🔗 应用页面: https://$ItchUsername.itch.io/$ItchGameName"
    Write-Output ""
    Write-Output "📌 后续步骤:"
    Write-Output "   1. 登录 https://itch.io/dashboard"
    Write-Output "   2. 完善应用信息 (描述、截图、定价)"
    Write-Output "   3. 发布应用 (Itch.io 无审核，即时上线)"
    Write-Output ""
    Write-Output "💡 提示: Itch.io 无审核流程，上传完成后即可在页面看到应用"
}
catch {
    Write-Error "❌ 提交过程中发生错误: $_"
    exit 1
}

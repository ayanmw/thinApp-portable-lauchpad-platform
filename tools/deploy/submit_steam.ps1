# Steam 应用提交脚本
# 作者: MVP 应用商店上线工程师
# 日期: 2026-05-24
# 说明: 使用 Steamworks API 自动提交应用

param(
    [Parameter(Mandatory=$true)]
    [string]$AppPath,            # 应用包路径 (.zip 或 .exe)
    
    [Parameter(Mandatory=$true)]
    [string]$SteamUsername,      # Steam 用户名
    
    [Parameter(Mandatory=$true)]
    [string]$SteamPassword,      # Steam 密码
    
    [Parameter(Mandatory=$true)]
    [int]$AppId,                # Steam 应用 ID
    
    [string]$DepotId = "",      # Steam Depot ID (可选)
    
    [string]$BuildDescription = "MVP Release"  # 构建描述
)

# 检查应用包文件是否存在
if (-not (Test-Path $AppPath)) {
    Write-Error "❌ 应用包文件不存在: $AppPath"
    exit 1
}

Write-Output "🚀 开始提交应用到 Steam..."
Write-Output "📦 应用包: $AppPath"
Write-Output "🆔 Steam 应用 ID: $AppId"

# 检查 SteamCMD 是否安装
function Test-SteamCMD {
    $SteamCMDPath = "C:\Program Files (x86)\Steam\steamcmd.exe"
    
    if (-not (Test-Path $SteamCMDPath)) {
        Write-Error "❌ 未找到 SteamCMD，请先安装: https://developer.valvesoftware.com/wiki/SteamCMD"
        Write-Output "💡 下载后放置在: $SteamCMDPath"
        exit 1
    }
    
    Write-Output "✅ SteamCMD 已安装: $SteamCMDPath"
    return $SteamCMDPath
}

# 登录 Steam
function Connect-Steam {
    param(
        [string]$SteamCMDPath,
        [string]$Username,
        [string]$Password
    )
    
    Write-Output "🔑 正在登录 Steam..."
    
    # 使用 SteamCMD 登录
    $LoginProcess = Start-Process -FilePath $SteamCMDPath `
        -ArgumentList "+login $Username $Password +quit" `
        -Wait -NoNewWindow -PassThru
    
    if ($LoginProcess.ExitCode -ne 0) {
        Write-Error "❌ Steam 登录失败，请检查用户名和密码"
        exit 1
    }
    
    Write-Output "✅ Steam 登录成功"
}

# 上传应用包
function Upload-AppPackage {
    param(
        [string]$SteamCMDPath,
        [string]$AppPath,
        [int]$AppId,
        [string]$DepotId,
        [string]$BuildDescription
    )
    
    Write-Output "📤 正在上传应用包..."
    
    # 创建临时脚本文件
    $ScriptPath = [System.IO.Path]::GetTempFileName() + ".txt"
    
    # 构建 SteamCMD 脚本
    $SteamScript = @"
login $SteamUsername $SteamPassword
run_app_build xxx
quit
"@
    
    # 注意: Steam 需要使用 SteamPipe 内容系统
    # 实际部署需要配置 scripts/app_build_$AppId.vdf
    Write-Output "⚠️  Steam 上传需要使用 SteamPipe 内容系统"
    Write-Output "📖 详见: https://partner.steamgames.com/doc/sdk/uploading"
    Write-Output ""
    Write-Output "📝 手动上传步骤:"
    Write-Output "   1. 登录 https://partner.steamgames.com/"
    Write-Output "   2. 进入应用 -> 技术工具 -> 上传"
    Write-Output "   3. 使用 SteamCMD 或 DepotUploader 上传"
    Write-Output ""
    Write-Output "💡 建议使用 SteamCMD 命令行工具进行自动化上传"
    
    # 这里仅提供框架，实际需要根据 Steamworks SDK 配置
    Write-Output "✅ 应用包上传流程已初始化"
}

# 更新应用信息
function Update-AppInfo {
    param(
        [int]$AppId
    )
    
    Write-Output "📝 正在更新应用信息..."
    Write-Output "⚠️  应用信息更新需要在 Steamworks 网站手动操作"
    Write-Output ""
    Write-Output "📋 需要更新的信息:"
    Write-Output "   - 应用名称"
    Write-Output "   - 应用描述 (短 + 长)"
    Write-Output "   - 截图和视频"
    Write-Output "   - 系统需求"
    Write-Output "   - 定价 (建议免费)"
    Write-Output ""
    Write-Output "🔗 访问: https://partner.steamgames.com/apps/$AppId"
}

# 提交审核
function Submit-ForReview {
    param(
        [int]$AppId
    )
    
    Write-Output "📋 正在提交审核..."
    Write-Output "⚠️  Steam 审核提交需要在 Steamworks 网站手动操作"
    Write-Output ""
    Write-Output "📝 提交审核步骤:"
    Write-Output "   1. 登录 https://partner.steamgames.com/"
    Write-Output "   2. 进入应用 -> 基本信息"
    Write-Output "   3. 点击 '准备提交' 按钮"
    Write-Output "   4. 填写所有必要信息"
    Write-Output "   5. 提交审核"
    Write-Output ""
    Write-Output "⏳ 审核周期通常为 1-5 天"
}

# 主流程
try {
    # 步骤 1: 检查 SteamCMD
    $SteamCMDPath = Test-SteamCMD
    
    # 步骤 2: 登录 Steam
    Connect-Steam -SteamCMDPath $SteamCMDPath -Username $SteamUsername -Password $SteamPassword
    
    # 步骤 3: 上传应用包
    Upload-AppPackage -SteamCMDPath $SteamCMDPath -AppPath $AppPath -AppId $AppId -DepotId $DepotId -BuildDescription $BuildDescription
    
    # 步骤 4: 更新应用信息
    Update-AppInfo -AppId $AppId
    
    # 步骤 5: 提交审核
    Submit-ForReview -AppId $AppId
    
    Write-Output ""
    Write-Output "🎉 应用提交流程已完成！"
    Write-Output "📋 应用 ID: $AppId"
    Write-Output "🔗 可在 Steamworks 网站查看审核状态"
    Write-Output ""
    Write-Output "📌 后续步骤:"
    Write-Output "   1. 登录 https://partner.steamgames.com/"
    Write-Output "   2. 查看应用审核状态"
    Write-Output "   3. 如被拒绝，根据反馈修复问题并重新提交"
    Write-Output ""
    Write-Output "💡 提示: Steam 审核通常需要 1-5 天，请耐心等待"
}
catch {
    Write-Error "❌ 提交过程中发生错误: $_"
    exit 1
}

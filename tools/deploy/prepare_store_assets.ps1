# 商店素材准备脚本 for AI ThinApp Portable Launchpad Platform
# 准备商店素材（截图、图标、描述文案）
# 所有注释使用中文

param(
    [string]$ScreenshotDir = ".\StoreAssets\Screenshots",
    [string]$IconFile = ".\StoreAssets\Icons\icon.png",
    [string]$OutputDir = ".\StoreAssets"
)

# ============================================
# 初始化
# ============================================

Write-Host "================================================" -ForegroundColor Cyan
Write-Host "AI ThinApp Portable Launchpad Platform - 商店素材准备" -ForegroundColor Cyan
Write-Host "================================================" -ForegroundColor Cyan
Write-Host "准备开始时间: $(Get-Date)" -ForegroundColor Gray
Write-Host "截图目录: $ScreenshotDir" -ForegroundColor Gray
Write-Host "图标文件: $IconFile" -ForegroundColor Gray
Write-Host "输出目录: $OutputDir" -ForegroundColor Gray
Write-Host "================================================" -ForegroundColor Cyan

# 日志文件
$LogFile = Join-Path $OutputDir "prepare_assets.log"
Start-Transcript -Path $LogFile -Force -ErrorAction SilentlyContinue

# ============================================
# 函数定义
# ============================================

function Write-Log {
    param([string]$Message)
    $Timestamp = Get-Date -Format "yyyy-MM-dd HH:mm:ss"
    Write-Host "[$Timestamp] $Message"
}

function Test-AndCreateDir {
    param([string]$Path)
    
    if (-not (Test-Path $Path)) {
        New-Item -ItemType Directory -Path $Path -Force | Out-Null
        Write-Log "创建目录: $Path"
    }
}

function Copy-AssetFile {
    param(
        [string]$Source,
        [string]$Destination
    )
    
    if (Test-Path $Source) {
        Copy-Item -Path $Source -Destination $Destination -Force
        Write-Log "已复制: $(Split-Path $Source -Leaf) -> $Destination"
        return $true
    } else {
        Write-Log "警告: 未找到文件: $Source"
        return $false
    }
}

# ============================================
# 步骤 1: 创建输出目录结构
# ============================================

Write-Log "步骤 1: 创建输出目录结构..."

# 创建主目录
Test-AndCreateDir -Path $OutputDir

# 创建子目录
$SubDirs = @(
    "Screenshots\MicrosoftStore",
    "Screenshots\Steam",
    "Icons",
    "Descriptions",
    "PromoImages"
)

foreach ($Dir in $SubDirs) {
    Test-AndCreateDir -Path (Join-Path $OutputDir $Dir)
}

Write-Log "步骤 1 完成: 目录结构创建完成"
Write-Host "================================================" -ForegroundColor Cyan

# ============================================
# 步骤 2: 准备截图
# ============================================

Write-Log "步骤 2: 准备截图..."

# Microsoft Store 截图要求
# - 尺寸: 1280x720 (16:9) 或 1920x1080 (16:9)
# - 格式: PNG 或 JPG
# - 大小: 不超过 50MB
# - 数量: 至少 1 张，最多 8 张

# Steam 截图要求
# - 尺寸: 1280x720 (16:9) 或 1920x1080 (16:9)
# - 格式: PNG 或 JPG
# - 大小: 不超过 20MB
# - 数量: 至少 2 张，最多 20 张

if (Test-Path $ScreenshotDir) {
    Write-Log "找到截图目录: $ScreenshotDir"
    
    # 获取所有截图文件
    $Screenshots = Get-ChildItem -Path $ScreenshotDir -Include *.png,*.jpg,*.jpeg -Recurse
    
    if ($Screenshots.Count -eq 0) {
        Write-Log "警告: 未找到截图文件"
    } else {
        Write-Log "找到 $($Screenshots.Count) 个截图文件"
        
        # 复制截图到 Microsoft Store 目录
        $MsIndex = 1
        foreach ($Screenshot in $Screenshots) {
            $DestPath = Join-Path $OutputDir "Screenshots\MicrosoftStore\screenshot_$MsIndex$($Screenshot.Extension)"
            Copy-AssetFile -Source $Screenshot.FullName -Destination $DestPath
            $MsIndex++
        }
        
        # 复制截图到 Steam 目录
        $SteamIndex = 1
        foreach ($Screenshot in $Screenshots) {
            $DestPath = Join-Path $OutputDir "Screenshots\Steam\screenshot_$SteamIndex$($Screenshot.Extension)"
            Copy-AssetFile -Source $Screenshot.FullName -Destination $DestPath
            $SteamIndex++
        }
    }
} else {
    Write-Log "警告: 截图目录不存在: $ScreenshotDir"
    Write-Log "提示: 请手动添加截图到 $OutputDir\Screenshots\ 目录"
}

Write-Log "步骤 2 完成: 截图准备完成"
Write-Host "================================================" -ForegroundColor Cyan

# ============================================
# 步骤 3: 准备图标
# ============================================

Write-Log "步骤 3: 准备图标..."

# Microsoft Store 图标要求
# - 尺寸: 300x300 PNG (透明背景)
# - 格式: PNG
# - 大小: 不超过 20MB

# Steam 图标要求
# - 尺寸: 512x512 PNG
# - 格式: PNG
# - 大小: 不超过 10MB

if (Test-Path $IconFile) {
    Write-Log "找到图标文件: $IconFile"
    
    # 复制到输出目录
    $IconDest = Join-Path $OutputDir "Icons\icon_512x512.png"
    Copy-AssetFile -Source $IconFile -Destination $IconDest
    
    # 创建缩略图版本 (300x300 for Microsoft Store)
    # 注意: 这里需要 ImageMagick 或类似工具来转换大小
    # 这里只是复制并重命名
    $MsIconDest = Join-Path $OutputDir "Icons\icon_300x300.png"
    Copy-AssetFile -Source $IconFile -Destination $MsIconDest
    
    Write-Log "提示: 请手动调整图标大小为 300x300 (Microsoft Store) 和 512x512 (Steam)"
} else {
    Write-Log "警告: 图标文件不存在: $IconFile"
    Write-Log "提示: 请手动添加图标到 $OutputDir\Icons\ 目录"
}

Write-Log "步骤 3 完成: 图标准备完成"
Write-Host "================================================" -ForegroundColor Cyan

# ============================================
# 步骤 4: 准备描述文案
# ============================================

Write-Log "步骤 4: 准备描述文案..."

$DescDir = Join-Path $OutputDir "Descriptions"

# Microsoft Store 描述文案
$MsDesc = @()
$MsDesc += "# Microsoft Store 描述文案"
$MsDesc += ""
$MsDesc += "## 应用名称"
$MsDesc += "AI ThinApp Portable Launchpad"
$MsDesc += ""
$MsDesc += "## 简短描述 (10-200 字符)"
$MsDesc += "轻量级便携式应用启动和管理平台，支持环境隔离。"
$MsDesc += ""
$MsDesc += "## 详细描述 (100-2000 字符)"
$MsDesc += "AI ThinApp Portable Launchpad Platform 是一个轻量级的应用程序启动和管理平台，支持便携式应用的无缝运行和环境隔离。"
$MsDesc += ""
$MsDesc += "主要特性："
$MsDesc += "• 应用程序启动和管理 - 提供简洁的 GUI 界面"
$MsDesc += "• 环境隔离（Hook 引擎）- 实现基础的 API Hook 机制"
$MsDesc += "• 配置管理 - JSON 格式的配置文件解析和应用管理"
$MsDesc += "• 进程管理 - 启动、监控和终止目标进程"
$MsDesc += ""
$MsDesc += "优势："
$MsDesc += "• 便携性 - 无需安装，可运行在 U 盘或移动设备"
$MsDesc += "• 轻量级 - 资源占用少，不影响系统性能"
$MsDesc += "• 兼容性 - 支持 Windows 10/11 x64"
$MsDesc += ""
$MsDesc += "适用场景："
$MsDesc += "• IT 管理员批量部署便携式应用"
$MsDesc += "• 开发者测试应用在不同环境下的运行"
$MsDesc += "• 普通用户管理常用便携式工具"
$MsDesc += ""
$MsDesc += "立即下载并体验！"
$MsDesc += ""

# 保存 Microsoft Store 描述文案
$MsDescFile = Join-Path $DescDir "MicrosoftStore.txt"
$MsDesc | Out-File -FilePath $MsDescFile -Encoding utf8
Write-Log "已创建: Microsoft Store 描述文案 -> $MsDescFile"

# Steam 描述文案
$SteamDesc = @()
$SteamDesc += "# Steam 描述文案"
$SteamDesc += ""
$SteamDesc += "## 应用名称"
$SteamDesc += "AI ThinApp Portable Launchpad"
$SteamDesc += ""
$SteamDesc += "## 简短描述 (不超过 255 字符)"
$SteamDesc += "轻量级便携式应用启动和管理平台，支持环境隔离。提供简洁的 GUI 界面，支持 Hook 引擎、配置管理、进程管理等功能。"
$SteamDesc += ""
$SteamDesc += "## 详细描述 (支持 HTML 格式)"
$SteamDesc += "<h1>AI ThinApp Portable Launchpad Platform</h1>"
$SteamDesc += ""
$SteamDesc += "<p>轻量级的应用程序启动和管理平台，支持便携式应用的无缝运行和环境隔离。</p>"
$SteamDesc += ""
$SteamDesc += "<h2>主要特性</h2>"
$SteamDesc += "<ul>"
$SteamDesc += "    <li><strong>应用程序启动和管理</strong> - 提供简洁的 GUI 界面用于启动和管理便携式应用</li>"
$SteamDesc += "    <li><strong>环境隔离（Hook 引擎）</strong> - 实现基础的 API Hook 机制，支持环境隔离</li>"
$SteamDesc += "    <li><strong>配置管理</strong> - JSON 格式的配置文件解析和应用管理</li>"
$SteamDesc += "    <li><strong>进程管理</strong> - 启动、监控和终止目标进程</li>"
$SteamDesc += "</ul>"
$SteamDesc += ""
$SteamDesc += "<h2>优势</h2>"
$SteamDesc += "<ul>"
$SteamDesc += "    <li><strong>便携性</strong> - 无需安装，可运行在 U 盘或移动设备</li>"
$SteamDesc += "    <li><strong>轻量级</strong> - 资源占用少，不影响系统性能</li>"
$SteamDesc += "    <li><strong>兼容性</strong> - 支持 Windows 10/11 x64</li>"
$SteamDesc += "</ul>"
$SteamDesc += ""
$SteamDesc += "<h2>适用场景</h2>"
$SteamDesc += "<ul>"
$SteamDesc += "    <li>IT 管理员批量部署便携式应用</li>"
$SteamDesc += "    <li>开发者测试应用在不同环境下的运行</li>"
$SteamDesc += "    <li>普通用户管理常用便携式工具</li>"
$SteamDesc += "</ul>"
$SteamDesc += ""
$SteamDesc += "<h2>系统要求</h2>"
$SteamDesc += "<ul>"
$SteamDesc += "    <li><strong>操作系统</strong>: Windows 10 版本 1809 或更高 / Windows 11</li>"
$SteamDesc += "    <li><strong>处理器</strong>: x64 兼容处理器</li>"
$SteamDesc += "    <li><strong>内存</strong>: 最低 4GB RAM</li>"
$SteamDesc += "    <li><strong>硬盘空间</strong>: 100MB 可用空间</li>"
$SteamDesc += "</ul>"
$SteamDesc += ""
$SteamDesc += "<p>立即下载并体验！</p>"
$SteamDesc += ""

# 保存 Steam 描述文案
$SteamDescFile = Join-Path $DescDir "Steam.txt"
$SteamDesc | Out-File -FilePath $SteamDescFile -Encoding utf8
Write-Log "已创建: Steam 描述文案 -> $SteamDescFile"

Write-Log "步骤 4 完成: 描述文案准备完成"
Write-Host "================================================" -ForegroundColor Cyan

# ============================================
# 步骤 5: 准备宣传图 (可选)
# ============================================

Write-Log "步骤 5: 准备宣传图 (可选)..."

# Steam 宣传图要求
# - 尺寸: 1920x1080 (16:9)
# - 格式: PNG 或 JPG
# - 大小: 不超过 20MB

$PromoDir = Join-Path $OutputDir "PromoImages"

# 创建示例宣传图说明文件
$PromoReadme = @()
$PromoReadme += "# 宣传图要求"
$PromoReadme += ""
$PromoReadme += "## Steam 宣传图"
$PromoReadme += "- 尺寸: 1920x1080 (16:9)"
$PromoReadme += "- 格式: PNG 或 JPG"
$PromoReadme += "- 大小: 不超过 20MB"
$PromoReadme += ""
$PromoReadme += "## 小型宣传图 (Steam)"
$PromoReadme += "- 尺寸: 230x377"
$PromoReadme += "- 格式: JPG 或 PNG"
$PromoReadme += "- 大小: 不超过 10MB"
$PromoReadme += ""
$PromoReadme += "请将宣传图放置在此目录: $PromoDir"
$PromoReadme += ""

$PromoReadmeFile = Join-Path $PromoDir "README.txt"
$PromoReadme | Out-File -FilePath $PromoReadmeFile -Encoding utf8
Write-Log "已创建: 宣传图说明文件 -> $PromoReadmeFile"

Write-Log "步骤 5 完成: 宣传图准备完成 (可选)"
Write-Host "================================================" -ForegroundColor Cyan

# ============================================
# 步骤 6: 生成素材清单
# ============================================

Write-Log "步骤 6: 生成素材清单..."

$ManifestFile = Join-Path $OutputDir "ASSET-MANIFEST.txt"
$Manifest = @()
$Manifest += "AI ThinApp Portable Launchpad Platform - 商店素材清单"
$Manifest += "准备时间: $(Get-Date)"
$Manifest += "================================================"
$Manifest += ""
$Manifest += "目录结构:"
$Manifest += "$OutputDir\"
$Manifest += "├── Screenshots\"
$Manifest += "│   ├── MicrosoftStore\"
$Manifest += "│   └── Steam\"
$Manifest += "├── Icons\"
$Manifest += "├── Descriptions\"
$Manifest += "│   ├── MicrosoftStore.txt"
$Manifest += "│   └── Steam.txt"
$Manifest += "├── PromoImages\"
$Manifest += "├── ASSET-MANIFEST.txt (本文件)"
$Manifest += "└── prepare_assets.log"
$Manifest += ""
$Manifest += "================================================"
$Manifest += "素材准备完成"
$Manifest += ""
$Manifest += "下一步:"
$Manifest += "1. 检查截图是否符合要求 (1280x720 或 1920x1080)"
$Manifest += "2. 检查图标是否符合要求 (300x300 和 512x512)"
$Manifest += "3. 修改描述文案 (如有需要)"
$Manifest += "4. 准备宣传图 (可选，Steam 需要)"
$Manifest += "5. 提交到应用商店"

# 保存清单
$Manifest | Out-File -FilePath $ManifestFile -Encoding utf8
Write-Log "素材清单已生成: $ManifestFile"

Write-Log "步骤 6 完成: 素材清单生成完成"
Write-Host "================================================" -ForegroundColor Cyan

# ============================================
# 准备完成
# ============================================

Write-Host "================================================" -ForegroundColor Green
Write-Host "商店素材准备完成!" -ForegroundColor Green
Write-Host "================================================" -ForegroundColor Green
Write-Host "准备结束时间: $(Get-Date)" -ForegroundColor Gray
Write-Host ""
Write-Host "输出目录: $OutputDir" -ForegroundColor Yellow
Write-Host ""
Write-Host "包含文件:" -ForegroundColor Yellow
Write-Host "  - 截图: $OutputDir\Screenshots\" -ForegroundColor White
Write-Host "  - 图标: $OutputDir\Icons\" -ForegroundColor White
Write-Host "  - 描述文案: $OutputDir\Descriptions\" -ForegroundColor White
Write-Host "  - 宣传图: $OutputDir\PromoImages\" -ForegroundColor White
Write-Host "  - 清单: $ManifestFile" -ForegroundColor White
Write-Host "  - 日志: $LogFile" -ForegroundColor White
Write-Host ""
Write-Host "下一步行动:" -ForegroundColor Yellow
Write-Host "  1. 检查截图是否符合要求" -ForegroundColor White
Write-Host "  2. 检查图标是否符合要求" -ForegroundColor White
Write-Host "  3. 修改描述文案 (如有需要)" -ForegroundColor White
Write-Host "  4. 提交到应用商店" -ForegroundColor White
Write-Host "================================================" -ForegroundColor Green

Stop-Transcript -ErrorAction SilentlyContinue
exit 0

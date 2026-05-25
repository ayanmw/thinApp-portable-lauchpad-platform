# 便携版 ZIP 打包脚本 for AI ThinApp Portable Launchpad Platform
# 打包便携版 ZIP，包含运行所需的所有文件
# 所有注释使用中文

param(
    [string]$BuildDir = "D:\anmw_work_proj\ai-thinApp-portable-launchpad-platform\build\output\x64\Release",
    [string]$Version = "0.1.0",
    [string]$OutputDir = "D:\anmw_work_proj\ai-thinApp-portable-launchpad-platform\build\release"
)

# ============================================
# 初始化
# ============================================

Write-Host "================================================" -ForegroundColor Cyan
Write-Host "AI ThinApp Portable Launchpad Platform - 便携版打包" -ForegroundColor Cyan
Write-Host "================================================" -ForegroundColor Cyan
Write-Host "打包开始时间: $(Get-Date)" -ForegroundColor Gray
Write-Host "构建目录: $BuildDir" -ForegroundColor Gray
Write-Host "版本号: $Version" -ForegroundColor Gray
Write-Host "输出目录: $OutputDir" -ForegroundColor Gray
Write-Host "================================================" -ForegroundColor Cyan

# 创建输出目录
if (-not (Test-Path $OutputDir)) {
    Write-Host "创建输出目录: $OutputDir" -ForegroundColor Yellow
    New-Item -ItemType Directory -Path $OutputDir -Force | Out-Null
}

# 日志文件
$LogFile = Join-Path $OutputDir "portable_package.log"
Start-Transcript -Path $LogFile -Force -ErrorAction SilentlyContinue

# ============================================
# 函数定义
# ============================================

function Write-Log {
    param([string]$Message)
    $Timestamp = Get-Date -Format "yyyy-MM-dd HH:mm:ss"
    Write-Host "[$Timestamp] $Message"
}

function Test-RequiredFile {
    param([string]$FilePath)
    
    if (-not (Test-Path $FilePath)) {
        Write-Log "错误: 未找到必需文件: $(Split-Path $FilePath -Leaf)"
        return $false
    }
    
    Write-Log "找到: $(Split-Path $FilePath -Leaf)"
    return $true
}

# ============================================
# 步骤 1: 检查构建产物
# ============================================

Write-Log "步骤 1: 检查构建产物..."

if (-not (Test-Path $BuildDir)) {
    Write-Log "错误: 构建目录不存在: $BuildDir"
    Stop-Transcript -ErrorAction SilentlyContinue
    exit 1
}

# 检查必需文件
$RequiredFiles = @(
    "launchpad.exe",
    "hook_engine.dll"
)

$MissingFiles = @()
foreach ($File in $RequiredFiles) {
    $FilePath = Join-Path $BuildDir $File
    if (-not (Test-Path $FilePath)) {
        $MissingFiles += $File
    }
}

if ($MissingFiles.Count -gt 0) {
    Write-Log "错误: 缺少必需文件，无法继续打包"
    foreach ($File in $MissingFiles) {
        Write-Log "  缺少: $File"
    }
    Stop-Transcript -ErrorAction SilentlyContinue
    exit 1
}

Write-Log "步骤 1 完成: 构建产物检查通过"
Write-Host "================================================" -ForegroundColor Cyan

# ============================================
# 步骤 2: 创建临时目录并复制文件
# ============================================

Write-Log "步骤 2: 创建临时目录并复制文件..."

# 创建临时目录
$TempDir = Join-Path $OutputDir "portable_temp"
if (Test-Path $TempDir) {
    Remove-Item -Path $TempDir -Recurse -Force
}
New-Item -ItemType Directory -Path $TempDir -Force | Out-Null
Write-Log "创建临时目录: $TempDir"

# 复制主程序和 DLL
$CoreFiles = @(
    "launchpad.exe",
    "hook_engine.dll"
)

foreach ($File in $CoreFiles) {
    $SourcePath = Join-Path $BuildDir $File
    $DestPath = Join-Path $TempDir $File
    Copy-Item -Path $SourcePath -Destination $DestPath -Force
    Write-Log "已复制: $File"
}

# 复制 Qt6 DLLs
Write-Log "正在复制 Qt6 DLLs..."

$QtDlls = @(
    "Qt6Core.dll",
    "Qt6Gui.dll",
    "Qt6Widgets.dll",
    "Qt6Network.dll"
)

$QtFound = $false
$QtPaths = @(
    "C:\Qt\6.5.0\msvc2019_64\bin",
    "C:\Qt\6.6.0\msvc2019_64\bin",
    "C:\Qt\6.7.0\msvc2019_64\bin"
)

foreach ($Path in $QtPaths) {
    if (Test-Path $Path) {
        $QtFound = $true
        Write-Log "找到 Qt 路径: $Path"
        
        foreach ($Dll in $QtDlls) {
            $SourcePath = Join-Path $Path $Dll
            $DestPath = Join-Path $TempDir $Dll
            
            if (Test-Path $SourcePath) {
                Copy-Item -Path $SourcePath -Destination $DestPath -Force
                Write-Log "已复制: $Dll"
            } else {
                Write-Log "警告: 未找到 $Dll"
            }
        }
        break
    }
}

if (-not $QtFound) {
    Write-Log "警告: 未找到 Qt6 安装路径"
    Write-Log "提示: 请确保 Qt6 DLLs 已存在于构建目录"
}

# 复制用户文档
Write-Log "正在复制用户文档..."

$ProjectRoot = "D:\anmw_work_proj\ai-thinApp-portable-launchpad-platform"
$DocFiles = @(
    "README.md",
    "LICENSE"
)

foreach ($Doc in $DocFiles) {
    $SourcePath = Join-Path $ProjectRoot $Doc
    $DestPath = Join-Path $TempDir $Doc
    
    if (Test-Path $SourcePath) {
        Copy-Item -Path $SourcePath -Destination $DestPath -Force
        Write-Log "已复制: $Doc"
    } else {
        Write-Log "警告: 未找到 $Doc"
    }
}

Write-Log "步骤 2 完成: 文件复制完成"
Write-Host "================================================" -ForegroundColor Cyan

# ============================================
# 步骤 3: 创建便携版说明文件
# ============================================

Write-Log "步骤 3: 创建便携版说明文件..."

$ReadmeContent = @()
$ReadmeContent += "# AI ThinApp Portable Launchpad Platform - 便携版"
$ReadmeContent += ""
$ReadmeContent += "版本: $Version"
$ReadmeContent += "打包时间: $(Get-Date -Format "yyyy-MM-dd HH:mm:ss")"
$ReadmeContent += ""
$ReadmeContent += "## 文件说明"
$ReadmeContent += ""
$ReadmeContent += "- launchpad.exe - 主程序"
$ReadmeContent += "- hook_engine.dll - Hook 引擎"
$ReadmeContent += "- Qt6*.dll - Qt6 运行库"
$ReadmeContent += "- README.md - 说明文档"
$ReadmeContent += "- LICENSE - 许可证"
$ReadmeContent += ""
$ReadmeContent += "## 使用方法"
$ReadmeContent += ""
$ReadmeContent += "1. 解压此 ZIP 文件到任意目录"
$ReadmeContent += "2. 运行 launchpad.exe"
$ReadmeContent += "3. 无需安装，可直接使用"
$ReadmeContent += ""
$ReadmeContent += "## 系统要求"
$ReadmeContent += ""
$ReadmeContent += "- Windows 10 版本 1809 或更高"
$ReadmeContent += "- Windows 11"
$ReadmeContent += "- 支持 x64 处理器"
$ReadmeContent += ""
$ReadmeContent += "## 注意事项"
$ReadmeContent += ""
$ReadmeContent += "- 此版本为便携版，无需安装"
$ReadmeContent += "- 可以将整个目录复制到 U 盘或其他移动存储设备"
$ReadmeContent += "- 所有配置文件保存在程序目录"
$ReadmeContent += ""
$ReadmeContent += "## 卸载方法"
$ReadmeContent += ""
$ReadmeContent += "直接删除整个解压目录即可"
$ReadmeContent += ""

# 保存说明文件
$ReadmeFile = Join-Path $TempDir "便携版使用说明.txt"
$ReadmeContent | Out-File -FilePath $ReadmeFile -Encoding utf8
Write-Log "已创建: 便携版使用说明.txt"

Write-Log "步骤 3 完成: 说明文件创建完成"
Write-Host "================================================" -ForegroundColor Cyan

# ============================================
# 步骤 4: 打包为 ZIP
# ============================================

Write-Log "步骤 4: 打包为 ZIP..."

$ZipFile = Join-Path $OutputDir "launchpad-v$Version-portable.zip"

# 删除旧文件
if (Test-Path $ZipFile) {
    Remove-Item -Path $ZipFile -Force
    Write-Log "删除旧文件: $ZipFile"
}

# 创建 ZIP 文件
try {
    Compress-Archive -Path "$TempDir\*" -DestinationPath $ZipFile -CompressionLevel Optimal
    Write-Log "ZIP 打包成功: $ZipFile"
    
    # 显示文件大小
    $FileSize = (Get-Item $ZipFile).Length / 1MB
    Write-Log "文件大小: $([math]::Round($FileSize, 2)) MB"
} catch {
    Write-Log "错误: ZIP 打包失败: $_"
    Stop-Transcript -ErrorAction SilentlyContinue
    exit 1
}

Write-Log "步骤 4 完成: ZIP 打包完成"
Write-Host "================================================" -ForegroundColor Cyan

# ============================================
# 步骤 5: 清理临时文件
# ============================================

Write-Log "步骤 5: 清理临时文件..."

if (Test-Path $TempDir) {
    Remove-Item -Path $TempDir -Recurse -Force
    Write-Log "已删除临时目录: $TempDir"
}

Write-Log "步骤 5 完成: 临时文件清理完成"
Write-Host "================================================" -ForegroundColor Cyan

# ============================================
# 步骤 6: 生成清单文件
# ============================================

Write-Log "步骤 6: 生成清单文件..."

$ManifestFile = Join-Path $OutputDir "portable_manifest.txt"
$Manifest = @()
$Manifest += "AI ThinApp Portable Launchpad Platform - 便携版清单"
$Manifest += "版本: $Version"
$Manifest += "打包时间: $(Get-Date)"
$Manifest += "================================================"
$Manifest += ""
$Manifest += "ZIP 文件: $(Split-Path $ZipFile -Leaf)"
$Manifest += "路径: $ZipFile"
$Manifest += "大小: $((Get-Item $ZipFile).Length / 1MB | ForEach-Object { [math]::Round($_, 2) }) MB"
$Manifest += ""
$Manifest += "包含文件:"
$Manifest += "  - launchpad.exe (主程序)"
$Manifest += "  - hook_engine.dll (Hook 引擎)"
$Manifest += "  - Qt6Core.dll (Qt6 核心库)"
$Manifest += "  - Qt6Gui.dll (Qt6 GUI 库)"
$Manifest += "  - Qt6Widgets.dll (Qt6 Widgets 库)"
$Manifest += "  - README.md (说明文档)"
$Manifest += "  - LICENSE (许可证)"
$Manifest += "  - 便携版使用说明.txt (使用说明)"
$Manifest += ""
$Manifest += "================================================"
$Manifest += "打包完成"

# 保存清单
$Manifest | Out-File -FilePath $ManifestFile -Encoding utf8
Write-Log "清单文件已生成: $ManifestFile"

Write-Log "步骤 6 完成: 清单文件生成完成"
Write-Host "================================================" -ForegroundColor Cyan

# ============================================
# 打包完成
# ============================================

Write-Host "================================================" -ForegroundColor Green
Write-Host "便携版打包成功完成!" -ForegroundColor Green
Write-Host "================================================" -ForegroundColor Green
Write-Host "打包结束时间: $(Get-Date)" -ForegroundColor Gray
Write-Host ""
Write-Host "输出文件:" -ForegroundColor Yellow
Write-Host "  ZIP: $ZipFile" -ForegroundColor White
Write-Host "  清单: $ManifestFile" -ForegroundColor White
Write-Host "  日志: $LogFile" -ForegroundColor White
Write-Host ""
Write-Host "使用方法:" -ForegroundColor Yellow
Write-Host "  1. 解压 ZIP 文件到任意目录" -ForegroundColor White
Write-Host "  2. 运行 launchpad.exe" -ForegroundColor White
Write-Host "  3. 无需安装，可直接使用" -ForegroundColor White
Write-Host "================================================" -ForegroundColor Green

Stop-Transcript -ErrorAction SilentlyContinue
exit 0

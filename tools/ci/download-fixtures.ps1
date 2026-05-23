# ==============================================================================
# 下载基准应用安装包
# 用途：为 POC Week 4 测试准备基准应用安装包
# 作者：Test Environment Prep Engineer
# 日期：2026-05-23
# ==============================================================================

$ErrorActionPreference = "Stop"
$ProgressPreference = 'SilentlyContinue'  # 加快下载速度

# 设置工作目录为脚本所在目录的上两级（项目根目录）
$ScriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$ProjectRoot = Split-Path -Parent (Split-Path -Parent $ScriptDir)
$FixturesDir = Join-Path $ProjectRoot "tests\fixtures"

# 创建 fixtures 目录（如果不存在）
if (-not (Test-Path $FixturesDir)) {
    New-Item -ItemType Directory -Path $FixturesDir -Force | Out-Null
    Write-Host "✅ 创建目录: $FixturesDir" -ForegroundColor Green
}

Write-Host "========================================" -ForegroundColor Cyan
Write-Host "基准应用安装包下载脚本" -ForegroundColor Cyan
Write-Host "========================================" -ForegroundColor Cyan
Write-Host ""

# ==============================================================================
# 1. 下载 Notepad++ (64-bit)
# ==============================================================================
Write-Host "📥 [1/3] 下载 Notepad++ (64-bit)..." -ForegroundColor Yellow
$NotepadPPUrl = "https://github.com/notepad-plus-plus/notepad-plus-plus/releases/download/v8.6.4/npp.8.6.4.Installer.x64.exe"
$NotepadPPOutFile = Join-Path $FixturesDir "npp.8.6.4.Installer.x64.exe"

try {
    Write-Host "   URL: $NotepadPPUrl"
    Write-Host "   保存到: $NotepadPPOutFile"
    Invoke-WebRequest -Uri $NotepadPPUrl -OutFile $NotepadPPOutFile -UseBasicParsing
    $FileSize = (Get-Item $NotepadPPOutFile).Length / 1MB
    Write-Host "   ✅ Notepad++ 下载成功 ($([math]::Round($FileSize, 2)) MB)" -ForegroundColor Green
} catch {
    Write-Host "   ❌ Notepad++ 下载失败: $($_.Exception.Message)" -ForegroundColor Red
    Write-Host "   请手动下载: $NotepadPPUrl" -ForegroundColor Yellow
}

Write-Host ""

# ==============================================================================
# 2. 下载 7-Zip (64-bit)
# ==============================================================================
Write-Host "📥 [2/3] 下载 7-Zip (64-bit)..." -ForegroundColor Yellow
$SevenZipUrl = "https://www.7-zip.org/a/7z2301-x64.exe"
$SevenZipOutFile = Join-Path $FixturesDir "7z2301-x64.exe"

try {
    Write-Host "   URL: $SevenZipUrl"
    Write-Host "   保存到: $SevenZipOutFile"
    Invoke-WebRequest -Uri $SevenZipUrl -OutFile $SevenZipOutFile -UseBasicParsing
    $FileSize = (Get-Item $SevenZipOutFile).Length / 1MB
    Write-Host "   ✅ 7-Zip 下载成功 ($([math]::Round($FileSize, 2)) MB)" -ForegroundColor Green
} catch {
    Write-Host "   ❌ 7-Zip 下载失败: $($_.Exception.Message)" -ForegroundColor Red
    Write-Host "   请手动下载: $SevenZipUrl" -ForegroundColor Yellow
}

Write-Host ""

# ==============================================================================
# 3. Firefox Portable (需要浏览器手动下载)
# ==============================================================================
Write-Host "📥 [3/3] Firefox Portable..." -ForegroundColor Yellow
Write-Host "   ⚠️  Firefox Portable 需要通过浏览器手动下载" -ForegroundColor Yellow
Write-Host ""
Write-Host "   请按照以下步骤手动下载：" -ForegroundColor Cyan
Write-Host "   1. 访问: https://portableapps.com/downloading?a=FirefoxPortable" -ForegroundColor Cyan
Write-Host "   2. 下载 Firefox Portable" -ForegroundColor Cyan
Write-Host "   3. 将下载的文件重命名为: FirefoxPortable_115.13.0_English.paf.exe" -ForegroundColor Cyan
Write-Host "   4. 将文件复制到: $FixturesDir" -ForegroundColor Cyan
Write-Host ""

# ==============================================================================
# 下载完成汇总
# ==============================================================================
Write-Host "========================================" -ForegroundColor Cyan
Write-Host "下载完成汇总" -ForegroundColor Cyan
Write-Host "========================================" -ForegroundColor Cyan
Write-Host ""

$Files = @(
    @{ Name = "Notepad++"; File = "npp.8.6.4.Installer.x64.exe"; Mandatory = $true },
    @{ Name = "7-Zip"; File = "7z2301-x64.exe"; Mandatory = $true },
    @{ Name = "Firefox Portable"; File = "FirefoxPortable_115.13.0_English.paf.exe"; Mandatory = $false }
)

$SuccessCount = 0
$TotalCount = $Files.Count

foreach ($Item in $Files) {
    $FilePath = Join-Path $FixturesDir $Item.File
    if (Test-Path $FilePath) {
        $FileSize = (Get-Item $FilePath).Length / 1MB
        Write-Host "✅ $($Item.Name): 已下载 ($([math]::Round($FileSize, 2)) MB)" -ForegroundColor Green
        $SuccessCount++
    } else {
        if ($Item.Mandatory) {
            Write-Host "❌ $($Item.Name): 未下载 (必需)" -ForegroundColor Red
        } else {
            Write-Host "⚠️  $($Item.Name): 未下载 (可选，需手动下载)" -ForegroundColor Yellow
        }
    }
}

Write-Host ""
Write-Host "成功下载: $SuccessCount / $TotalCount" -ForegroundColor Cyan

if ($SuccessCount -eq $TotalCount) {
    Write-Host "🎉 所有文件已准备就绪！" -ForegroundColor Green
} elseif ($SuccessCount -eq 2) {
    Write-Host "⚠️  请手动下载 Firefox Portable" -ForegroundColor Yellow
} else {
    Write-Host "❌ 部分必需文件下载失败，请检查网络连接后重试" -ForegroundColor Red
}

Write-Host ""
Write-Host "固件目录: $FixturesDir" -ForegroundColor Cyan
Write-Host "========================================" -ForegroundColor Cyan

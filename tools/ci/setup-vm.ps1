# ==============================================================================
# 虚拟机快速准备脚本（VMware Workstation）
# 用途：为 POC Week 4 测试准备虚拟机环境
# 前提：已安装 VMware Workstation Pro
# 注意：AI Agent 无法操作 VMware GUI，此脚本仅提供框架
# 作者：Test Environment Prep Engineer
# 日期：2026-05-23
# ==============================================================================

[CmdletBinding()]
param(
    [Parameter(Mandatory = $false)]
    [string]$VMName = "Win10-POC-Test",
    
    [Parameter(Mandatory = $false)]
    [string]$ISOPath = "C:\ISO\win10-22h2.iso",
    
    [Parameter(Mandatory = $false)]
    [string]$VMStorePath = "C:\VMs",
    
    [Parameter(Mandatory = $false)]
    [int]$MemoryMB = 4096,
    
    [Parameter(Mandatory = $false)]
    [int]$NumCPUs = 2
)

$ErrorActionPreference = "Stop"

Write-Host "========================================" -ForegroundColor Cyan
Write-Host "虚拟机快速准备脚本" -ForegroundColor Cyan
Write-Host "========================================" -ForegroundColor Cyan
Write-Host ""

# ==============================================================================
# 检查 VMware Workstation 是否安装
# ==============================================================================
Write-Host "🔍 检查 VMware Workstation..." -ForegroundColor Yellow

$VMRunPath = ""
$PossiblePaths = @(
    "C:\Program Files (x86)\VMware\VMware Workstation\vmrun.exe",
    "C:\Program Files\VMware\VMware Workstation\vmrun.exe"
)

foreach ($Path in $PossiblePaths) {
    if (Test-Path $Path) {
        $VMRunPath = $Path
        Write-Host "   ✅ 找到 vmrun: $VMRunPath" -ForegroundColor Green
        break
    }
}

if (-not $VMRunPath) {
    Write-Host "   ❌ 未找到 vmrun.exe，请先安装 VMware Workstation" -ForegroundColor Red
    Write-Host "   下载地址: https://www.vmware.com/products/workstation-pro.html" -ForegroundColor Yellow
    exit 1
}

Write-Host ""

# ==============================================================================
# 检查 ISO 文件是否存在
# ==============================================================================
Write-Host "🔍 检查 ISO 文件..." -ForegroundColor Yellow

if (-not (Test-Path $ISOPath)) {
    Write-Host "   ⚠️  ISO 文件不存在: $ISOPath" -ForegroundColor Yellow
    Write-Host "   请修改 -ISOPath 参数指向正确的 ISO 文件路径" -ForegroundColor Yellow
    Write-Host ""
    Write-Host "   推荐：Windows 10 22H2 ISO" -ForegroundColor Cyan
    Write-Host "   下载地址: https://www.microsoft.com/zh-cn/software-download/windows10" -ForegroundColor Cyan
    exit 1
} else {
    $ISOFileSize = (Get-Item $ISOPath).Length / 1GB
    Write-Host "   ✅ ISO 文件存在 ($([math]::Round($ISOFileSize, 2)) GB): $ISOPath" -ForegroundColor Green
}

Write-Host ""

# ==============================================================================
# 创建虚拟机存储目录
# ==============================================================================
Write-Host "📁 创建虚拟机存储目录..." -ForegroundColor Yellow

if (-not (Test-Path $VMStorePath)) {
    New-Item -ItemType Directory -Path $VMStorePath -Force | Out-Null
    Write-Host "   ✅ 创建目录: $VMStorePath" -ForegroundColor Green
} else {
    Write-Host "   ✅ 目录已存在: $VMStorePath" -ForegroundColor Green
}

$VMXPath = Join-Path $VMStorePath "$VMName\$VMName.vmx"

Write-Host ""

# ==============================================================================
# 创建虚拟机
# ==============================================================================
Write-Host "🖥️  创建虚拟机 $VMName..." -ForegroundColor Yellow

# 检查虚拟机是否已存在
if (Test-Path $VMXPath) {
    Write-Host "   ⚠️  虚拟机已存在: $VMXPath" -ForegroundColor Yellow
    Write-Host "   是否删除并重新创建？(Y/N)" -ForegroundColor Yellow
    $Response = Read-Host
    if ($Response -eq "Y" -or $Response -eq "y") {
        Remove-Item -Path (Split-Path $VMXPath) -Recurse -Force
        Write-Host "   ✅ 已删除旧虚拟机" -ForegroundColor Green
    } else {
        Write-Host "   ❌ 取消创建虚拟机" -ForegroundColor Red
        exit 0
    }
}

# 使用 vmrun 创建虚拟机
# 注意：vmrun createVM 功能有限，建议使用 VMware GUI 创建
Write-Host "   ⚠️  注意：vmrun 创建虚拟机功能有限" -ForegroundColor Yellow
Write-Host "   建议使用 VMware Workstation GUI 创建虚拟机：" -ForegroundColor Yellow
Write-Host "   1. 打开 VMware Workstation" -ForegroundColor Cyan
Write-Host "   2. 创建新的虚拟机 (File -> New Virtual Machine)" -ForegroundColor Cyan
Write-Host "   3. 选择典型配置 (Typical)" -ForegroundColor Cyan
Write-Host "   4. 安装来源：安装光盘镜像 (Installer disc image file, iso)" -ForegroundColor Cyan
Write-Host "   5. 选择 ISO: $ISOPath" -ForegroundColor Cyan
Write-Host "   6. 设置虚拟机名称: $VMName" -ForegroundColor Cyan
Write-Host "   7. 设置磁盘大小: 60 GB (建议)" -ForegroundColor Cyan
Write-Host "   8. 自定义硬件：内存 $MemoryMB MB, CPU $NumCPUs 核" -ForegroundColor Cyan
Write-Host ""

# ==============================================================================
# 手动创建虚拟机配置文件（替代方案）
# ==============================================================================
Write-Host "📝 创建虚拟机配置文件..." -ForegroundColor Yellow

$VMFolder = Split-Path $VMXPath
if (-not (Test-Path $VMFolder)) {
    New-Item -ItemType Directory -Path $VMFolder -Force | Out-Null
}

# 创建基础 .vmx 文件
$VMXContent = @"
.encoding = "windows-1252"
config.version = "8"
virtualHW.version = "19"
pciBridge0.present = "TRUE"
pciBridge4.present = "TRUE"
pciBridge4.virtualDev = "pcieRootPort"
pciBridge4.functions = "8"
pciBridge5.present = "TRUE"
pciBridge5.virtualDev = "pcieRootPort"
pciBridge5.functions = "8"
pciBridge6.present = "TRUE"
pciBridge6.virtualDev = "pcieRootPort"
pciBridge6.functions = "8"
pciBridge7.present = "TRUE"
pciBridge7.virtualDev = "pcieRootPort"
pciBridge7.functions = "8"
vmci0.present = "TRUE"
hpet0.present = "TRUE"
displayName = "$VMName"
guestOS = "windows9-64"
nvram = "$VMName.nvram"
virtualHW.productCompatibility = "hosted"
powerType.powerOff = "soft"
powerType.powerOn = "soft"
powerType.suspend = "soft"
powerType.reset = "soft"
memsize = "$MemoryMB"
numvcpus = "$NumCPUs"
cpuid.coresPerSocket = "1"
vcpu.hotadd = "FALSE"
vcpu.hotremove = "FALSE"
memory.hotadd = "FALSE"
scsi0.present = "TRUE"
scsi0.virtualDev = "lsisas1068"
scsi0:0.present = "TRUE"
scsi0:0.fileName = "$VMName.vmdk"
scsi0:0.size = "60"
ethernet0.present = "TRUE"
ethernet0.connectionType = "nat"
ethernet0.virtualDev = "e1000e"
ethernet0.wakeOnPcktRcv = "FALSE"
usb.present = "TRUE"
ehci.present = "TRUE"
sound.present = "TRUE"
sound.fileName = "-1"
sound.autodetect = "TRUE"
serial0.present = "FALSE"
parallel0.present = "FALSE"
serial1.present = "FALSE"
floppy0.present = "FALSE"
ide1:0.present = "TRUE"
ide1:0.fileName = "$ISOPath"
ide1:0.deviceType = "cdrom-image"
"@

$VMXContent | Out-File -FilePath $VMXPath -Encoding ascii
Write-Host "   ✅ 虚拟机配置文件已创建: $VMXPath" -ForegroundColor Green

Write-Host ""

# ==============================================================================
# 创建虚拟磁盘
# ==============================================================================
Write-Host "💾 创建虚拟磁盘..." -ForegroundColor Yellow
Write-Host "   ⚠️  需要手动创建虚拟磁盘，或使用 VMware Workstation GUI" -ForegroundColor Yellow
Write-Host "   命令示例 (需要在 VMware 安装目录下执行)：" -ForegroundColor Cyan
$VMDKPath = Join-Path $VMFolder "$VMName.vmdk"
Write-Host "   vmware-vdiskmanager.exe -c -t 0 -s 60GB -a lsilogic $VMDKPath" -ForegroundColor Cyan

Write-Host ""

# ==============================================================================
# 启动虚拟机
# ==============================================================================
Write-Host "🚀 启动虚拟机..." -ForegroundColor Yellow
Write-Host "   是否现在启动虚拟机？(Y/N)" -ForegroundColor Yellow
$Response = Read-Host

if ($Response -eq "Y" -or $Response -eq "y") {
    try {
        & $VMRunPath -T ws start $VMXPath
        Write-Host "   ✅ 虚拟机已启动" -ForegroundColor Green
        Write-Host ""
        Write-Host "   ⚠️  请手动完成 Windows 安装：" -ForegroundColor Yellow
        Write-Host "   1. 在 VMware Workstation 中查看虚拟机控制台" -ForegroundColor Cyan
        Write-Host "   2. 按照 Windows 安装向导完成安装" -ForegroundColor Cyan
        Write-Host "   3. 安装 VMware Tools (虚拟机 -> 安装 VMware Tools)" -ForegroundColor Cyan
        Write-Host "   4. 安装杀毒软件（Windows Defender / 360 / Tencent Manager）" -ForegroundColor Cyan
        Write-Host "   5. 复制测试脚本和应用到虚拟机" -ForegroundColor Cyan
    } catch {
        Write-Host "   ❌ 启动虚拟机失败: $($_.Exception.Message)" -ForegroundColor Red
    }
} else {
    Write-Host "   ⏸️  跳过启动虚拟机" -ForegroundColor Yellow
}

Write-Host ""

# ==============================================================================
# 输出后续步骤
# ==============================================================================
Write-Host "========================================" -ForegroundColor Cyan
Write-Host "后续步骤" -ForegroundColor Cyan
Write-Host "========================================" -ForegroundColor Cyan
Write-Host ""
Write-Host "1. 完成 Windows 10 22H2 安装" -ForegroundColor Yellow
Write-Host "2. 安装 VMware Tools" -ForegroundColor Yellow
Write-Host "3. 安装杀毒软件：" -ForegroundColor Yellow
Write-Host "   - Windows Defender (默认启用)" -ForegroundColor Cyan
Write-Host "   - 360 安全卫士: https://www.360.cn/" -ForegroundColor Cyan
Write-Host "   - 腾讯电脑管家: https://guanjia.qq.com/" -ForegroundColor Cyan
Write-Host "4. 复制测试文件到虚拟机：" -ForegroundColor Yellow
Write-Host "   - 测试脚本: tools\ci\run-benchmark-tests.ps1" -ForegroundColor Cyan
Write-Host "   - 基准应用安装包: tests\fixtures\*.exe" -ForegroundColor Cyan
Write-Host "   - Hook DLL: build\Release\hook_engine.dll" -ForegroundColor Cyan
Write-Host "5. 在虚拟机中运行测试" -ForegroundColor Yellow
Write-Host ""
Write-Host "虚拟机配置文件: $VMXPath" -ForegroundColor Cyan
Write-Host "========================================" -ForegroundColor Cyan

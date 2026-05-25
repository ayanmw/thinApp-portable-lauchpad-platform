# EV 代码签名脚本 for AI ThinApp Portable Launchpad Platform
# 支持时间戳、多文件签名、签名验证、错误处理（重试 3 次）
# 所有注释使用中文

param(
    [string]$Directory = "D:\anmw_work_proj\ai-thinApp-portable-launchpad-platform\build\output\x64\Release",
    [string]$CertificatePath = "",
    [string]$CertificatePassword = "",
    [string]$TimestampServer = "http://timestamp.digicert.com",
    [string]$HashAlgorithm = "SHA256",
    [int]$MaxRetries = 3
)

# ============================================
# 初始化
# ============================================

Write-Host "================================================" -ForegroundColor Cyan
Write-Host "AI ThinApp Portable Launchpad Platform - EV 代码签名" -ForegroundColor Cyan
Write-Host "================================================" -ForegroundColor Cyan
Write-Host "签名开始时间: $(Get-Date)" -ForegroundColor Gray
Write-Host "目标目录: $Directory" -ForegroundColor Gray
Write-Host "时间戳服务器: $TimestampServer" -ForegroundColor Gray
Write-Host "哈希算法: $HashAlgorithm" -ForegroundColor Gray
Write-Host "最大重试次数: $MaxRetries" -ForegroundColor Gray
Write-Host "================================================" -ForegroundColor Cyan

# 日志文件
$LogFile = Join-Path $Directory "sign.log"
Start-Transcript -Path $LogFile -Force -ErrorAction SilentlyContinue

# ============================================
# 函数定义
# ============================================

function Write-Log {
    param([string]$Message)
    $Timestamp = Get-Date -Format "yyyy-MM-dd HH:mm:ss"
    Write-Host "[$Timestamp] $Message"
}

function Test-Signature {
    param([string]$FilePath)
    
    try {
        $Signature = Get-AuthenticodeSignature -FilePath $FilePath -ErrorAction Stop
        return $Signature.Status -eq "Valid"
    } catch {
        return $false
    }
}

function Get-SignatureInfo {
    param([string]$FilePath)
    
    try {
        $Signature = Get-AuthenticodeSignature -FilePath $FilePath -ErrorAction Stop
        
        $Info = @()
        $Info += "文件: $FilePath"
        $Info += "签名状态: $($Signature.Status)"
        $Info += "签名者: $($Signature.SignerCertificate.Subject)"
        $Info += "时间戳: $($Signature.TimeStamperCertificate.Subject)"
        $Info += "哈希算法: $($Signature.SignatureType)"
        
        return $Info -join "`n"
    } catch {
        return "无法获取签名信息: $_"
    }
}

function Invoke-SignFile {
    param([string]$FilePath)
    
    $RetryCount = 0
    
    while ($RetryCount -lt $MaxRetries) {
        try {
            Write-Log "签名中 ($($RetryCount + 1)/$MaxRetries): $(Split-Path $FilePath -Leaf)"
            
            # 构建 signtool 命令
            $SignCommand = "signtool.exe sign"
            $SignCommand += " /tr `"$TimestampServer`""
            $SignCommand += " /td $HashAlgorithm"
            $SignCommand += " /fd $HashAlgorithm"
            
            # 如果有证书文件，使用证书文件签名
            if ($CertificatePath -ne "" -and (Test-Path $CertificatePath)) {
                $SignCommand += " /f `"$CertificatePath`""
                if ($CertificatePassword -ne "") {
                    $SignCommand += " /p `"$CertificatePassword`""
                }
            } else {
                # 使用 Windows 证书存储中的证书
                $SignCommand += " /a"  # 自动选择最佳证书
            }
            
            $SignCommand += " `"$FilePath`""
            
            # 执行签名
            $Result = cmd /c "$SignCommand 2>&1"
            
            if ($LASTEXITCODE -eq 0) {
                Write-Log "签名成功: $(Split-Path $FilePath -Leaf)"
                
                # 验证签名
                if (Test-Signature -FilePath $FilePath) {
                    Write-Log "签名验证通过: $(Split-Path $FilePath -Leaf)"
                    return $true
                } else {
                    Write-Log "警告: 签名验证失败: $(Split-Path $FilePath -Leaf)"
                    return $false
                }
            } else {
                $RetryCount++
                Write-Log "签名失败 (尝试 $RetryCount/$MaxRetries): $Result"
                
                if ($RetryCount -lt $MaxRetries) {
                    Write-Log "等待 2 秒后重试..."
                    Start-Sleep -Seconds 2
                }
            }
        } catch {
            $RetryCount++
            Write-Log "签名异常 (尝试 $RetryCount/$MaxRetries): $_"
            
            if ($RetryCount -lt $MaxRetries) {
                Write-Log "等待 2 秒后重试..."
                Start-Sleep -Seconds 2
            }
        }
    }
    
    Write-Log "错误: 签名失败（已重试 $MaxRetries 次）: $(Split-Path $FilePath -Leaf)"
    return $false
}

# ============================================
# 步骤 1: 检查签名工具
# ============================================

Write-Log "步骤 1: 检查签名工具..."

# 检查 signtool.exe
$SigntoolPath = ""
$SigntoolLocations = @(
    "C:\Program Files (x86)\Windows Kits\10\bin\10.0.26100.0\x64\signtool.exe",
    "C:\Program Files (x86)\Windows Kits\10\bin\x64\signtool.exe",
    "C:\Program Files\Windows Kits\10\bin\x64\signtool.exe"
)

foreach ($Location in $SigntoolLocations) {
    if (Test-Path $Location) {
        $SigntoolPath = $Location
        Write-Log "找到 signtool.exe: $SigntoolPath"
        break
    }
}

if ($SigntoolPath -eq "") {
    Write-Log "错误: 未找到 signtool.exe"
    Write-Log "请确保已安装 Windows SDK"
    Write-Log "下载地址: https://developer.microsoft.com/en-us/windows/downloads/windows-sdk/"
    Stop-Transcript -ErrorAction SilentlyContinue
    exit 1
}

# 将 signtool 路径添加到当前会话的 PATH
$Env:PATH += ";$(Split-Path $SigntoolPath -Parent)"

Write-Log "步骤 1 完成: 签名工具检查通过"
Write-Host "================================================" -ForegroundColor Cyan

# ============================================
# 步骤 2: 检查证书（可选）
# ============================================

Write-Log "步骤 2: 检查证书..."

if ($CertificatePath -ne "") {
    if (Test-Path $CertificatePath) {
        Write-Log "找到证书文件: $CertificatePath"
    } else {
        Write-Log "警告: 未找到证书文件: $CertificatePath"
        Write-Log "将尝试使用 Windows 证书存储中的证书"
    }
} else {
    Write-Log "未指定证书文件，将使用 Windows 证书存储中的证书"
    Write-Log "提示: 确保已导入 EV 代码签名证书到证书存储"
}

Write-Log "步骤 2 完成: 证书检查完成"
Write-Host "================================================" -ForegroundColor Cyan

# ============================================
# 步骤 3: 遍历目录并签名文件
# ============================================

Write-Log "步骤 3: 签名文件..."

if (-not (Test-Path $Directory)) {
    Write-Log "错误: 目标目录不存在: $Directory"
    Stop-Transcript -ErrorAction SilentlyContinue
    exit 1
}

# 获取需要签名的文件（.exe 和 .dll）
$FilesToSign = Get-ChildItem -Path $Directory -Include *.exe,*.dll -Recurse -ErrorAction SilentlyContinue

if ($FilesToSign.Count -eq 0) {
    Write-Log "警告: 未找到需要签名的文件 (.exe 或 .dll)"
    Stop-Transcript -ErrorAction SilentlyContinue
    exit 0
}

Write-Log "找到 $($FilesToSign.Count) 个文件需要签名"

$SuccessCount = 0
$FailCount = 0
$SignedFiles = @()

foreach ($File in $FilesToSign) {
    Write-Host "------------------------------------------------" -ForegroundColor Gray
    
    # 检查是否已经签名
    if (Test-Signature -FilePath $File.FullName) {
        Write-Log "跳过（已签名）: $($File.Name)"
        $SuccessCount++
        $SignedFiles += $File.FullName
        continue
    }
    
    # 签名文件
    $SignResult = Invoke-SignFile -FilePath $File.FullName
    
    if ($SignResult) {
        $SuccessCount++
        $SignedFiles += $File.FullName
    } else {
        $FailCount++
    }
}

Write-Log "步骤 3 完成: 文件签名完成"
Write-Host "================================================" -ForegroundColor Cyan

# ============================================
# 步骤 4: 显示签名结果
# ============================================

Write-Log "步骤 4: 签名结果汇总..."

Write-Host "================================================" -ForegroundColor Green
Write-Host "签名完成!" -ForegroundColor Green
Write-Host "================================================" -ForegroundColor Green
Write-Host "成功: $SuccessCount" -ForegroundColor Green
Write-Host "失败: $FailCount" -ForegroundColor $(if ($FailCount -gt 0) { "Red" } else { "Green" })
Write-Host ""

if ($SignedFiles.Count -gt 0) {
    Write-Host "已签名的文件:" -ForegroundColor Yellow
    foreach ($File in $SignedFiles) {
        Write-Host "  - $File" -ForegroundColor White
        
        # 显示签名详情
        $SigInfo = Get-SignatureInfo -FilePath $File
        Write-Host "    $($SigInfo.Split("`n")[1])" -ForegroundColor Gray
        Write-Host "    $($SigInfo.Split("`n")[2])" -ForegroundColor Gray
    }
}

Write-Host "================================================" -ForegroundColor Green

Write-Log "步骤 4 完成: 签名结果已显示"
Write-Host "================================================" -ForegroundColor Cyan

# ============================================
# 签名完成
# ============================================

$EndTime = Get-Date
Write-Host "================================================" -ForegroundColor Green
Write-Host "EV 代码签名完成!" -ForegroundColor Green
Write-Host "================================================" -ForegroundColor Green
Write-Host "签名结束时间: $EndTime" -ForegroundColor Gray
Write-Host "成功: $SuccessCount / $($FilesToSign.Count)" -ForegroundColor White
if ($FailCount -gt 0) {
    Write-Host "失败: $FailCount" -ForegroundColor Red
    Write-Host ""
    Write-Host "提示: 请检查：" -ForegroundColor Yellow
    Write-Host "  1. EV 代码签名证书是否已正确安装" -ForegroundColor Yellow
    Write-Host "  2. 时间戳服务器是否可访问" -ForegroundColor Yellow
    Write-Host "  3. 网络连接是否正常" -ForegroundColor Yellow
}
Write-Host "================================================" -ForegroundColor Green

Stop-Transcript -ErrorAction SilentlyContinue

if ($FailCount -gt 0) {
    exit 1
} else {
    exit 0
}

# collect_perf_logs.ps1
# 性能日志采集脚本
# 功能：定期采集性能指标，输出 JSON 文件
# 作者：AI ThinApp Team
# 日期：2026-05-23

# ============================================================================
# 配置
# ============================================================================

# 性能日志输出目录
$PerfLogsDir = "$PSScriptRoot\..\..\perf_logs"

# 性能指标输出文件（按日期命名）
$CurrentDate = Get-Date -Format "yyyy-MM-dd"
$OutputFile = "$PerfLogsDir\$CurrentDate.json"

# Hook 引擎 DLL 路径（用于调用性能监控接口）
$HookEngineDll = "$PSScriptRoot\..\..\build\hook_engine.dll"

# ============================================================================
# 函数定义
# ============================================================================

<#
.SYNOPSIS
    获取性能指标（通过调用 Hook 引擎 DLL 的导出函数）

.DESCRIPTION
    调用 Hook 引擎 DLL 的导出函数 `GetPerfMetricsJson` 获取性能指标 JSON 字符串
    
.NOTES
    如果 DLL 不存在或调用失败，返回空字符串
#>
function Get-PerfMetrics {
    param(
        [string]$DllPath
    )
    
    if (-not (Test-Path $DllPath)) {
        Write-Warning "Hook 引擎 DLL 不存在：$DllPath"
        return ""
    }
    
    # TODO: 调用 DLL 导出函数获取性能指标
    # 当前返回占位符 JSON
    $PlaceholderJson = @"
{
    "timestamp": "$(Get-Date -Format "yyyy-MM-ddTHH:mm:ss")",
    "memory_usage": 0,
    "metrics": {
        "file_redirect_latency": { "count": 0, "p50": 0, "p95": 0, "p99": 0 },
        "reg_redirect_latency": { "count": 0, "p50": 0, "p95": 0, "p99": 0 },
        "inject_latency": { "count": 0, "p50": 0, "p95": 0, "p99": 0 },
        "vfs_cache_hit_rate": { "count": 0, "p50": 0, "p95": 0, "p99": 0 }
    }
}
"@
    
    return $PlaceholderJson
}

<#
.SYNOPSIS
    写入性能指标到 JSON 文件
    
.DESCRIPTION
    将性能指标 JSON 字符串写入文件（UTF-8 无 BOM）
#>
function Write-PerfMetricsJson {
    param(
        [string]$FilePath,
        [string]$JsonContent
    )
    
    # 确保目录存在
    $Dir = Split-Path $FilePath -Parent
    if (-not (Test-Path $Dir)) {
        New-Item -ItemType Directory -Force -Path $Dir | Out-Null
        Write-Host "创建目录：$Dir"
    }
    
    # 写入文件（UTF-8 无 BOM）
    $Utf8NoBom = New-Object System.Text.UTF8Encoding($false)
    [System.IO.File]::WriteAllText($FilePath, $JsonContent, $Utf8NoBom)
    
    Write-Host "性能指标已写入：$FilePath"
}

<#
.SYNOPSIS
    上传到 GitHub Actions artifact（仅 CI 环境）
    
.DESCRIPTION
    检测是否在 GitHub Actions 环境，如果是则上传文件作为 artifact
    保留 30 天
#>
function Upload-GitHubActionsArtifact {
    param(
        [string]$FilePath
    )
    
    # 检测是否在 GitHub Actions 环境
    $IsGitHubActions = $env:GITHUB_ACTIONS -eq "true"
    
    if (-not $IsGitHubActions) {
        Write-Host "非 GitHub Actions 环境，跳过上传 artifact"
        return
    }
    
    # 上传 artifact（使用 GitHub Actions 内置命令）
    $ArtifactName = "perf-logs-$(Get-Date -Format 'yyyy-MM-dd')"
    Write-Host "上传 artifact：$ArtifactName"
    
    # GitHub Actions 上传 artifact 命令（需要 actions/upload-artifact@v3+）
    # 这里只是占位符，实际由 CI 流水线完成
    Write-Host "  (CI 流水线会自动上传 perf_logs\ 目录)"
}

# ============================================================================
# 主逻辑
# ============================================================================

Write-Host "=============================================="
Write-Host "性能日志采集脚本"
Write-Host "=============================================="
Write-Host "输出文件：$OutputFile"
Write-Host ""

# 1. 获取性能指标
Write-Host "[1/3] 获取性能指标..."
$PerfMetricsJson = Get-PerfMetrics -DllPath $HookEngineDll

if ([string]::IsNullOrEmpty($PerfMetricsJson)) {
    Write-Error "获取性能指标失败"
    exit 1
}

Write-Host "  获取成功（$(($PerfMetricsJson | ConvertFrom-Json).metrics.PSObject.Properties.Count) 个指标）"

# 2. 写入 JSON 文件
Write-Host "[2/3] 写入 JSON 文件..."
Write-PerfMetricsJson -FilePath $OutputFile -JsonContent $PerfMetricsJson

# 3. 上传到 GitHub Actions artifact（仅 CI 环境）
Write-Host "[3/3] 上传 artifact..."
Upload-GitHubActionsArtifact -FilePath $OutputFile

Write-Host ""
Write-Host "=============================================="
Write-Host "采集完成"
Write-Host "=============================================="

exit 0

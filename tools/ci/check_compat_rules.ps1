<#
.SYNOPSIS
检查应用是否匹配兼容性规则

.DESCRIPTION
此脚本用于检查应用安装目录中的文件是否匹配兼容性规则库中的规则。
根据匹配结果，输出建议的 Hook 配置。

.PARAMETER AppDir
应用安装目录路径

.PARAMETER RulesFile
兼容性规则文件路径（YAML 格式）

.EXAMPLE
.\check_compat_rules.ps1 -AppDir "C:\Program Files\Notepad++" -RulesFile "..\src\packager\rules.yaml"

.NOTES
作者：Windows C++ 底层开发工程师
日期：2026-05-23
#>

param(
    [Parameter(Mandatory=$true)]
    [string]$AppDir,
    
    [Parameter(Mandatory=$true)]
    [string]$RulesFile
)

# 检查应用安装目录是否存在
if (-not (Test-Path $AppDir)) {
    Write-Error "应用安装目录不存在: $AppDir"
    exit 1
}

# 检查兼容性规则文件是否存在
if (-not (Test-Path $RulesFile)) {
    Write-Error "兼容性规则文件不存在: $RulesFile"
    exit 1
}

Write-Host "[INFO] 开始检查兼容性规则..." -ForegroundColor Green
Write-Host "[INFO] 应用安装目录: $AppDir" -ForegroundColor Green
Write-Host "[INFO] 兼容性规则文件: $RulesFile" -ForegroundColor Green

# 读取规则文件（简化解析，生产环境应使用 PowerShell YAML 模块）
$rulesContent = Get-Content $RulesFile -Raw

# 解析规则（简化实现：使用正则表达式提取）
$rulePattern = 'path:\s*"([^"]+)"[\s\S]*?description:\s*"([^"]+)"'
$matches = [regex]::Matches($rulesContent, $rulePattern)

$rules = @()
foreach ($match in $matches) {
    $rule = @{
        Path = $match.Groups[1].Value
        Description = $match.Groups[2].Value
    }
    $rules += $rule
}

Write-Host "[INFO] 已加载 $($rules.Count) 条规则" -ForegroundColor Green

# 枚举应用安装目录中的所有文件
Write-Host "[INFO] 枚举应用文件..." -ForegroundColor Green
$appFiles = Get-ChildItem $AppDir -Recurse -File | Select-Object -ExpandProperty FullName

Write-Host "[INFO] 共找到 $($appFiles.Count) 个文件" -ForegroundColor Green

# 检查每个文件是否匹配规则
$matchedRules = @()
foreach ($file in $appFiles) {
    foreach ($rule in $rules) {
        $rulePattern = $rule.Path
        
        # 转换通配符模式为正则表达式
        $regexPattern = $rulePattern -replace '\*', '.*' -replace '\?', '.'
        
        # 检查是否匹配
        if ($file -match $regexPattern) {
            $matchedRule = @{
                File = $file
                Rule = $rule.Path
                Description = $rule.Description
            }
            $matchedRules += $matchedRule
            
            Write-Host "[MATCH] 文件匹配规则: $file -> $($rule.Path)" -ForegroundColor Yellow
        }
    }
}

# 输出匹配结果
Write-Host "`n[INFO] 匹配结果:" -ForegroundColor Green
Write-Host "========================================" -ForegroundColor Green

if ($matchedRules.Count -eq 0) {
    Write-Host "未匹配到任何规则" -ForegroundColor Cyan
} else {
    Write-Host "共匹配 $($matchedRules.Count) 条规则:`n" -ForegroundColor Cyan
    
    foreach ($matchedRule in $matchedRules) {
        Write-Host "  文件: $($matchedRule.File)" -ForegroundColor Cyan
        Write-Host "  规则: $($matchedRule.Rule)" -ForegroundColor Cyan
        Write-Host "  描述: $($matchedRule.Description)`n" -ForegroundColor Cyan
    }
}

# 输出建议的 Hook 配置
Write-Host "`n[INFO] 建议的 Hook 配置:" -ForegroundColor Green
Write-Host "========================================" -ForegroundColor Green

if ($matchedRules.Count -eq 0) {
    Write-Host "无需特殊 Hook 配置" -ForegroundColor Cyan
} else {
    Write-Host "请在应用中注入以下 Hook DLL:" -ForegroundColor Cyan
    Write-Host "  - hook_filesystem.dll (文件系统 Hook)" -ForegroundColor Cyan
    Write-Host "  - hook_registry.dll (注册表 Hook)`n" -ForegroundColor Cyan
    
    # 生成 Hook 配置文件（JSON 格式）
    $hookConfig = @{
        app_name = Split-Path $AppDir -Leaf
        app_dir = $AppDir
        hook_dlls = @("hook_filesystem.dll", "hook_registry.dll")
        matched_rules = $matchedRules | ForEach-Object {
            @{
                file = $_.File
                rule = $_.Rule
                description = $_.Description
            }
        }
    } | ConvertTo-Json -Depth 10
    
    $hookConfigPath = Join-Path $AppDir "hook_config.json"
    $hookConfig | Out-File $hookConfigPath -Encoding UTF8
    
    Write-Host "Hook 配置文件已生成: $hookConfigPath" -ForegroundColor Green
}

Write-Host "`n[INFO] 兼容性规则检查完成" -ForegroundColor Green

exit 0

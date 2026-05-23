# 文档术语自动修复脚本（POC 阶段可选）
# 自动替换文档中的非标准术语为标准术语

param(
    [string]$DocsDir = "docs",
    [switch]$DryRun
)

# 术语映射表（非标准 → 标准）
$termMap = @{
    "绿色软件" = "便携化应用"
    "钩子" = "Hook"
    "沙盒" = "沙箱"
    "启动器" = "Launchpad"
    "vapp 包" = ".vapp 包"
    "便携式应用" = "便携化应用"
}

# 获取脚本所在目录（用于相对路径）
$scriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
if (-not [System.IO.Path]::IsPathRooted($DocsDir)) {
    $DocsDir = Join-Path $scriptDir "..\..\$DocsDir"
    $DocsDir = [System.IO.Path]::GetFullPath($DocsDir)
}

Write-Host "========================================" -ForegroundColor Cyan
Write-Host "文档术语自动修复脚本" -ForegroundColor Cyan
Write-Host "========================================" -ForegroundColor Cyan
Write-Host "文档目录: $DocsDir" -ForegroundColor White
Write-Host "模式: $(if ($DryRun) { 'DryRun（仅检查）' } else { '正式修复' })" -ForegroundColor White
Write-Host "========================================" -ForegroundColor Cyan
Write-Host ""

# 验证文档目录是否存在
if (-not (Test-Path $DocsDir)) {
    Write-Host "❌ 错误：文档目录不存在：$DocsDir" -ForegroundColor Red
    exit 1
}

# 遍历所有 .md 文件
$totalFiles = 0
$totalReplacements = 0
$filesWithReplacements = 0

Get-ChildItem -Path $DocsDir -Filter "*.md" -Recurse | ForEach-Object {
    $file = $_.FullName
    $relativePath = $file.Replace($DocsDir, "").TrimStart('\', '/')
    $content = Get-Content $file -Raw -Encoding UTF8
    $originalContent = $content
    $fileReplacements = 0

    # 替换术语
    foreach ($term in $termMap.Keys) {
        $standardTerm = $termMap[$term]
        
        # 使用正则表达式进行替换（避免部分匹配）
        $pattern = [regex]::Escape($term)
        $matches = [regex]::Matches($content, $pattern)
        
        if ($matches.Count -gt 0) {
            if ($DryRun) {
                Write-Host "[DryRun] 在 $relativePath 中发现 '$term'（$matches.Count 处），建议替换为 '$standardTerm'" -ForegroundColor Yellow
            } else {
                $content = $content -replace $pattern, $standardTerm
                Write-Host "✅ 已替换 $relativePath 中的 '$term' → '$standardTerm'（$matches.Count 处）" -ForegroundColor Green
            }
            $fileReplacements += $matches.Count
        }
    }

    # 写回文件
    if (-not $DryRun -and $content -ne $originalContent) {
        Set-Content -Path $file -Value $content -Encoding UTF8
        $filesWithReplacements++
        $totalReplacements += $fileReplacements
    } elseif ($DryRun -and $fileReplacements -gt 0) {
        $filesWithReplacements++
        $totalReplacements += $fileReplacements
    }

    $totalFiles++
}

Write-Host ""
Write-Host "========================================" -ForegroundColor Cyan
Write-Host "修复完成！" -ForegroundColor Green
Write-Host "========================================" -ForegroundColor Cyan
Write-Host "扫描文件数: $totalFiles" -ForegroundColor White
Write-Host "发现问题的文件数: $filesWithReplacements" -ForegroundColor White
Write-Host "总替换次数: $totalReplacements" -ForegroundColor White
Write-Host "========================================" -ForegroundColor Cyan

if ($DryRun) {
    Write-Host ""
    Write-Host "💡 提示：这是 DryRun 模式，未实际修改文件。" -ForegroundColor Yellow
    Write-Host "   运行 .\fix-doc-terms.ps1 进行实际修复。" -ForegroundColor Yellow
}

Write-Host ""
Write-Host "按任意键退出..."
$null = $Host.UI.RawUI.ReadKey("NoEcho,IncludeKeyDown")

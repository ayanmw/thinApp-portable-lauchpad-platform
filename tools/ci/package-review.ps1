# POC 评审材料包打包脚本
# 用途：将评审材料复制到 review-package 目录，并打包成 ZIP

$ErrorActionPreference = "Stop"

$ProjectRoot = "D:\anmw_work_proj\ai-thinApp-portable-lauchpad-platform"
$ReviewPackage = "$ProjectRoot\review-package"
$DocsTarget = "$ReviewPackage\docs"
$TestReportsTarget = "$ReviewPackage\test-reports"
$DemosTarget = "$ReviewPackage\demos"

Write-Host "========================================" -ForegroundColor Cyan
Write-Host "POC 评审材料包打包脚本" -ForegroundColor Cyan
Write-Host "========================================" -ForegroundColor Cyan
Write-Host ""

# 创建目标目录
Write-Host "[1/5] 创建目标目录..." -ForegroundColor Yellow
$dirs = @($DocsTarget, $TestReportsTarget, $DemosTarget)
foreach ($dir in $dirs) {
    if (-not (Test-Path $dir)) {
        New-Item -ItemType Directory -Path $dir -Force | Out-Null
        Write-Host "  创建: $dir" -ForegroundColor Gray
    }
}
Write-Host "  目录创建完成" -ForegroundColor Green

# 复制技术文档到 review-package\docs\
Write-Host ""
Write-Host "[2/5] 复制技术文档..." -ForegroundColor Yellow
$DocFiles = @(
    @{Src = "$ProjectRoot\docs\SPEC.md"; Dst = "$DocsTarget\SPEC.md"; Name = "产品规格说明书" },
    @{Src = "$ProjectRoot\docs\ARCHITECTURE.md"; Dst = "$DocsTarget\ARCHITECTURE.md"; Name = "架构文档" },
    @{Src = "$ProjectRoot\docs\RISK-REGISTER.md"; Dst = "$DocsTarget\RISK-REGISTER.md"; Name = "风险登记表" },
    @{Src = "$ProjectRoot\docs\POC-PLAN.md"; Dst = "$DocsTarget\POC-PLAN.md"; Name = "POC 计划" },
    @{Src = "$ProjectRoot\docs\POC-VERIFICATION-CHECKLIST.md"; Dst = "$DocsTarget\POC-VERIFICATION-CHECKLIST.md"; Name = "POC 验证项检查表" },
    @{Src = "$ProjectRoot\docs\CODE-REVIEW-REPORT.md"; Dst = "$DocsTarget\CODE-REVIEW-REPORT.md"; Name = "代码审查报告" },
    @{Src = "$ProjectRoot\docs\TEST-ENV-CHECKLIST.md"; Dst = "$DocsTarget\TEST-ENV-CHECKLIST.md"; Name = "测试环境检查表" },
    @{Src = "$ProjectRoot\docs\POC-REVIEW-FRAMEWORK.md"; Dst = "$DocsTarget\POC-REVIEW-FRAMEWORK.md"; Name = "POC 评审框架" },
    @{Src = "$ProjectRoot\docs\POC-REVIEW-MATERIALS-CHECKLIST.md"; Dst = "$DocsTarget\POC-REVIEW-MATERIALS-CHECKLIST.md"; Name = "POC 评审材料清单" },
    @{Src = "$ProjectRoot\docs\POC-REVIEW-APPLICATION.md"; Dst = "$DocsTarget\POC-REVIEW-APPLICATION.md"; Name = "POC 评审申请表" },
    @{Src = "$ProjectRoot\docs\POC-VERIFICATION-REPORT.md"; Dst = "$DocsTarget\POC-VERIFICATION-REPORT.md"; Name = "POC 验证报告" },
    @{Src = "$ProjectRoot\docs\POC-REVIEW-PREP-REPORT.md"; Dst = "$DocsTarget\POC-REVIEW-PREP-REPORT.md"; Name = "POC 评审准备报告" }
)

$DocSuccess = 0
$DocFail = 0
foreach ($doc in $DocFiles) {
    if (Test-Path $doc.Src) {
        Copy-Item -Path $doc.Src -Destination $doc.Dst -Force
        Write-Host "  ✅ $($doc.Name)" -ForegroundColor Green
        $DocSuccess++
    } else {
        Write-Host "  ❌ $($doc.Name) (源文件不存在: $($doc.Src))" -ForegroundColor Red
        $DocFail++
    }
}
Write-Host "  技术文档复制完成 ($DocSuccess 成功, $DocFail 失败)" -ForegroundColor Green

# 复制 MVP 文档
Write-Host ""
Write-Host "[3/5] 复制 MVP 文档..." -ForegroundColor Yellow
$MvpFiles = @(
    @{Src = "$ProjectRoot\MVP-SCOPE.md"; Dst = "$DocsTarget\MVP-SCOPE.md"; Name = "MVP 范围文档" },
    @{Src = "$ProjectRoot\MVP-PLAN.md"; Dst = "$DocsTarget\MVP-PLAN.md"; Name = "MVP 计划" }
)

$MvpSuccess = 0
$MvpFail = 0
foreach ($mvp in $MvpFiles) {
    if (Test-Path $mvp.Src) {
        Copy-Item -Path $mvp.Src -Destination $mvp.Dst -Force
        Write-Host "  ✅ $($mvp.Name)" -ForegroundColor Green
        $MvpSuccess++
    } else {
        Write-Host "  ⚠️  $($mvp.Name) (源文件不存在: $($mvp.Src))" -ForegroundColor Yellow
        $MvpFail++
    }
}
Write-Host "  MVP 文档复制完成 ($MvpSuccess 成功, $MvpFail 跳过)" -ForegroundColor Green

# 复制测试报告到 review-package\test-reports\
Write-Host ""
Write-Host "[4/5] 复制测试报告..." -ForegroundColor Yellow
$TestFiles = @(
    @{Src = "$ProjectRoot\docs\WEEK2-V2-TEST-REPORT.md"; Dst = "$TestReportsTarget\V2-filesystem-redirection.md"; Name = "V2 文件系统重定向测试报告" },
    @{Src = "$ProjectRoot\docs\WEEK2-V3-TEST-REPORT.md"; Dst = "$TestReportsTarget\V3-registry-redirection.md"; Name = "V3 注册表重定向测试报告" },
    @{Src = "$ProjectRoot\docs\WEEK2-V4-TEST-REPORT.md"; Dst = "$TestReportsTarget\V4-child-process-inheritance.md"; Name = "V4 子进程继承测试报告" },
    @{Src = "$ProjectRoot\docs\BENCHMARK-TEST-REPORT.md"; Dst = "$TestReportsTarget\V7-benchmark-apps.md"; Name = "V7 基准应用验证报告" },
    @{Src = "$ProjectRoot\docs\PORTABILITY-TEST-REPORT.md"; Dst = "$TestReportsTarget\V8-portability.md"; Name = "V8 便携性测试报告" },
    @{Src = "$ProjectRoot\docs\ANTIVIRUS-TEST-REPORT.md"; Dst = "$TestReportsTarget\V6-antivirus-compatibility.md"; Name = "V6 杀软兼容性测试报告" }
)

$TestSuccess = 0
$TestFail = 0
foreach ($test in $TestFiles) {
    if (Test-Path $test.Src) {
        Copy-Item -Path $test.Src -Destination $test.Dst -Force
        Write-Host "  ✅ $($test.Name)" -ForegroundColor Green
        $TestSuccess++
    } else {
        Write-Host "  ⚠️  $($test.Name) (源文件不存在: $($test.Src))" -ForegroundColor Yellow
        $TestFail++
    }
}
Write-Host "  测试报告复制完成 ($TestSuccess 成功, $TestFail 待完成)" -ForegroundColor Green

# 创建演示视频 README
Write-Host ""
Write-Host "[5/5] 创建演示视频说明..." -ForegroundColor Yellow
$DemoReadme = "$DemosTarget\README.md"
$DemoContent = @"
# 演示视频录制说明

## 演示 1：Hook 引擎工作原理

**时长**：3-5 分钟  
**内容**：
1. 启动测试程序 \`test_hook.exe\`，展示 Hook 安装/卸载
2. 启动 Notepad++，展示文件操作重定向（使用 Process Monitor 监控）
3. 启动 7-Zip，展示注册表操作重定向（使用 RegEdit 监控）
4. 启动 Firefox Portable，展示子进程继承（使用 Process Monitor 监控）

**录制工具**：OBS Studio（免费）  
**输出格式**：MP4（H.264，1080p）  
**输出文件**：\`hook-engine-demo.mp4\`

---

## 演示 2：Launchpad 原型交互

**时长**：3-5 分钟  
**内容**：
1. 打开 Launchpad 主界面，展示应用列表
2. 点击应用图标，启动应用（展示 Hook 注入）
3. 打开应用详情页，展示沙箱状态
4. 打开应用商店，展示浏览/搜索/安装流程
5. 打开新手引导，展示 3 步引导流程

**录制工具**：OBS Studio（免费）  
**输出格式**：MP4（H.264，1080p）  
**输出文件**：\`launchpad-prototype-demo.mp4\`

---

## 录制检查清单

- [ ] 音频清晰（麦克风无杂音）
- [ ] 屏幕分辨率 1080p
- [ ] 操作步骤流畅（无卡顿）
- [ ] 关键界面有文字标注（使用 OBS 文本插件）
- [ ] 文件保存至 \`$DemosTarget\` 目录
"@
Set-Content -Path $DemoReadme -Value $DemoContent -Encoding UTF8
Write-Host "  ✅ 演示视频录制说明已创建 ($DemoReadme)" -ForegroundColor Green

# 打包成 ZIP
Write-Host ""
Write-Host "========================================" -ForegroundColor Cyan
Write-Host "打包评审材料包..." -ForegroundColor Cyan
Write-Host "========================================" -ForegroundColor Cyan

$ZipFile = "$ProjectRoot\review-package-$(Get-Date -Format 'yyyyMMdd-HHmmss').zip"
Write-Host "  目标文件: $ZipFile" -ForegroundColor Gray

try {
    Compress-Archive -Path "$ReviewPackage\*" -DestinationPath $ZipFile -CompressionLevel Optimal
    Write-Host "  ✅ 打包成功！" -ForegroundColor Green
    Write-Host "  文件大小: $((Get-Item $ZipFile).Length / 1MB) MB" -ForegroundColor Gray
} catch {
    Write-Host "  ❌ 打包失败: $_" -ForegroundColor Red
    exit 1
}

# 输出汇总报告
Write-Host ""
Write-Host "========================================" -ForegroundColor Cyan
Write-Host "打包汇总" -ForegroundColor Cyan
Write-Host "========================================" -ForegroundColor Cyan
Write-Host "  技术文档: $DocSuccess 个" -ForegroundColor Gray
Write-Host "  MVP 文档: $MvpSuccess 个" -ForegroundColor Gray
Write-Host "  测试报告: $TestSuccess 个" -ForegroundColor Gray
Write-Host "  演示说明: 1 个" -ForegroundColor Gray
Write-Host "  ZIP 文件: $ZipFile" -ForegroundColor Gray
Write-Host ""
Write-Host "✅ 评审材料包准备完成！" -ForegroundColor Green
Write-Host ""
Write-Host "下一步:" -ForegroundColor Yellow
Write-Host "  1. 检查 review-package\ 目录内容是否完整" -ForegroundColor Gray
Write-Host "  2. 补充缺失的测试报告（V2、V3、V4、V6、V7、V8）" -ForegroundColor Gray
Write-Host "  3. 录制演示视频（hook-engine-demo.mp4、launchpad-prototype-demo.mp4）" -ForegroundColor Gray
Write-Host "  4. 将演示视频复制到 review-package\demos\ 目录" -ForegroundColor Gray
Write-Host "  5. 重新运行本脚本，生成最终的评审材料包 ZIP" -ForegroundColor Gray

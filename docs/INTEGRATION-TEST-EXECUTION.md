# 集成测试执行方案 (Integration Test Execution Plan)

**项目:** AI ThinApp Portable Launchpad Platform
**POC 阶段:** Week 4 (最终集成测试)
**文档版本:** V1.0
**更新日期:** 2026-05-23

---

## 1. 测试执行流程

| 日期 | 任务内容                                                                 | 负责人 | 交付物 |
| :--- | :----------------------------------------------------------------------- | :----- | :----- |
| Day 1 | 准备测试环境（参考 `docs\TEST-ENV-CHECKLIST.md`）：<br>- 安装 Visual Studio 2022（编译 Hook DLL）<br>- 下载基准应用安装包（Notepad++, 7-Zip, Firefox Portable）<br>- 安装虚拟机（Win10 22H2, Win11 23H2）<br>- 配置杀软（Windows Defender, 360, Tencent Manager） | 测试团队 | 测试环境就绪 |
| Day 2 | 执行模块间接口测试（IT-01 ~ IT-03）：<br>- 执行测试，记录结果<br>- 若失败，调试并修复 Bug，重新测试 | 测试团队 | 模块间接口测试报告 |
| Day 3 | 执行端到端流程测试（步骤 1 ~ 5）：<br>- 执行测试，记录结果<br>- 若失败，调试并修复 Bug，重新测试 | 测试团队 | 端到端流程测试报告 |
| Day 4 | 执行异常场景测试（IT-04 ~ IT-06）和性能测试（IT-07 ~ IT-08）：<br>- 执行测试，记录结果<br>- 若失败，调试并修复 Bug，重新测试 | 测试团队 | 异常场景与性能测试报告 |
| Day 5 | 输出集成测试报告，提交 POC 评审                                           | 测试团队 | 集成测试报告、POC 评审材料 |

---

## 2. 测试自动化脚本

### 2.1 脚本路径
`tools\ci\run-integration-tests.ps1`

### 2.2 脚本功能
- 自动设置测试环境变量
- 校验应用安装状态
- 调用应用捕获工具生成 .vapp 包
- 调用 Launchpad 安装并启动应用
- 检查应用启动状态
- 输出测试结果

### 2.3 脚本内容
```powershell
# 集成测试执行脚本（POC Week 4）
# 前提：Hook DLL 已编译，应用捕获工具已编译，Launchpad UI 已编译

param(
    [string]$TestApp = "Notepad++",
    [string]$VAppPackage = "D:\MyApps\Notepad++.vapp"
)

# 1. 设置环境变量
$env:AI_THINAPP_APP_DIR = "D:\MyApps\$TestApp"
$env:AI_THINAPP_HOOK_DLL = "..\build\Release\hook_engine.dll"
$env:AI_THINAPP_CAPTURE_MODE = "1"

# 2. 校验应用安装状态
$appExePath = "D:\Program Files\Notepad++\notepad++.exe"
if (-not (Test-Path $appExePath)) {
    Write-Host "❌ 应用未安装：$appExePath" -ForegroundColor Red
    exit 1
}

# 3. 捕获应用（调用应用捕获工具 CLI）
Write-Host "开始捕获应用..."
Start-Process "..\build\Release\app_capture.exe" -ArgumentList "--output", $VAppPackage -Wait

# 4. 检查 .vapp 包是否生成
if (-not (Test-Path $VAppPackage)) {
    Write-Host "❌ .vapp 包生成失败：$VAppPackage" -ForegroundColor Red
    exit 1
}

# 5. 安装 .vapp 包（调用 Launchpad CLI）
Write-Host "安装 .vapp 包..."
Start-Process "..\build\Release\launchpad.exe" -ArgumentList "--install", $VAppPackage -Wait

# 6. 启动应用（通过 Launchpad）
Write-Host "启动应用..."
$launchResult = Start-Process "..\build\Release\launchpad.exe" -ArgumentList "--launch", $TestApp -Wait -PassThru

# 7. 检查应用是否启动成功
Start-Sleep -Seconds 5
$process = Get-Process | Where-Object { $_.ProcessName -like "*$TestApp*" }
if ($process) {
    Write-Host "✅ $TestApp 已启动 (PID: $($process.Id))" -ForegroundColor Green
} else {
    Write-Host "❌ $TestApp 启动失败" -ForegroundColor Red
    exit 1
}

Write-Host "集成测试核心流程执行完成！" -ForegroundColor Green
```

### 2.4 脚本使用说明
1. 打开 PowerShell 终端
2. 切换到项目根目录
3. 执行脚本：`.\tools\ci\run-integration-tests.ps1 -TestApp "Notepad++" -VAppPackage "D:\MyApps\Notepad++.vapp"`
4. 查看输出结果，记录测试状态

---

## 3. 测试环境清理
- 测试完成后，删除临时快照目录（`C:\Snapshots\`）
- 卸载测试应用，恢复系统环境
- 归档测试日志和报告
# AI ThinApp Portable Launchpad — 企业部署指南

> 面向 IT 管理员的部署和管理指南
> 版本：v0.1.0（MVP）
> 更新日期：2026-05-24

---

## 目录

- [第 1 章：系统要求](#第-1-章系统要求)
- [第 2 章：安装方式](#第-2-章安装方式)
- [第 3 章：配置管理](#第-3-章配置管理)
- [第 4 章：更新管理](#第-4-章更新管理)
- [第 5 章：故障排除](#第-5-章故障排除)

---

## 第 1 章：系统要求

### 1.1 最低系统要求

| 项目 | 要求 |
|------|------|
| 操作系统 | Windows 10 22H2（19044+）/ Windows 11 23H2（22631+） |
| 处理器 | x64（64 位），1 GHz 或更高 |
| 内存 | 4 GB RAM（建议 8 GB） |
| 磁盘空间 | 500 MB（Launchpad 本身）+ 每个应用约 100 MB~2 GB（视应用而定） |
| 网络 | 需要联网（下载应用包、检查更新） |
| 权限 | 管理员权限（首次安装和 Hook DLL 注入需要） |

### 1.2 推荐配置

| 项目 | 推荐 |
|------|------|
| 操作系统 | Windows 11 23H2+ |
| 处理器 | x64，2 GHz 或更高 |
| 内存 | 8 GB 或更高 |
| 磁盘空间 | 2 GB 以上可用空间 |
| 磁盘类型 | SSD（推荐，提升 Hook 引擎性能） |

### 1.3 不兼容的软件

以下类型的软件可能与 AI ThinApp 的沙箱引擎冲突：

| 类型 | 说明 | 建议 |
|------|------|------|
| 反作弊系统 | 直接使用 Syscall 绕过 ntdll，Hook 无法拦截 | 以真实模式运行 |
| 杀毒软件 | 可能与 DLL 注入冲突 | 添加白名单 |
| 其他 Hook 工具 | Hook 可能被覆盖 | 注意加载顺序 |
| 虚拟化软件（VMware/Hyper-V） | 部分功能可能与沙箱冲突 | 测试后使用 |

### 1.4 网络要求

| 功能 | 需要联网 | 说明 |
|------|----------|------|
| Launchpad 基本功能 | ❌ 否 | 应用管理、启动、停止等 |
| 软件商店（下载应用） | ✅ 是 | 需要 HTTPS 访问社区服务器 |
| 自动更新检查 | ✅ 是 | 需要 HTTPS 访问 GitHub |
| 云端同步（V2，未发布） | ✅ 是 | 加密同步应用数据 |

**防火墙规则**：

如果企业网络有防火墙，请开放以下出站规则：

| 目标地址 | 端口 | 协议 | 用途 |
|----------|------|------|------|
| `github.com` | 443 | HTTPS | 下载更新 |
| `github.io` | 443 | HTTPS | 应用商店 API |
| `api.thinapp.local`（待定） | 443 | HTTPS | 社区服务器（V2） |

---

## 第 2 章：安装方式

### 2.1 标准安装（交互式）

适用于单个用户安装。

**步骤**：

1. 下载 `ai-thinapp-v0.1.0-setup.exe`
2. 以管理员权限运行安装包
3. 按向导完成安装

**安装后文件**：

```
C:\Program Files\AI ThinApp\
├── ai-thinapp.exe              # Launchpad 主程序
├── hook_engine.dll             # 沙箱引擎 DLL
├── resources/                  # 资源文件
├── locales/                    # 本地化文件
└── ...
```

### 2.2 静默安装（批量部署）

适用于企业批量部署，支持命令行参数。

**静默安装命令**：

```bat
:: 默认路径安装
ai-thinapp-v0.1.0-setup.exe /S /D=C:\Program Files\AI ThinApp

:: 自定义路径安装
ai-thinapp-v0.1.0-setup.exe /S /D=D:\AIThinApp

:: 安装时添加杀软白名单（Windows Defender）
:: 需要在安装后通过 Group Policy 或 PowerShell 添加
```

**命令行参数**：

| 参数 | 说明 |
|------|------|
| `/S` | 静默安装（无 UI） |
| `/D=<路径>` | 指定安装路径 |
| `/L=<路径>` | 指定日志文件路径 |

**Group Policy 静默安装**：

```powershell
# 通过 Group Policy 的"计算机配置 → 软件安装"部署
# 或使用 Group Policy Preferences 的"注册表"或"文件"项

# 或使用 PowerShell DSC
Configuration InstallAIThinApp {
    Node 'localhost' {
        Package AIThinApp {
            Ensure = 'Present'
            Name = 'AI ThinApp Portable Launchpad'
            Path = 'C:\Downloads\ai-thinapp-v0.1.0-setup.exe'
            Arguments = '/S /D=C:\Program Files\AI ThinApp'
        }
    }
}
InstallAIThinApp
```

### 2.3 杀软白名单（Group Policy）

**Windows Defender 白名单（Group Policy）**：

```powershell
# 添加文件夹排除项
Add-MpPreference -ExclusionPath "C:\Program Files\AI ThinApp\"

# 添加文件排除项
Add-MpPreference -ExclusionProcess "ai-thinapp.exe"
Add-MpPreference -ExclusionProcess "hook_engine.dll"
```

**通过 Group Policy 下发**：

1. 打开 Group Policy Management Console (gpedit.msc)
2. 导航到：`计算机配置 → 管理模板 → Windows 组件 → Microsoft Defender 防病毒 → 排除项`
3. 配置"路径排除项"：`C:\Program Files\AI ThinApp\`
4. 配置"进程排除项"：`ai-thinapp.exe`、`hook_engine.dll`
5. 运行 `gpupdate /force` 应用策略

### 2.4 便携版（无需安装）

> 便携版（Green Version）计划在未来版本提供。

**当前状态**：MVP 阶段仅提供安装版（.exe 安装包）。便携版计划在下一次大版本更新中提供。

---

## 第 3 章：配置管理

### 3.1 配置文件

Launchpad 的配置文件存储位置：

| 配置类型 | 位置 |
|----------|------|
| 用户配置 | `%APPDATA%\AI ThinApp\config.json` |
| 应用列表 | `%APPDATA%\AI ThinApp\apps.json` |
| 日志文件 | `%LOCALAPPDATA%\AI ThinApp\Logs\` |
| 全局缓存 | `%LOCALAPPDATA%\AI ThinApp\Cache\` |

**config.json 示例**：

```json
{
  "version": "0.1.0",
  "theme": "dark",
  "viewMode": "card",
  "autoStart": false,
  "minimizeToTray": true,
  "autoCheckUpdate": true,
  "hookSettings": {
    "enabled": true,
    "redirectFile": true,
    "redirectRegistry": true,
    "verboseLog": false
  },
  "sandboxPath": "",
  "defaultAppsPath": "C:\\ThinAppApps"
}
```

### 3.2 Group Policy 配置（企业场景）

> 注意：MVP 版本暂未内置 Group Policy 模板（.admx），计划在企业版中提供。以下为通过 PowerShell 实现的等效配置。

**禁用自动更新**：

```powershell
# 修改 config.json
$configPath = "$env:APPDATA\AI ThinApp\config.json"
$config = Get-Content $configPath | ConvertFrom-Json
$config.autoCheckUpdate = $false
$config | ConvertTo-Json | Set-Content $configPath
```

**强制使用指定沙箱路径**：

```powershell
$config.sandboxPath = "D:\CorporateSandbox"
$config | ConvertTo-Json | Set-Content $configPath
```

**禁用软件商店**：

```powershell
# 当前版本无此配置项，计划在下一次更新中添加
```

### 3.3 应用批量部署

通过 .vapp 包批量部署应用到多台电脑。

**步骤**：

1. 准备应用 .vapp 包（如 `notepad++.vapp`）
2. 将 .vapp 包复制到网络共享目录
3. 通过脚本或 Group Policy 分发

**批量安装脚本**：

```powershell
# 批量安装 .vapp 包
$vappPath = "\\server\thinapp-apps\"
$installPath = "C:\ThinAppApps\"

Get-ChildItem "$vappPath*.vapp" | ForEach-Object {
    # 调用 Launchpad 的命令行接口安装
    & "C:\Program Files\AI ThinApp\ai-thinapp.exe" --install-vapp $_.FullName --install-dir $installPath
    Write-Host "Installed: $($_.Name)"
}
```

### 3.4 日志管理

**日志文件位置**：`%LOCALAPPDATA%\AI ThinApp\Logs\`

**日志类型**：

| 日志文件 | 说明 |
|----------|------|
| `hook_engine.log` | Hook 引擎日志（拦截和重定向操作） |
| `launchpad.log` | Launchpad 运行日志 |
| `crash_report_<timestamp>.txt` | 崩溃报告 |

**日志级别**：

| 级别 | 说明 | 建议 |
|------|------|------|
| Error | 仅记录错误 | 生产环境推荐 |
| Warning | 记录警告和错误 | 默认 |
| Info | 记录常规信息 | 调试用 |
| Debug | 详细日志 | 排查问题时使用 |

**日志轮转**：

- 日志文件自动轮转，单个日志文件最大 10 MB
- 保留最近 5 个日志文件
- 超过限制的日志文件自动删除

---

## 第 4 章：更新管理

### 4.1 更新方式

| 方式 | 说明 |
|------|------|
| 自动更新 | Launchpad 启动时自动检查更新（默认开启） |
| 手动更新 | 设置 → 关于 → 检查更新 |
| 批量更新 | IT 管理员手动分发新版本安装包 |

### 4.2 手动更新

**步骤**：

1. 前往 [GitHub Releases](https://github.com/anmingwei/ai-thinApp-portable-lauchpad-platform/releases)
2. 下载最新版本的安装包
3. 验证 SHA-256 校验和
4. 运行新版本安装包（覆盖安装）
5. 旧版本的用户配置和应用列表会自动迁移

**覆盖安装说明**：

- 用户配置（config.json）保留
- 应用列表保留
- 应用沙箱数据保留
- 仅程序文件被更新

### 4.3 批量更新（企业场景）

**方式一：通过安装包静默更新**

```bat
:: 静默覆盖安装
ai-thinapp-v0.1.1-setup.exe /S /D=C:\Program Files\AI ThinApp /Upgrade
```

**方式二：通过 Group Policy 分发**

```powershell
# 通过 SCCM 或 Intune 分发 MSI/EXE 安装包
# 或使用 Group Policy 的"计算机启动脚本"
```

### 4.4 回滚

如果需要回滚到旧版本：

1. 卸载当前版本
2. 安装旧版本（.vapp 数据不丢失）
3. 确认应用正常运行

> **注意**：回滚后，用户配置可能不兼容。建议先备份 config.json。

---

## 第 5 章：故障排除

### 5.1 常见部署问题

| 问题 | 原因 | 解决方法 |
|------|------|----------|
| 安装失败 | 权限不足 | 以管理员权限运行安装包 |
| 启动闪退 | 杀软拦截 | 添加白名单 |
| Hook 注入失败 | 杀软拦截 / 权限不足 | 添加白名单 + 管理员权限 |
| 应用启动失败 | 沙箱冲突 / 反作弊 | 切换真实模式 |
| 性能下降 | 详细日志开启 | 关闭详细日志 |

### 5.2 日志收集

**收集用户日志**：

```powershell
# 收集指定用户的日志
$logPath = "$env:LOCALAPPDATA\AI ThinApp\Logs\"
$archive = "C:\Temp\AIThinApp_Logs_$env:USERNAME.zip"
Compress-Archive -Path "$logPath*" -DestinationPath $archive
```

**收集所有用户日志（管理员）**：

```powershell
# 收集所有用户的日志
$logs = @()
Get-ChildItem "C:\Users" -Directory | ForEach-Object {
    $userLogPath = "$($_.FullName)\AppData\Local\AI ThinApp\Logs\"
    if (Test-Path $userLogPath) {
        $logs += Get-ChildItem $userLogPath -File
    }
}
Compress-Archive -Path $logs -DestinationPath "C:\Temp\AIThinApp_AllUser_Logs.zip"
```

### 5.3 崩溃报告

**崩溃报告位置**：`%LOCALAPPDATA%\AI ThinApp\Logs\crash_report_<timestamp>.txt`

**崩溃报告内容**：

- 操作系统信息
- Launchpad 版本
- 崩溃时间
- 堆栈跟踪（Stack Trace）
- 最近的操作日志

**如何提交崩溃报告**：

1. 找到最新的崩溃报告文件
2. 到 [GitHub Issues](https://github.com/anmingwei/ai-thinApp-portable-lauchpad-platform/issues) 创建新 Issue
3. 附上崩溃报告文件和复现步骤

### 5.4 性能问题排查

**Hook 引擎性能**：

1. 检查 Hook 引擎日志中是否有异常频繁的拦截
2. 确认未开启"详细日志"（影响性能）
3. 使用 Process Monitor 监控文件/注册表操作频率

**内存泄漏检测**：

1. 使用任务管理器观察 AI ThinApp 相关进程的内存使用
2. 目标：24 小时内内存增长 < 5 MB
3. 如果内存持续增长，可能是 Hook 引擎内存泄漏，请报告

### 5.5 联系支持

| 渠道 | 链接 | 用途 |
|------|------|------|
| GitHub Issues | [链接](https://github.com/anmingwei/ai-thinApp-portable-lauchpad-platform/issues) | Bug 报告、技术支持 |
| GitHub Discussions | [链接](https://github.com/anmingwei/ai-thinApp-portable-lauchpad-platform/discussions) | 使用问题、部署咨询 |
| 企业版支持 | 待定（Phase 4） | 企业专属技术支持 |

---


---

## 第 6 章：CI/CD 流水线配置

### 6.1 流水线概述

本项目使用 GitHub Actions 实现 CI/CD 自动化。配置文件位于 `.github/workflows/ci.yml`。

**主要功能**：

| 功能 | 说明 | 触发条件 |
|------|------|----------|
| 自动构建 | 编译 Release 和 Debug 版本 | Push 到 main 分支、Pull Request |
| 自动测试 | 运行单元测试和集成测试 | Push 到 main 分支、Pull Request |
| 自动打包 | 生成安装包和便携版 ZIP | 创建 Git Tag（如 v0.1.0） |
| 自动发布 | 创建 GitHub Release 并上传构建产物 | 创建 Git Tag（如 v0.1.0） |

### 6.2 流水线配置文件（.github/workflows/ci.yml）

**配置文件结构**：

```yaml
name: CI/CD Pipeline

on:
  push:
    branches: [ main, develop ]
    tags: [ 'v*' ]
  pull_request:
    branches: [ main ]

jobs:
  build:
    name: 构建
    runs-on: windows-latest
    steps:
      - name: 检出代码
        uses: actions/checkout@v4
      
      - name: 安装 vcpkg
        run: .\vcpkg\bootstrap-vcpkg.bat
      
      - name: 配置 CMake
        run: cmake -B build -DCMAKE_BUILD_TYPE=Release
      
      - name: 编译 Release
        run: cmake --build build --config Release
      
      - name: 上传构建产物
        uses: actions/upload-artifact@v4
        with:
          name: release-build
          path: build/bin/Release/

  test:
    name: 测试
    needs: build
    runs-on: windows-latest
    steps:
      - name: 下载构建产物
        uses: actions/download-artifact@v4
        with:
          name: release-build
      
      - name: 运行单元测试
        run: .\test_all.exe --gtest_output=json:test_results.json
      
      - name: 上传测试结果
        uses: actions/upload-artifact@v4
        with:
          name: test-results
          path: test_results.json

  package:
    name: 打包
    needs: [build, test]
    if: startsWith(github.ref, 'refs/tags/v')
    runs-on: windows-latest
    steps:
      - name: 下载构建产物
        uses: actions/download-artifact@v4
        with:
          name: release-build
      
      - name: 创建安装包
        run: |
          iscc setup.iss
          Compress-Archive -Path bin\Release\* -DestinationPath ai-thinapp-${{ github.ref_name }}-portable.zip
      
      - name: 上传安装包
        uses: actions/upload-artifact@v4
        with:
          name: installer
          path: |
            ai-thinapp-${{ github.ref_name }}-setup.exe
            ai-thinapp-${{ github.ref_name }}-portable.zip

  release:
    name: 发布
    needs: package
    if: startsWith(github.ref, 'refs/tags/v')
    runs-on: ubuntu-latest
    steps:
      - name: 下载安装包
        uses: actions/download-artifact@v4
        with:
          name: installer
      
      - name: 创建 GitHub Release
        uses: softprops/action-gh-release@v1
        with:
          files: |
            ai-thinapp-${{ github.ref_name }}-setup.exe
            ai-thinapp-${{ github.ref_name }}-portable.zip
          body: |
            ## AI ThinApp Portable Launchpad ${{ github.ref_name }}
            
            ### 安装包
            - `ai-thinapp-${{ github.ref_name }}-setup.exe` - 安装版
            - `ai-thinapp-${{ github.ref_name }}-portable.zip` - 便携版
            
            ### 校验和
            见附件 `SHA256SUMS`
          draft: false
          prerelease: contains(github.ref, 'beta') || contains(github.ref, 'alpha')
```

### 6.3 如何使用 CI/CD 流水线

**开发流程**：

1. **日常开发**：Push 到 `develop` 分支 → 触发自动构建和测试
2. **合并到主分支**：Pull Request 合并到 `main` → 触发自动构建和测试
3. **发布版本**：
   - 创建并推送 Git Tag：`git tag v0.1.0 && git push origin v0.1.0`
   - 自动触发打包和发布流程
   - GitHub Actions 自动创建 Release 并上传安装包

**查看流水线执行状态**：

1. 访问 [GitHub 仓库的 Actions 页面](https://github.com/anmingwei/ai-thinApp-portable-launchpad-platform/actions)
2. 查看各个 Job 的执行状态和日志
3. 如果失败，查看日志排查问题

### 6.4 本地测试 CI/CD 配置

**安装 GitHub CLI（可选）**：

```powershell
# 安装 GitHub CLI
winget install --interactive --locale en-US GitHub.cli

# 验证安装
gh --version

# 登录 GitHub
gh auth login
```

**本地运行 GitHub Actions（使用 act）**：

```powershell
# 安装 act（需要 Docker）
choco install act-cli -y

# 运行整个流水线
act

# 运行特定 Job
act -j build
```

---

## 第 7 章：代码签名证书申请

### 7.1 为什么需要代码签名证书？

代码签名证书可以：

| 作用 | 说明 |
|------|------|
| 验证发布者身份 | 用户可以看到"已验证的发布者"，而不是"未知发布者" |
| 保证文件完整性 | 文件被篡改后，签名会失效 |
| 通过 SmartScreen 过滤 | 有证书签名的程序不会被 Smarts 拦截 |
| 提升用户信任 | 用户更愿意下载和运行已签名的程序 |

**EV 代码签名证书 vs 标准代码签名证书**：

| 对比项 | 标准 OV/IV | EV（扩展验证） |
|--------|-------------|----------------|
| SmartScreen 信誉 | 需要时间积累 | 立即获得信誉 |
| 证书载体 | 文件（PFX） | 硬件令牌（USB）或云签名 |
| 验证严格度 | 中等 | 非常严格 |
| 价格 | 较低（$200-400/年） | 较高（$300-800/年） |
| 适合场景 | 个人开发者、小项目 | 企业、商业软件 |

**推荐**：对于 AI ThinApp Portable Launchpad 这样的系统级工具，建议使用 **EV 代码签名证书**，以避免 SmartScreen 拦截影响用户体验。

### 7.2 选择证书颁发机构 (CA)

**主流 CA 对比**：

| CA | EV 证书价格（约） | 发货速度 | 硬件令牌 | 推荐指数 |
|----|-------------------|----------|----------|----------|
| [Sectigo (原 Comodo)](https://sectigo.com) | $300-400/年 | 3-5 工作日 | USB 令牌或云签名 | ⭐⭐⭐⭐ |
| [DigiCert](https://digicert.com) | $600-800/年 | 1-3 工作日 | USB 令牌 | ⭐⭐⭐⭐⭐ |
| [GlobalSign](https://globalsign.com) | $400-600/年 | 3-7 工作日 | USB 令牌 | ⭐⭐⭐⭐ |

**建议**：
- **预算有限**：选择 Sectigo（性价比高）
- **追求品牌和信誉**：选择 DigiCert（最严格的验证，最受信任）
- **欧洲用户**：选择 GlobalSign（在欧洲信誉更好）

### 7.3 准备申请材料

**个人开发者**：

| 材料 | 说明 |
|------|------|
| 身份证或护照 | 扫描件（清晰、完整） |
| 地址证明 | 水电费账单、银行对账单（3 个月内） |
| 电话号码 | 用于电话验证 |
| 邮箱 | 用于接收验证邮件 |

**企业申请**：

| 材料 | 说明 |
|------|------|
| 企业营业执照 | 扫描件（清晰、完整） |
| 法人身份证 | 扫描件 |
| 企业地址证明 | 水电费账单、银行对账单（3 个月内） |
| 企业电话 | 用于电话验证（需能接通） |
| 授权书 | 如果申请人不是法人，需要提供授权书 |
| 企业邮箱 | 用于接收验证邮件（建议用企业域名邮箱） |

### 7.4 购买和验证流程

**以 Sectigo EV 代码签名证书为例**：

**第一步：购买证书**

1. 访问 [Sectigo 官网](https://sectigo.com) 或授权代理商（如 [SSLs.com](https://www.ssl.com)）
2. 选择"EV Code Signing Certificate"
3. 填写企业/个人信息
4. 支付费用（信用卡或 PayPal）

**第二步：验证流程**

1. **邮箱验证**：CA 发送验证邮件到企业邮箱或个人信息邮箱
2. **电话验证**：CA 会拨打企业电话或个人电话进行语音验证
3. **地址验证**：CA 验证地址证明文件的真实性
4. **第三方数据库验证**：CA 会查询 Dun & Bradstreet (D&B) 等第三方数据库验证企业信息

**第三步：接收硬件令牌**

1. 验证通过后，CA 会邮寄 USB 硬件令牌到企业地址
2. 收到令牌后，按照说明书安装驱动和软件
3. 激活令牌（设置 PIN 码）

**验证时间**：

| 验证步骤 | 预计时间 |
|----------|----------|
| 邮箱验证 | 1-2 工作日 |
| 电话验证 | 1-2 工作日 |
| 地址验证 | 2-3 工作日 |
| 第三方数据库验证 | 3-5 工作日 |
| 邮寄硬件令牌 | 3-7 工作日（取决于快递） |
| **总计** | **10-20 工作日** |

### 7.5 签名工具和流程

**安装签名工具**：

```powershell
# 安装 SDK（包含 signtool.exe）
# 下载 Windows SDK：https://developer.microsoft.com/en-us/windows/downloads/windows-sdk/
# 安装时选择"Windows Signing Tools"

# 验证安装
Get-Command signtool.exe -ErrorAction SilentlyContinue
```

**签名命令（使用硬件令牌）**：

```powershell
# 基本签名命令
signtool.exe sign /tr http://timestamp.sectigo.com /td sha256 /fd sha256 /a "D:\build\ai-thinapp.exe"

# 参数说明：
#   /tr <URL>        - 时间戳服务器地址（推荐 Sectigo 或 DigiCert 的时间戳服务器）
#   /td sha256       - 时间戳摘要算法
#   /fd sha256       - 文件摘要算法
#   /a               - 自动选择最佳证书
#   "路径"           - 要签名的文件路径

# 签名多个文件
Get-ChildItem "D:\build\*.exe" -Recurse | ForEach-Object {
    signtool.exe sign /tr http://timestamp.sectigo.com /td sha256 /fd sha256 /a $_.FullName
}
```

**验证签名**：

```powershell
# 验证文件签名
Get-AuthenticodeSignature "D:\build\ai-thinapp.exe" | Select-Object Status, SignerCertificate

# 查看签名详细信息
signtool.exe verify /pa /v "D:\build\ai-thinapp.exe"
```

### 7.6 云签名（推荐）

**什么是云签名？**

云签名（Cloud Signing）不需要硬件令牌，而是通过 API 调用 CA 的云端签名服务完成签名。

**优势**：

| 优势 | 说明 |
|------|------|
| 无需硬件令牌 | 不用担心令牌丢失或损坏 |
| 适合 CI/CD | 可以在 GitHub Actions 中自动签名 |
| 更高的安全性 | 私钥在云端 HSM（硬件安全模块）中，不会泄露 |

**支持的 CA**：

- DigiCert：提供 [DigiCert ONE](https://one.digicert.com) 云签名平台
- Sectigo：提供 [Sectigo Certificate Manager](https://sectigo.com/products/scm) 云签名平台

**GitHub Actions 中自动签名（使用 DigiCert ONE）**：

```yaml
jobs:
  sign:
    name: 签名
    needs: package
    runs-on: windows-latest
    steps:
      - name: 下载安装包
        uses: actions/download-artifact@v4
        with:
          name: installer
      
      - name: 安装 DigiCert ONE Client
        run: |
          Invoke-WebRequest -Uri "https://one.digicert.com/client/setup.exe" -OutFile setup.exe
          .\setup.exe /S
      
      - name: 签名 exe 文件
        env:
          DIGICERT_API_KEY: ${{ secrets.DIGICERT_API_KEY }}
          DIGICERT_CERT_ID: ${{ secrets.DIGICERT_CERT_ID }}
        run: |
          digicert-one sign --api-key $env:DIGICERT_API_KEY --cert-id $env:DIGICERT_CERT_ID --input ai-thinapp-setup.exe --output ai-thinapp-setup-signed.exe
      
      - name: 上传签名后的安装包
        uses: actions/upload-artifact@v4
        with:
          name: signed-installer
          path: ai-thinapp-setup-signed.exe
```

### 7.7 时间戳服务器

**为什么需要时间戳？**

如果证书过期或被吊销，没有时间戳的签名会失效。有时间戳的签名，即使证书过期，只要签名时证书有效，签名就永远有效。

**主流时间戳服务器**：

| CA | 时间戳服务器地址 |
|----|-------------------|
| Sectigo | `http://timestamp.sectigo.com` |
| DigiCert | `http://timestamp.digicert.com` |
| GlobalSign | `http://timestamp.globalsign.com` |

**推荐**：使用与证书相同 CA 的时间戳服务器，以避免兼容性问题。

### 7.8 常见问题

**Q：证书申请失败怎么办？**

A：常见原因和解决方法：

| 原因 | 解决方法 |
|------|----------|
| 电话验证未接通 | 确保企业电话有人接听，语音提示选择"企业验证" |
| 地址证明不清晰 | 重新上传更清晰的文件，确保地址和姓名/企业名可见 |
| 第三方数据库无记录 | 企业需要先注册 Dun & Bradstreet (D&B) 号码 |
| 授权书格式不正确 | 使用 CA 提供的授权书模板 |

**Q：硬件令牌丢失怎么办？**

A：立即联系 CA 吊销证书，然后重新申请。硬件令牌通常不支持补办，需要重新购买和验证。

**Q：可以在多台电脑上签名吗？**

A：可以。EV 证书的硬件令牌可以在多台电脑上使用（需要安装驱动和软件）。云签名则更加灵活，只需 API Key 即可在任何地方签名。

---

## 附录

### A. 版本兼容性

| Launchpad 版本 | 最低 Windows 版本 | 最低 .NET 版本 | 最低 VC++ 版本 |
|----------------|-------------------|----------------|----------------|
| v0.1.0 (MVP) | Windows 10 22H2 | 不适用（Electron 内置） | VC++ 2022 (14.30+) |

### B. 安装包大小

| 文件 | 大小（约） |
|------|-----------|
| ai-thinapp-v0.1.0-setup.exe | ~150 MB（含 Electron 运行时） |
| 安装后磁盘占用 | ~200 MB |

### C. 文件清单

```
C:\Program Files\AI ThinApp\
├── ai-thinapp.exe              # 主程序
├── hook_engine.dll             # 沙箱引擎
├── hook_engine_x86.dll         # 32位沙箱引擎（用于32位应用）
├── resources/                  # Electron 资源
│   ├── app.asar               # Launchpad 前端代码
│   └── ...
├── locales/                    # 多语言文件
│   ├── zh_CN.pak              # 简体中文
│   └── en_US.pak              # 英文
└── ...
```

---

**最后更新**：2026-05-24
**维护者**：AI ThinApp Portable Launchpad Platform 开源社区


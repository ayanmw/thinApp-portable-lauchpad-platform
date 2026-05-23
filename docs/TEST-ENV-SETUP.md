# 测试环境搭建指南 (Test Environment Setup Guide)

> 本文档提供 AI ThinApp Portable Launchpad Platform 项目杀软兼容性测试的环境搭建步骤。
> 适用于测试工程师和开发人员。

---

## 1. 创建 Windows 虚拟机（VMware Workstation）

### 1.1 准备工作

**软件下载**：
- VMware Workstation Pro：https://www.vmware.com/products/workstation-pro.html
  - 商业软件，但有 30 天试用期
  - 替代方案：VMware Workstation Player（免费，功能受限）
  - 开源替代：VirtualBox（https://www.virtualbox.org/）

**Windows ISO 镜像下载**：
- Windows 10 22H2：https://www.microsoft.com/zh-cn/software-download/windows10
- Windows 11 23H2：https://www.microsoft.com/zh-cn/software-download/windows11

**硬件要求**：
- CPU：4 核以上（支持虚拟化技术 Intel VT-x / AMD-V）
- 内存：8GB 以上（分配给虚拟机 4GB）
- 硬盘：60GB 以上（虚拟机磁盘 40GB）

### 1.2 创建 Windows 10 22H2 虚拟机

**步骤**：

1. **打开 VMware Workstation**，点击 **"创建新的虚拟机"**

2. **选择安装来源**
   - 选择 **"安装程序光盘映像文件（iso）"**
   - 浏览并选择 Windows 10 ISO 文件
   - 点击 **"下一步"**

3. **选择客户机操作系统**
   - 选择 **"Microsoft Windows"**
   - 版本选择 **"Windows 10 x64"**
   - 点击 **"下一步"**

4. **命名虚拟机**
   - 虚拟机名称：`Win10-22H2-Defender`（根据杀软命名）
   - 位置：选择磁盘空间充足的目录（例如 `D:\VMs\`）
   - 点击 **"下一步"**

5. **配置磁盘**
   - 磁盘容量：40 GB（推荐）
   - 选择 **"将虚拟磁盘拆分成多个文件"**（便于移动）
   - 点击 **"下一步"**

6. **自定义硬件**
   - 点击 **"自定义硬件"**
   - 内存：4096 MB（4 GB）
   - 处理器：2 核（或更多）
   - 网络适配器：选择 **"桥接模式"**（便于测试网络连接）
   - 点击 **"关闭"**，然后 **"完成"**

7. **启动虚拟机，安装 Windows**
   - 按照提示完成 Windows 安装
   - 跳过产品密钥（可以选择"我没有产品密钥"）
   - 选择 **"Windows 10 专业版"**（推荐）
   - 同意许可条款
   - 选择 **"自定义：仅安装 Windows"**
   - 选择磁盘（虚拟机磁盘），下一步
   - 等待安装完成（10-20 分钟）

8. **完成初始设置**
   - 选择区域：**"中国"**
   - 键盘布局：**"微软拼音"**
   - 网络连接：可以跳过（避免强制登录 Microsoft 账号）
   - 选择账户类型：**"脱机账户"**
   - 输入用户名：`tester`
   - 设置密码（可选）
   - 关闭 Cortana、位置服务、诊断数据等（保护隐私）

### 1.3 创建 Windows 11 23H2 虚拟机

**步骤**：

与 Windows 10 类似，但需要注意：

1. **硬件要求更高**
   - 内存：8GB（分配给虚拟机 4-8GB）
   - 处理器：4 核（或更多）
   - 磁盘：60GB（虚拟机磁盘 40-60GB）

2. **TPM 2.0 要求**
   - VMware Workstation 16+ 支持虚拟 TPM 2.0
   - 创建虚拟机时，选择 **"加密"**，然后勾选 **"启用信任平台模块（TPM）"**

3. **安装步骤**
   - 其他步骤与 Windows 10 相同

### 1.4 优化虚拟机性能

**步骤**：

1. **安装 VMware Tools**
   - 启动虚拟机
   - 菜单栏：**"虚拟机"** → **"安装 VMware Tools"**
   - 打开文件资源管理器，双击 DVD 驱动器
   - 运行 `setup.exe`
   - 选择 **"典型"** 安装
   - 完成后重启虚拟机

2. **关闭不必要的服务**
   - 按 `Win + R`，输入 `services.msc`
   - 关闭以下服务（右键 → 属性 → 启动类型：禁用）：
     - Print Spooler（打印服务）
     - Xbox Accessory Management（Xbox 配件管理）
     - Downloaded Maps Manager（下载地图管理）

3. **关闭 Windows 更新（可选，测试环境）**
   - 按 `Win + R`，输入 `gpedit.msc`
   - 导航到：**计算机配置 → 管理模板 → Windows 组件 → Windows 更新**
   - 双击 **"配置自动更新"**
   - 选择 **"已禁用"**
   - 点击 **"确定"**

4. **创建干净快照**
   - 关闭虚拟机
   - 在 VMware 中，右键虚拟机 → **"快照"** → **"拍摄快照"**
   - 名称：`Clean Install - No Antivirus`
   - 描述：`Windows 干净安装，未安装任何杀软`
   - 点击 **"拍摄快照"**

---

## 2. 安装和配置杀软

### 2.1 Windows Defender（默认启用）

Windows Defender 是 Windows 10/11 内置杀软，默认启用。

**验证 Defender 已启用**：

1. 打开 **Windows 安全中心**（任务栏右下角图标，或搜索"Windows 安全中心"）
2. 点击 **"病毒和威胁防护"**
3. 确认 **"实时防护"** 已开启

**关闭 Defender（测试其他杀软时）**：

1. 打开 **Windows 安全中心**
2. 点击 **"病毒和威胁防护"** → **"管理设置"**
3. 关闭 **"实时防护"**
4. 关闭 **"云提供的保护"**
5. 关闭 **"自动提交样本"**

**注意**：安装第三方杀软（如火绒、360）时，Defender 会自动关闭（实时防护被禁用）。

### 2.2 火绒安全

**下载地址**：https://www.huorong.cn/

**安装步骤**：

1. 访问上述地址，点击 **"立即下载"**
2. 下载完成后，运行 `sysdiag.exe`
3. 选择安装语言：**"简体中文"**
4. 点击 **"快速安装"**（或"自定义安装"选择安装目录）
5. 等待安装完成
6. 安装完成后，火绒会自动更新病毒库

**配置火绒**：

1. 打开 **火绒安全**
2. 点击右上角 **"设置"**（齿轮图标）
3. 配置以下选项：
   - **病毒防护**：
     - 实时防护：**开启**
     - 文件监控：**开启**
     - 下载防御：**开启**
   - **系统防护**：
     - 系统加固：**开启**（可选，测试时可以关闭）
     - 漏洞修复：**关闭**（测试环境不需要）
   - **通用设置**：
     - 消息通知：**开启**（便于观察误报）
     - 声音提醒：**关闭**（避免干扰）

**创建快照**：

1. 关闭虚拟机
2. 右键虚拟机 → **"快照"** → **"拍摄快照"**
3. 名称：`Win10-22H2-Huorong`
4. 描述：`Windows 10 22H2 + 火绒安全（最新版）`
5. 点击 **"拍摄快照"**

### 2.3 360 安全卫士

**下载地址**：https://www.360.cn/

**安装步骤**：

1. 访问上述地址，点击 **"下载"**
2. 下载完成后，运行 `setup.exe`
3. 选择 **"自定义安装"**
4. 取消勾选 **"设为默认浏览器"**、**"安装360安全浏览器"** 等捆绑软件
5. 选择安装目录（例如 `C:\Program Files\360\`）
6. 点击 **"安装"**
7. 等待安装完成
8. 安装完成后，360 会自动更新病毒库

**配置 360**：

1. 打开 **360 安全卫士**
2. 点击右上角 **"设置"**（齿轮图标）
3. 配置以下选项：
   - **病毒查杀**：
     - 实时防护：**开启**
     - 下载保护：**开启**
   - **优化加速**：
     - 关闭不必要的功能（测试环境不需要）
   - **弹窗设置**：
     - 关闭所有弹窗通知（避免干扰测试）

**注意**：360 安全卫士会安装捆绑软件（如 360 浏览器、360 压缩），安装时务必取消勾选。

**创建快照**：

1. 关闭虚拟机
2. 右键虚拟机 → **"快照"** → **"拍摄快照"**
3. 名称：`Win10-22H2-360`
4. 描述：`Windows 10 22H2 + 360 安全卫士（最新版）`
5. 点击 **"拍摄快照"**

---

## 3. 创建虚拟机快照（测试前还原）

快照可以让你快速还原到干净状态，避免测试污染。

### 3.1 快照策略

**推荐快照点**：

1. **干净安装（无杀软）**
   - 名称：`Clean Install - No Antivirus`
   - 用途：安装不同杀软前还原

2. **Defender 启用**
   - 名称：`Win10-22H2-Defender`
   - 用途：测试 Windows Defender

3. **火绒安全**
   - 名称：`Win10-22H2-Huorong`
   - 用途：测试火绒安全

4. **360 安全卫士**
   - 名称：`Win10-22H2-360`
   - 用途：测试 360 安全卫士

5. **所有杀软安装（不推荐）**
   - 多个杀软可能互相干扰，不建议安装在同一虚拟机中

### 3.2 创建快照步骤

1. **确保虚拟机已关闭**（推荐关机状态创建快照，避免文件锁）
2. 在 VMware 中，右键虚拟机 → **"快照"** → **"拍摄快照"**
3. 填写快照名称（简洁明了）
4. 填写快照描述（记录当前状态，便于后续识别）
5. 点击 **"拍摄快照"**
6. 等待快照创建完成（1-5 分钟，取决于虚拟机磁盘大小）

### 3.3 还原快照步骤

1. **确保虚拟机已关闭**
2. 在 VMware 中，右键虚拟机 → **"快照"** → **"快照管理器"**
3. 选择要还原的快照
4. 点击 **"转到"**
5. 确认还原（会丢失当前状态）
6. 等待还原完成（1-2 分钟）

### 3.4 自动还原脚本（可选）

**使用 VMware CLI（vmrun）自动还原快照**：

```powershell
# restore_snapshot.ps1
# 自动还原虚拟机快照

$vmxPath = "D:\VMs\Win10-22H2-Defender\Windows 10 x64.vmx"
$snapshotName = "Win10-22H2-Defender"

# VMware Workstation 安装路径
$vmrunPath = "C:\Program Files (x86)\VMware\VMware Workstation\vmrun.exe"

# 还原快照
& $vmrunPath -T ws revertToSnapshot "$vmxPath" "$snapshotName"

if ($LASTEXITCODE -eq 0) {
    Write-Host "✓ Snapshot restored: $snapshotName" -ForegroundColor Green
} else {
    Write-Host "✗ Failed to restore snapshot" -ForegroundColor Red
}
```

---

## 4. 收集杀软日志（用于分析误报原因）

杀软日志可以帮助你了解为什么文件被误报、误报类型、建议操作等。

### 4.1 Windows Defender 日志收集

#### 方法一：使用 Windows 安全中心

1. 打开 **Windows 安全中心**
2. 点击 **"病毒和威胁防护"**
3. 点击 **"防护历史记录"**
4. 查看最近的检测记录：
   - 检测时间
   - 威胁名称
   - 操作（隔离/允许/阻止）
   - 详细信息（点击可查看）

#### 方法二：使用 PowerShell

**查看防护历史记录**：

```powershell
# 查看最近的 Defender 检测记录
Get-MpThreatDetection | Select-Object -Last 10 | Format-Table -AutoSize

# 查看详细信息
Get-MpThreat | Select-Object -Last 10 | Format-List
```

**导出 Defender 日志**：

```powershell
# 导出 Defender 操作日志（事件查看器）
Get-WinEvent -LogName "Microsoft-Windows-Windows Defender/Operational" | 
    Where-Object { $_.TimeCreated -gt (Get-Date).AddDays(-7) } |
    Export-Csv -Path "Defender_Log.csv" -Encoding UTF8
```

#### 方法三：使用事件查看器

1. 按 `Win + R`，输入 `eventvwr.msc`
2. 导航到：**应用程序和服务日志 → Microsoft → Windows → Windows Defender → Operational**
3. 查看最近的事件（误报通常会记录在此）
4. 右键事件 → **"将事件另存为"**，保存为 `.evtx` 文件

### 4.2 火绒安全日志收集

#### 方法一：使用火绒界面

1. 打开 **火绒安全**
2. 点击 **"安全日志"**（或"防护日志"）
3. 查看最近的防护记录：
   - 时间
   - 防护类型（病毒防护/系统防护/网络防护）
   - 威胁名称
   - 操作（允许/阻止/隔离）
4. 点击 **"导出日志"**，保存为 `.log` 或 `.txt` 文件

#### 方法二：查看日志文件

火绒日志文件位于：

```
C:\ProgramData\Huorong\Sysdiag\Logs\
```

**主要日志文件**：
- `sysdiag.log`：主日志文件
- `virus.log`：病毒防护日志
- `defense.log`：系统防护日志

**使用 PowerShell 查看日志**：

```powershell
# 查看火绒主日志（最后 50 行）
Get-Content "C:\ProgramData\Huorong\Sysdiag\Logs\sysdiag.log" -Tail 50

# 搜索包含 "hook_engine" 的日志行
Select-String -Path "C:\ProgramData\Huorong\Sysdiag\Logs\virus.log" -Pattern "hook_engine"
```

### 4.3 360 安全卫士日志收集

#### 方法一：使用 360 界面

1. 打开 **360 安全卫士**
2. 点击 **"日志"**（或"防护日志"）
3. 查看最近的防护记录
4. 点击 **"导出"**，保存为 `.txt` 文件

#### 方法二：查看日志文件

360 日志文件位于：

```
C:\ProgramData\360safe\Log\
```

**主要日志文件**：
- `360Log.log`：主日志文件
- `VirusLog.log`：病毒防护日志

**使用 PowerShell 查看日志**：

```powershell
# 查看 360 主日志（最后 50 行）
Get-Content "C:\ProgramData\360safe\Log\360Log.log" -Tail 50
```

### 4.4 日志分析要点

**分析误报日志时，关注以下信息**：

1. **时间**：误报发生的时间
2. **文件路径**：哪个文件被误报
3. **威胁名称**：杀软给的威胁名称（例如 "Trojan:Win32/Wacatac.B!ml"）
4. **行动**：杀软建议的操作（隔离/允许/阻止）
5. **原因**：为什么被误报（特征码匹配？行为可疑？）

**示例日志分析**：

```
[2026-05-23 14:30:15] Virus detected:
  File: D:\AI-ThinApp\hook_engine.dll
  Threat: Trojan:Win32/Wacatac.B!ml
  Action: Quarantine
  Reason: Heuristic analysis (suspicious behavior)
```

**解读**：
- 文件：`hook_engine.dll`
- 威胁名称：`Trojan:Win32/Wacatac.B!ml`（Defender 误报为 Trojan）
- 行动：隔离（用户需要手动恢复）
- 原因：启发式分析（行为可疑）

**应对措施**：
1. 将文件添加到排除项
2. 提交误报申诉
3. 代码签名后重新测试

---

## 5. 测试环境检查清单

在开始测试前，使用以下清单确保环境正确。

### 5.1 虚拟机检查

- [ ] 虚拟机已创建（Windows 10/11）
- [ ] VMware Tools 已安装
- [ ] 干净快照已创建（无杀软）
- [ ] 网络连接正常（可以访问 internet）

### 5.2 杀软检查

- [ ] 杀软已安装（Defender / 火绒 / 360）
- [ ] 病毒库已更新到最新版本
- [ ] 实时防护已启用
- [ ] 杀软快照已创建

### 5.3 测试文件检查

- [ ] `hook_engine.dll` 已编译（未签名）
- [ ] `test_hook.exe` 已编译（未签名）
- [ ] `test_process.exe` 已编译（未签名）
- [ ] 上述文件的 OV 代码签名版本已准备（若证书已到手）

### 5.4 日志收集检查

- [ ] 知道如何查看杀软日志（Defender / 火绒 / 360）
- [ ] 知道如何导出日志（用于后续分析）
- [ ] 日志分析要点已理解

---

## 6. 修订历史

| 版本 | 日期 | 作者 | 变更说明 |
|------|------|------|----------|
| 0.1 | 2026-05-23 | QA | 初版，基于任务要求输出 |

---

## 附录：VMware Workstation 常用快捷键

| 快捷键 | 功能 |
|--------|------|
| `Ctrl + G` | 聚焦到虚拟机 |
| `Ctrl + Alt` | 释放鼠标（从虚拟机返回主机） |
| `Ctrl + Alt + Enter` | 全屏模式 |
| `Ctrl + Alt + Insert` | 发送 `Ctrl + Alt + Del` 到虚拟机 |
| `Ctrl + Shift + Tab` | 切换虚拟机标签页 |

---

**注意**：本文档是动态文档，随着测试进展会不断更新。若遇到环境搭建问题，请记录到 `docs/WEEK3-BLOCKERS.md`。

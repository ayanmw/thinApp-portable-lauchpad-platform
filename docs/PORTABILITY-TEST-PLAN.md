# 跨目录便携性验证方案

## 1. 验证目标

跨目录便携性验证（V8）是 POC 阶段的关键验证项。本方案旨在验证以下目标：

1. **路径无关性**：验证应用目录可复制到任意路径（同一台电脑或不同电脑），应用仍能正常运行
2. **重定向正确性**：验证复制后，所有文件/注册表操作仍然正确重定向到新目录
3. **功能完整性**：验证应用功能正常（注册表读取、文件保存、插件加载等）
4. **用户数据便携性**：验证用户配置、插件等数据可随目录迁移

---

## 2. 测试环境

| 环境 | 说明 | 用途 | 准备状态 |
|------|------|------|----------|
| 本机（Win10 22H2） | 初始安装环境 | 捕获应用安装、基准测试 | ✅ 可用 |
| 本机不同路径 | 同一台电脑，不同目录 | 验证路径无关性 | ✅ 可用 |
| 虚拟机 A（Win10 22H2） | 干净系统，无应用依赖 | 验证跨机器便携性 | ⚠️ 需准备 |
| 虚拟机 B（Win11 23H2） | 不同 Windows 版本 | 验证系统版本兼容性 | ⚠️ 需准备 |
| U 盘 / 移动硬盘 | 外部存储设备 | 验证移动场景 | ✅ 可用 |

### 2.1 环境准备详细要求

#### 本机（Win10 22H2）
- 已安装 AI ThinApp Portable Launchpad Platform 开发环境
- 已安装测试应用（Notepad++ / 7-Zip / Firefox Portable）
- 已配置 Hook 引擎和 VFS/VReg 模块
- 用于：初始捕获、基准测试、路径无关性验证

#### 本机不同路径
- 同一台电脑上的不同盘符或目录
- 例如：从 `D:\MyApps\` 复制到 `E:\PortableApps\`
- 用于：快速验证路径无关性（无需跨机器）

#### 虚拟机 A（Win10 22H2）
- 干净 Windows 10 22H2 系统
- 未安装测试应用（Notepad++ 等）
- 安装 AI ThinApp Portable Launchpad Platform 运行时（仅运行时，无开发环境）
- 用于：验证跨机器便携性（应用未安装时能否运行）

#### 虚拟机 B（Win11 23H2）
- 干净 Windows 11 23H2 系统
- 用于：验证系统版本兼容性（Win10 捕获的应用能否在 Win11 运行）

#### U 盘 / 移动硬盘
- 外部存储设备（USB 3.0 推荐）
- 用于：验证移动场景（从一台电脑带到另一台电脑）

---

## 3. 测试步骤

### 步骤 1：捕获应用（在"本机"环境）

**目标**：使用 AppCapture 工具捕获 Notepad++ 安装，生成 .vapp 包

**操作步骤**：

1. 启动 AppCapture，开始捕获
2. 运行 Notepad++ 安装程序（从官网下载最新版）
3. 安装到默认路径（`C:\Program Files\Notepad++`）
4. 完成安装后，进行基础配置（主题、语言、插件）
5. 停止捕获，生成 .vapp 包（`Notepad++.vapp`）

**预期结果**：
- 成功生成 .vapp 包
- .vapp 包包含：
  - 文件系统差异（`VFS\C\Program Files\Notepad++\`）
  - 注册表差异（`VFS\registry.hive`）
  - 元数据（`manifest.json`）

**实际结果**：待填写

**状态**：待测试

---

### 步骤 2：解包到应用目录

**目标**：将 .vapp 包解压到应用目录，确认目录结构正确

**操作步骤**：

1. 将 `Notepad++.vapp` 解压到 `D:\MyApps\Notepad++\`
2. 确认目录结构：

```
D:\MyApps\Notepad++\
├── Notepad++.exe           # Launchpad 启动器（包装后的 exe）
├── VFS\                    # 虚拟文件系统
│   ├── C\Program Files\Notepad++\   # 应用安装文件
│   │   ├── notepad++.exe
│   │   ├── plugins\
│   │   ├── langs.xml
│   │   └── ...
│   ├── AppData\Roaming\Notepad++\   # 用户配置
│   │   ├── config.xml
│   │   ├── styles.xml
│   │   └── ...
│   ├── Temp\               # 临时文件
│   └── registry.hive       # 虚拟注册表 hive
├── sandbox.json             # 沙箱配置（拦截规则、Hook 开关）
└── manifest.json            # 应用元数据（名称、版本、作者）
```

3. 验证 `sandbox.json` 配置正确：
   - 文件重定向：启用
   - 注册表重定向：启用
   - 子进程继承：启用

**预期结果**：
- 目录结构完整
- `sandbox.json` 配置正确
- `manifest.json` 包含应用信息

**实际结果**：待填写

**状态**：待测试

---

### 步骤 3：在本机不同路径运行（验证路径无关性）

**目标**：验证应用目录复制到不同路径后，仍能正常运行（路径无关性）

**操作步骤**：

1. 复制 `D:\MyApps\Notepad++\` 到 `E:\PortableApps\Notepad++\`
2. 双击 `E:\PortableApps\Notepad++\Notepad++.exe`
3. 进行以下验证：

#### 验证 3.1：应用正常启动
- **操作**：双击 exe，观察应用是否启动
- **预期**：应用正常启动，无错误对话框

#### 验证 3.2：文件重定向到新路径
- **操作**：
  1. 在 Notepad++ 中，点击"文件" → "打开"
  2. 观察默认路径是否为 `E:\PortableApps\Notepad++\VFS\...`
  3. 打开一个文件，编辑后保存
  4. 检查文件是否保存在 `E:\PortableApps\Notepad++\VFS\...`
- **预期**：
  - 打开文件对话框默认路径指向新目录的 VFS
  - 保存的文件出现在新目录的 VFS 中
  - 真实文件系统未受影响

#### 验证 3.3：注册表重定向到新路径
- **操作**：
  1. 在 Notepad++ 中，修改设置（主题、字体）
  2. 关闭应用
  3. 使用注册表工具（如 RegEdit）检查 `E:\PortableApps\Notepad++\VFS\registry.hive`
- **预期**：
  - 设置存储在虚拟 hive 中
  - 真实注册表未受影响

#### 验证 3.4：相对路径解析正确
- **操作**：
  1. 在 Notepad++ 中，打开插件管理器
  2. 检查插件路径是否为 `E:\PortableApps\Notepad++\VFS\C\Program Files\Notepad++\plugins\`
- **预期**：插件从新目录加载，功能正常

**预期结果**：
- 应用正常启动
- 所有文件操作重定向到新路径
- 注册表操作重定向到新路径
- 相对路径解析正确

**实际结果**：待填写

**状态**：待测试

---

### 步骤 4：在虚拟机 A 运行（验证跨机器便携性）

**目标**：验证应用目录复制到另一台电脑（干净系统）后，仍能正常运行

**前置条件**：
- 虚拟机 A（Win10 22H2）已启动
- 虚拟机 A **未安装** Notepad++
- 虚拟机 A 已安装 AI ThinApp Portable Launchpad Platform 运行时

**操作步骤**：

1. 将 `D:\MyApps\Notepad++\` 复制到虚拟机 A 的 `C:\PortableApps\Notepad++\`
   - 使用共享文件夹、网络共享或 U 盘复制
2. 双击 `C:\PortableApps\Notepad++\Notepad++.exe`
3. 进行以下验证：

#### 验证 4.1：应用正常启动（未安装状态）
- **操作**：双击 exe，观察应用是否启动
- **预期**：应用正常启动（虚拟机 A 未安装 Notepad++）

#### 验证 4.2：所有功能正常
- **操作**：
  1. 打开文件（Ctrl+O）
  2. 编辑文件，保存（Ctrl+S）
  3. 切换主题（设置 → 主题）
  4. 加载插件（插件 → 插件管理）
- **预期**：所有功能正常，无错误

#### 验证 4.3：运行时文件生成在 VFS
- **操作**：
  1. 关闭应用
  2. 检查 `C:\PortableApps\Notepad++\VFS\` 目录
- **预期**：运行时生成的文件（如缓存、日志）出现在 VFS 中

#### 验证 4.4：真实系统未受污染
- **操作**：
  1. 检查 `C:\Program Files\` 是否有 Notepad++ 目录
  2. 检查注册表 `HKLM\Software\Notepad++` 是否存在
- **预期**：真实系统无 Notepad++ 痕迹

**预期结果**：
- 应用正常启动（干净系统，未安装）
- 所有功能正常
- 运行时文件生成在 VFS
- 真实系统未受污染

**实际结果**：待填写

**状态**：待测试

---

### 步骤 5：在虚拟机 B 运行（验证系统版本兼容性）

**目标**：验证 Win10 捕获的应用能否在 Win11 系统运行

**前置条件**：
- 虚拟机 B（Win11 23H2）已启动
- 虚拟机 B 已安装 AI ThinApp Portable Launchpad Platform 运行时

**操作步骤**：

1. 将 `D:\MyApps\Notepad++\` 复制到虚拟机 B 的 `C:\PortableApps\Notepad++\`
2. 双击 `C:\PortableApps\Notepad++\Notepad++.exe`
3. 重复步骤 4 的所有验证

**预期结果**：
- 应用正常启动
- 所有功能正常（Win11 兼容性）

**实际结果**：待填写

**状态**：待测试

---

### 步骤 6：在 U 盘运行（验证移动场景）

**目标**：验证应用目录在外部存储设备上运行的正常性

**操作步骤**：

1. 将 `D:\MyApps\Notepad++\` 复制到 U 盘 `F:\PortableApps\Notepad++\`
2. 在另一台电脑（非虚拟机，真实物理机）上，双击 `F:\PortableApps\Notepad++\Notepad++.exe`
3. 进行以下验证：

#### 验证 6.1：应用正常启动（外部存储）
- **操作**：双击 exe，观察应用是否启动
- **预期**：应用正常启动，无权限错误

#### 验证 6.2：文件读写速度正常
- **操作**：
  1. 打开一个大文件（10MB+）
  2. 编辑后保存
- **预期**：读写速度可接受（无明显卡顿）

#### 验证 6.3：安全删除后无残留
- **操作**：
  1. 关闭应用
  2. 安全删除 U 盘
  3. 检查原电脑是否有残留文件
- **预期**：无残留（所有文件在 U 盘的应用目录中）

**预期结果**：
- 应用正常启动
- 文件读写速度正常
- 安全删除后无残留

**实际结果**：待填写

**状态**：待测试

---

### 步骤 7：修改应用配置（验证用户数据便携性）

**目标**：验证用户配置、插件等数据可随目录迁移

**操作步骤**：

1. 在"本机"启动 Notepad++（从 `D:\MyApps\Notepad++\`）
2. 修改设置：
   - 主题：从默认改为"深色主题"
   - 字体：改为"Consolas, 14pt"
   - 语言：改为"中文（简体）"
   - 安装插件：NppExport、Mime Tools
3. 关闭应用
4. 复制应用目录到虚拟机 A（`C:\PortableApps\Notepad++\`）
5. 在虚拟机 A 启动 Notepad++
6. 验证设置已迁移：
   - 主题是否为"深色主题"
   - 字体是否为"Consolas, 14pt"
   - 语言是否为"中文（简体）"
   - 插件是否可用

**预期结果**：
- 用户配置已迁移（主题、字体、语言）
- 插件可用（随目录复制）

**实际结果**：待填写

**状态**：待测试

---

## 4. 测试用例

| ID | 测试场景 | 操作步骤 | 预期结果 | 实际结果 | 状态 |
|----|----------|----------|----------|----------|------|
| PT-01 | 本机不同路径 | 复制目录到不同盘符，双击 exe | 应用启动，文件重定向到新路径 | 待填写 | 待测试 |
| PT-02 | 跨机器（Win10 → Win10） | 复制到虚拟机 A，双击 exe | 应用启动，功能正常 | 待填写 | 待测试 |
| PT-03 | 跨机器（Win10 → Win11） | 复制到虚拟机 B，双击 exe | 应用启动，功能正常 | 待填写 | 待测试 |
| PT-04 | U 盘移动场景 | 复制到 U 盘，在另一台电脑运行 | 应用启动，功能正常 | 待填写 | 待测试 |
| PT-05 | 用户配置迁移 | 修改配置 → 复制目录 → 在另一台电脑验证 | 配置已迁移 | 待填写 | 待测试 |
| PT-06 | 插件/扩展迁移 | 安装插件 → 复制目录 → 在另一台电脑验证 | 插件可用 | 待填写 | 待测试 |
| PT-07 | 大文件处理 | 打开 100MB 文件，保存，复制目录 | 文件正确保存在 VFS | 待填写 | 待测试 |
| PT-08 | 注册表重定向 | 读取/写入注册表，复制目录后验证 | 注册表数据来自虚拟 hive | 待填写 | 待测试 |

### 4.1 测试用例详细说明

#### PT-01：本机不同路径
- **验证点**：
  - 应用启动正常
  - 文件重定向到新路径（`E:\PortableApps\Notepad++\VFS\...`）
  - 注册表重定向到新路径（`E:\PortableApps\Notepad++\VFS\registry.hive`）
  - 相对路径解析正确（插件路径、配置文件路径）
- **测试方法**：手动测试 + Process Monitor 监控
- **通过标准**：所有验证点通过

#### PT-02：跨机器（Win10 → Win10）
- **验证点**：
  - 应用启动正常（干净系统，未安装）
  - 所有功能正常（打开/保存文件、插件、设置）
  - 运行时文件生成在 VFS
  - 真实系统未受污染
- **测试方法**：手动测试 + 文件系统/注册表检查
- **通过标准**：所有验证点通过

#### PT-03：跨机器（Win10 → Win11）
- **验证点**：
  - 同 PT-02
  - 额外验证：Win11 特定路径（`C:\Program Files\WindowsApps\`）不影响应用运行
- **测试方法**：手动测试
- **通过标准**：所有验证点通过

#### PT-04：U 盘移动场景
- **验证点**：
  - 应用启动正常（外部存储）
  - 文件读写速度正常（无明显卡顿）
  - 安全删除后无残留
- **测试方法**：手动测试 + 性能监控
- **通过标准**：所有验证点通过

#### PT-05：用户配置迁移
- **验证点**：
  - 主题、字体、语言等配置已迁移
  - 配置文件存储在 `VFS\AppData\Roaming\Notepad++\`
- **测试方法**：手动测试 + 文件对比
- **通过标准**：所有验证点通过

#### PT-06：插件/扩展迁移
- **验证点**：
  - 插件文件存储在 `VFS\C\Program Files\Notepad++\plugins\`
  - 插件在目标机器上可用
- **测试方法**：手动测试 + 文件对比
- **通过标准**：所有验证点通过

#### PT-07：大文件处理
- **验证点**：
  - 打开 100MB 文件正常（无内存溢出）
  - 保存文件到 VFS 正常
  - 复制目录后，大文件仍在 VFS 中
- **测试方法**：手动测试 + 文件哈希校验
- **通过标准**：所有验证点通过

#### PT-08：注册表重定向
- **验证点**：
  - 注册表读取来自虚拟 hive
  - 注册表写入到虚拟 hive
  - 真实注册表未受影响
- **测试方法**：注册表监控工具 + 注册表对比
- **通过标准**：所有验证点通过

---

## 5. 验收标准

- [ ] **PT-01 通过**：应用在本机不同路径可正常运行
- [ ] **PT-02 通过**：应用在另一台 Win10 电脑可正常运行
- [ ] **PT-03 通过**：应用在 Win11 电脑可正常运行
- [ ] **PT-04 通过**：应用在 U 盘可正常运行
- [ ] **PT-05 通过**：用户配置可随目录迁移
- [ ] **PT-06 通过**：插件/扩展可随目录迁移
- [ ] **PT-07 通过**：大文件处理正常
- [ ] **PT-08 通过**：注册表重定向正确

### 5.1 通过标准定义

- **全部通过（8/8）**：✅ 应用满足便携性要求，可以进入 MVP 阶段
- **部分通过（6-7/8）**：⚠️ 应用基本满足便携性要求，但部分测试用例失败，需修复后进入 MVP 阶段
- **少数通过（<6/8）**：❌ 应用不满足便携性要求，需重新评估技术路线

### 5.2 优先级划分

| 优先级 | 测试用例 | 理由 |
|--------|----------|------|
| P0 | PT-01 | 路径无关性是便携性的基础 |
| P0 | PT-02 | 跨机器便携性是核心需求 |
| P1 | PT-03 | 系统版本兼容性影响适用范围 |
| P1 | PT-08 | 注册表重定向是核心技术点 |
| P2 | PT-04 | 移动场景是常见用法 |
| P2 | PT-05 | 用户配置迁移影响用户体验 |
| P2 | PT-06 | 插件迁移影响功能完整性 |
| P3 | PT-07 | 大文件处理是边界情况 |

---

## 6. 自动化脚本（可选）

### 6.1 脚本目标

编写 PowerShell 脚本 `tools\ci\test-portability.ps1`，实现以下目标：

1. **自动化执行 PT-01 ~ PT-08**（部分需要手动，如跨机器）
2. **输出测试报告**（HTML 格式，便于查看）
3. **集成到 CI/CD**（仅本机测试，跨机器测试需手动）

### 6.2 脚本功能设计

#### 功能 1：自动化测试 PT-01（本机不同路径）
```powershell
# 复制应用目录到不同路径
Copy-Item -Path "D:\MyApps\Notepad++" -Destination "E:\PortableApps\Notepad++" -Recurse

# 启动应用
Start-Process "E:\PortableApps\Notepad++\Notepad++.exe"

# 等待应用启动
Start-Sleep -Seconds 5

# 检查进程是否运行
$process = Get-Process "Notepad++" -ErrorAction SilentlyContinue
if ($process) {
    Write-Host "PT-01: 应用启动成功" -ForegroundColor Green
} else {
    Write-Host "PT-01: 应用启动失败" -ForegroundColor Red
}

# 关闭应用
Stop-Process -Name "Notepad++"
```

#### 功能 2：文件重定向验证
```powershell
# 监控文件操作（使用 Process Monitor CLI）
# 检查文件是否重定向到 VFS
```

#### 功能 3：注册表重定向验证
```powershell
# 使用 RegEdit 加载虚拟 hive
# 检查注册表操作是否重定向到虚拟 hive
```

#### 功能 4：生成 HTML 测试报告
```powershell
# 收集测试结果
$testResults = @(
    @{ID="PT-01"; Name="本机不同路径"; Status="通过"; Details="..."},
    @{ID="PT-02"; Name="跨机器（Win10 → Win10）"; Status="待测试"; Details="..."},
    # ...
)

# 生成 HTML 报告
$html = @"
<!DOCTYPE html>
<html>
<head>
    <title>便携性验证报告</title>
    <style>
        table { border-collapse: collapse; width: 100%; }
        th, td { border: 1px solid #ddd; padding: 8px; text-align: left; }
        th { background-color: #4CAF50; color: white; }
        .pass { color: green; }
        .fail { color: red; }
        .pending { color: orange; }
    </style>
</head>
<body>
    <h1>便携性验证报告</h1>
    <table>
        <tr>
            <th>ID</th>
            <th>测试场景</th>
            <th>状态</th>
            <th>详情</th>
        </tr>
        $(foreach ($result in $testResults) {
            "<tr><td>$($result.ID)</td><td>$($result.Name)</td><td class='$($result.Status)'>$($result.Status)</td><td>$($result.Details)</td></tr>"
        })
    </table>
</body>
</html>
"@

# 保存报告
$html | Out-File -FilePath "tools\ci\test-portability-report.html" -Encoding utf8
```

### 6.3 脚本使用说明

```powershell
# 运行自动化测试（仅本机测试）
.\tools\ci\test-portability.ps1 -TestScope "Local"

# 运行所有测试（包括跨机器，需手动确认）
.\tools\ci\test-portability.ps1 -TestScope "All"

# 生成测试报告
.\tools\ci\test-portability.ps1 -GenerateReportOnly
```

### 6.4 CI/CD 集成（可选）

```yaml
# .github/workflows/portability-test.yml
name: Portability Test

on:
  push:
    branches: [ main ]
  pull_request:
    branches: [ main ]

jobs:
  test:
    runs-on: windows-latest
    steps:
      - uses: actions/checkout@v3
      - name: Run Portability Test
        run: .\tools\ci\test-portability.ps1 -TestScope "Local"
      - name: Upload Test Report
        uses: actions/upload-artifact@v3
        with:
          name: portability-test-report
          path: tools\ci\test-portability-report.html
```

---

## 7. 测试工具

| 工具 | 用途 | 下载地址 |
|------|------|----------|
| Process Monitor | 文件系统/注册表/进程监控 | https://learn.microsoft.com/en-us/sysinternals/downloads/procmon |
| RegEdit | 注册表编辑/导入/导出 | Windows 内置 |
| Process Explorer | 进程/模块/DLL 查看 | https://learn.microsoft.com/en-us/sysinternals/downloads/process-explorer |
| API Monitor | API 调用跟踪 | http://www.rohitab.com/downloads |
| Notepad++ | 测试应用 | https://notepad-plus-plus.org/downloads/ |
| 7-Zip | 测试应用 | https://www.7-zip.org/download.html |
| Firefox Portable | 测试应用 | https://portableapps.com/apps/internet/firefox_portable |

---

## 8. 测试进度跟踪

| 测试用例 | 计划开始日期 | 计划完成日期 | 实际开始日期 | 实际完成日期 | 状态 |
|----------|--------------|--------------|--------------|--------------|------|
| PT-01 | 2026-05-24 | 2026-05-24 |  |  | 待测试 |
| PT-02 | 2026-05-25 | 2026-05-25 |  |  | 待测试 |
| PT-03 | 2026-05-26 | 2026-05-26 |  |  | 待测试 |
| PT-04 | 2026-05-27 | 2026-05-27 |  |  | 待测试 |
| PT-05 | 2026-05-27 | 2026-05-27 |  |  | 待测试 |
| PT-06 | 2026-05-28 | 2026-05-28 |  |  | 待测试 |
| PT-07 | 2026-05-28 | 2026-05-28 |  |  | 待测试 |
| PT-08 | 2026-05-29 | 2026-05-29 |  |  | 待测试 |

---

## 9. 风险与问题

### 9.1 风险

| ID | 风险 | 影响 | 概率 | 缓解措施 | 负责人 |
|----|------|------|------|----------|--------|
| R-01 | 虚拟机 License 过期 | 测试环境不可用 | 中 | 提前申请 License / 使用免费虚拟机（如 VirtualBox） | QA |
| R-02 | 测试应用下载链接失效 | 无法捕获应用 | 低 | 提前下载安装包，保存到本地 | QA |
| R-03 | Hook 引擎在跨机器环境失效 | 测试结果不准确 | 中 | 在跨机器环境重新安装 Hook DLL | Dev |
| R-04 | 文件重定向性能损耗过高 | 用户体验差 | 低 | POC 不优化性能；若损耗过高，MVP 阶段优化 | Dev |

### 9.2 问题记录

| ID | 问题 | 严重程度 | 发现日期 | 解决日期 | 状态 | 解决方案 |
|----|------|----------|----------|----------|------|----------|
| Q-01 | （示例）虚拟机 A 无法启动 | 高 | 2026-05-24 |  | 待解决 | 检查虚拟机配置 |

---

## 10. 附录

### 10.1 参考资料

- POC 计划：`docs\POC-PLAN.md`
- 架构文档：`docs\ARCHITECTURE.md`
- Hook 引擎设计：`src\engine\hook\README.md`
- VFS 模块设计：`src\engine\vfs\README.md`
- VReg 模块设计：`src\engine\vreg\README.md`

### 10.2 修订历史

| 版本 | 日期 | 作者 | 变更说明 |
|------|------|------|----------|
| 0.1 | 2026-05-23 | QA | 初版，基于 POC 计划和架构文档输出 |

---

**文档结束**

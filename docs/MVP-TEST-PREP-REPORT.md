# MVP 测试准备报告

**项目**：ai-thinApp-portable-launchpad-platform  
**报告日期**：2026-05-24  
**负责人**：测试工程师（Subagent）  
**状态**：测试环境准备完成

---

## 1. 任务完成情况

| 交付物 | 路径 | 状态 | 说明 |
|--------|------|------|------|
| 测试环境配置文档 | `docs/MVP-TEST-ENV-SETUP.md` | ✅ 已完成 | 包含工具列表、版本要求、安装命令 |
| 测试数据 | `tests/data/*` | ✅ 已完成 | 创建了模拟数据（桩文件） |
| 全模块测试脚本 | `tools/ci/run_all_tests.bat` | ✅ 已完成 | GBK 编码 + CRLF |
| 覆盖率测试脚本 | `tools/ci/run_coverage.bat` | ✅ 已完成 | GBK 编码 + CRLF |
| 性能测试脚本 | `tools/ci/run_perf_tests.bat` | ✅ 已完成 | GBK 编码 + CRLF |
| 集成测试脚本 | `tools/ci/run_integration_tests.bat` | ✅ 已完成 | GBK 编码 + CRLF |
| 测试报告生成脚本 | `tools/ci/generate_test_report.ps1` | ✅ 已完成 | UTF-8 BOM + CRLF |
| 测试准备报告 | `docs/MVP-TEST-PREP-REPORT.md` | ✅ 已完成 | 本报告 |

**完成度**：8/8 交付物已完成（100%）

---

## 2. 测试环境配置

### 2.1 必需工具清单

| 工具 | 最低版本 | 推荐版本 | 用途 | 安装方式 |
|------|---------|---------|------|----------|
| OpenCppCoverage | 0.9.9 | 最新版 | 测试覆盖率测量 | Chocolatey |
| Python 3 | 3.12 | 3.12+ | 性能数据分析、报告生成 | Chocolatey |
| Jinja2 | 3.1 | 最新版 | HTML 报告模板渲染 | pip |
| Matplotlib | 3.8 | 最新版 | 性能图表生成 | pip |
| 7-Zip | 23.0 | 最新版 | `.vapp` 包解压 | Chocolatey |

### 2.2 安装命令（PowerShell）

```powershell
# 以管理员身份运行 PowerShell

# 1. 安装 Chocolatey（如果未安装）
Set-ExecutionPolicy Bypass -Scope Process -Force
iex ((New-Object System.Net.WebClient).DownloadString('https://chocolatey.org/install.ps1'))

# 2. 安装测试依赖
choco install opencppcoverage python312 7zip -y

# 3. 安装 Python 依赖
pip install Jinja2>=3.1 matplotlib>=3.8

# 4. 验证安装
OpenCppCoverage --version
python --version
python -c "import jinja2; print(jinja2.__version__)"
python -c "import matplotlib; print(matplotlib.__version__)"
7z --help
```

### 2.3 环境变量配置

确保以下路径已添加到 `PATH` 环境变量：

```
C:\ProgramData\chocolatey\bin\          # Chocolatey 安装的工具
C:\Python312\                          # Python 可执行文件
C:\Python312\Scripts\                 # pip 安装的脚本
```

---

## 3. 测试数据准备

### 3.1 基准应用（模拟数据）

| 文件 | 路径 | 大小 | 用途 | 状态 |
|------|------|------|------|------|
| `notepad++.exe` | `tests/data/notepad++.exe` | 147 B | 基准应用 1（便携版） | ✅ 模拟数据 |
| `7z.exe` | `tests/data/7z.exe` | 152 B | 基准应用 2（便携版） | ✅ 模拟数据 |
| `firefox_portable.exe` | `tests/data/firefox_portable.exe` | 156 B | 基准应用 3（便携版） | ✅ 模拟数据 |

**注意**：以上为模拟数据（桩文件），实际测试时应替换为真实的便携版可执行文件。

### 3.2 虚拟注册表测试数据

| 文件 | 路径 | 大小 | 用途 | 状态 |
|------|------|------|------|------|
| `test_registry.hive` | `tests/data/test_registry.hive` | 664 B | 虚拟注册表测试数据 | ✅ 模拟数据 |

**内容说明**：包含模拟的注册表键值，用于测试注册表虚拟化功能。

### 3.3 虚拟文件系统测试数据

| 目录/文件 | 路径 | 用途 | 状态 |
|----------|------|------|------|
| `test_vfs/ProgramFiles/` | `tests/data/test_vfs/ProgramFiles/` | 模拟程序文件目录 | ✅ 已创建 |
| `test_vfs/Windows/` | `tests/data/test_vfs/Windows/` | 模拟 Windows 系统目录 | ✅ 已创建 |
| `test_vfs/User/` | `tests/data/test_vfs/User/` | 模拟用户配置目录 | ✅ 已创建 |
| `app.exe` | `tests/data/test_vfs/ProgramFiles/app.exe` | 模拟程序文件 | ✅ 模拟数据 |
| `system.dll` | `tests/data/test_vfs/Windows/system.dll` | 模拟系统文件 | ✅ 模拟数据 |
| `settings.ini` | `tests/data/test_vfs/User/settings.ini` | 模拟配置文件 | ✅ 模拟数据 |

---

## 4. 测试脚本清单

### 4.1 脚本列表

| 脚本名称 | 路径 | 编码 | 换行符 | 用途 | 状态 |
|---------|------|------|--------|------|------|
| `run_all_tests.bat` | `tools/ci/run_all_tests.bat` | GBK | CRLF | 运行所有单元测试 | ✅ 已完成 |
| `run_coverage.bat` | `tools/ci/run_coverage.bat` | GBK | CRLF | 测量测试覆盖率 | ✅ 已完成 |
| `run_perf_tests.bat` | `tools/ci/run_perf_tests.bat` | GBK | CRLF | 运行性能基准测试 | ✅ 已完成 |
| `run_integration_tests.bat` | `tools/ci/run_integration_tests.bat` | GBK | CRLF | 运行集成测试 | ✅ 已完成 |
| `generate_test_report.ps1` | `tools/ci/generate_test_report.ps1` | UTF-8 BOM | CRLF | 生成 HTML 测试报告 | ✅ 已完成 |

### 4.2 脚本说明

#### `run_all_tests.bat`（全模块测试）
- **功能**：运行所有单元测试（Hook 引擎、VFS 缓存、路径重定向）
- **依赖**：编译后的测试可执行文件（`test_hook_x64.exe`、`test_vfs_cache_x64.exe`、`test_path_redirect_x64.exe`）
- **输出**：控制台输出测试结果
- **使用前**：必须调用 `vcvars64.bat` 初始化 VS2026 环境

#### `run_coverage.bat`（覆盖率测试）
- **功能**：使用 OpenCppCoverage 测量测试覆盖率
- **依赖**：OpenCppCoverage、编译后的测试可执行文件
- **输出**：`tests/coverage/*.xml`（Cobertura 格式）
- **使用前**：必须调用 `vcvars64.bat` 初始化 VS2026 环境

#### `run_perf_tests.bat`（性能测试）
- **功能**：运行性能基准测试，生成性能数据
- **依赖**：编译后的性能测试可执行文件、Python 3（可选，用于生成图表）
- **输出**：`tests/performance/*.json`、`performance_report.html`（可选）
- **使用前**：必须调用 `vcvars64.bat` 初始化 VS2026 环境

#### `run_integration_tests.bat`（集成测试）
- **功能**：运行端到端集成测试（虚拟注册表、虚拟文件系统、应用启动重定向、.vapp 包加载）
- **依赖**：编译后的集成测试可执行文件、测试数据（`tests/data/*`）
- **输出**：控制台输出测试结果
- **使用前**：必须调用 `vcvars64.bat` 初始化 VS2026 环境

#### `generate_test_report.ps1`（测试报告生成）
- **功能**：收集测试结果、覆盖率数据、性能数据，生成 HTML 测试报告
- **依赖**：Python 3、Jinja2、Matplotlib（可选，用于高级报告）
- **输出**：`tests/reports/test_report_*.html`
- **使用前**：确保测试结果已生成（运行测试脚本后）

---

## 5. 已知限制

### 5.1 编译依赖

所有测试脚本依赖编译后的可执行文件，这些文件在编译通过后才可用：

- `test_hook_x64.exe`
- `test_vfs_cache_x64.exe`
- `test_path_redirect_x64.exe`
- `test_hook_perf_x64.exe`
- `test_vfs_cache_perf_x64.exe`
- `test_path_redirect_perf_x64.exe`
- `test_registry_virtualization_x64.exe`
- `test_vfs_virtualization_x64.exe`
- `test_app_launch_redirect_x64.exe`
- `test_vapp_loader_x64.exe`
- `test_e2e_workflow_x64.exe`

**当前状态**：编译错误修复进行中（22/22 已修复），等待编译通过。

### 5.2 测试数据为模拟数据

当前测试数据为桩文件（模拟数据），不适用于实际功能测试。实际测试时需要：

1. 下载真实的便携版应用（Notepad++、7-Zip、Firefox Portable）
2. 准备有效的注册表 hive 文件
3. 准备有效的虚拟文件系统测试数据

### 5.3 Python 依赖可选

`generate_test_report.ps1` 和 `run_perf_tests.bat` 中的高级功能（HTML 报告、性能图表）依赖 Python 3 和相关库。如果 Python 不可用，这些脚本会跳过高级功能，但基础功能仍可用。

---

## 6. 下一步行动

### 6.1 立即行动（编译通过后 30 分钟内）

1. **验证编译通过**
   - 检查编译错误是否已全部修复（22/22）
   - 确认所有测试可执行文件已生成在 `build/output/x64/` 目录

2. **运行全模块测试**
   ```powershell
   cd D:\anmw_work_proj\ai-thinApp-portable-launchpad-platform\tools\ci
   .\run_all_tests.bat
   ```

3. **运行覆盖率测试**
   ```powershell
   .\run_coverage.bat
   ```

4. **运行性能测试**
   ```powershell
   .\run_perf_tests.bat
   ```

5. **运行集成测试**
   ```powershell
   .\run_integration_tests.bat
   ```

6. **生成测试报告**
   ```powershell
   .\generate_test_report.ps1
   ```

### 6.2 后续行动（测试通过后）

1. **替换真实测试数据**
   - 下载便携版 Notepad++、7-Zip、Firefox Portable
   - 准备有效的注册表 hive 文件和 VFS 测试数据

2. **配置 CI/CD**
   - 将测试脚本集成到 CI/CD 流水线
   - 配置自动测试和报告生成

3. **测试报告分析**
   - 分析覆盖率报告，识别未覆盖代码
   - 分析性能数据，识别性能瓶颈

---

## 7. 附录

### 7.1 文件编码说明

| 文件类型 | 编码 | BOM | 换行符 | 原因 |
|---------|------|-----|--------|------|
| `.md` | UTF-8 | 无 | CRLF | Windows 默认，无 BOM 避免 Git 问题 |
| `.bat` | GBK | 无 | CRLF | Windows 批处理文件含中文时需要使用 GBK |
| `.ps1` | UTF-8 | 有 | CRLF | PowerShell 脚本需要使用 UTF-8 BOM |

### 7.2 脚本使用注意事项

1. **必须以管理员身份运行**：某些测试可能需要管理员权限（如注册表虚拟化测试）
2. **必须初始化 VS2026 环境**：所有 `.bat` 脚本都会调用 `vcvars64.bat`，确保 VS2026 已正确安装
3. **路径不能包含空格**：某些旧版工具可能不支持带空格的路径，建议使用短路径或引号包裹路径

### 7.3 联系人

- **项目负责人**：[待填写]
- **编译工程师**：[待填写]
- **测试工程师**：Subagent（本报告作者）

---

**报告结束**

---

**变更日志**

| 版本 | 日期 | 作者 | 变更说明 |
|------|------|------|----------|
| 1.0 | 2026-05-24 | 测试工程师（Subagent） | 初始版本 |

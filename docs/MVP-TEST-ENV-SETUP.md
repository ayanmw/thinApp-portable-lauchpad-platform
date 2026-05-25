# MVP 测试环境配置文档

## 概述

本文档描述 MVP 测试环境的配置要求，包括所需工具、版本要求和安装命令。

## 工具清单

| 工具 | 版本要求 | 用途 | 必需 |
|------|---------|------|------|
| OpenCppCoverage | 0.9.9+ | 测试覆盖率测量 | 是 |
| Python 3 | 3.12+ | 性能数据分析 | 是 |
| Jinja2 | 3.1+ | HTML 报告生成 | 是 |
| Matplotlib | 3.8+ | 性能图表生成 | 是 |
| 7-Zip | 23.0+ | `.vapp` 包解压 | 是 |

## 安装步骤

### 1. 安装 Chocolatey（如果未安装）

以管理员身份运行 PowerShell，执行以下命令：

```powershell
Set-ExecutionPolicy Bypass -Scope Process -Force
iex ((New-Object System.Net.WebClient).DownloadString('https://chocolatey.org/install.ps1'))
```

### 2. 安装测试依赖

```powershell
# 安装 OpenCppCoverage
choco install opencppcoverage -y

# 安装 Python 3.12+
choco install python312 -y

# 安装 Jinja2
pip install Jinja2>=3.1

# 安装 Matplotlib
pip install matplotlib>=3.8

# 安装 7-Zip
choco install 7zip -y
```

### 3. 验证安装

```powershell
# 验证 OpenCppCoverage
OpenCppCoverage --version

# 验证 Python
python --version

# 验证 Jinja2
python -c "import jinja2; print(jinja2.__version__)"

# 验证 Matplotlib
python -c "import matplotlib; print(matplotlib.__version__)"

# 验证 7-Zip
7z --help
```

## 环境配置

### Python 环境配置

确保 Python 和 pip 已添加到 PATH 环境变量：

```powershell
# 检查 Python 是否在 PATH 中
python --version

# 检查 pip 是否可用
pip --version
```

如果未找到命令，手动添加 Python 安装路径到 PATH：

```
C:\Python312\
C:\Python312\Scripts\
```

### 测试数据准备

测试数据应放置在 `tests/data/` 目录：

```
tests/
  data/
    notepad++.exe          # 基准应用 1（便携版）
    7z.exe                 # 基准应用 2（便携版）
    firefox_portable.exe   # 基准应用 3（便携版）
    test_registry.hive     # 虚拟注册表测试数据
    test_vfs/              # 虚拟文件系统测试数据
```

## 测试执行

### 运行全模块测试

```powershell
cd tools/ci
.\run_all_tests.bat
```

### 运行覆盖率测试

```powershell
cd tools/ci
.\run_coverage.bat
```

### 运行性能测试

```powershell
cd tools/ci
.\run_perf_tests.bat
```

### 运行集成测试

```powershell
cd tools/ci
.\run_integration_tests.bat
```

### 生成测试报告

```powershell
cd tools/ci
.\generate_test_report.ps1
```

## 故障排除

### 问题 1：OpenCppCoverage 找不到

**解决方案**：确认 Chocolatey 安装路径已添加到 PATH：
```
C:\ProgramData\chocolatey\bin\
```

### 问题 2：Python 模块导入失败

**解决方案**：重新安装模块
```powershell
pip install --upgrade Jinja2 matplotlib
```

### 问题 3：脚本执行失败（编码问题）

**解决方案**：确认脚本使用 GBK 编码（`.bat`/`.ps1` 文件）

## 参考文档

- OpenCppCoverage 官方文档：https://github.com/OpenCppCoverage/OpenCppCoverage
- Python 官方文档：https://docs.python.org/3.12/
- Jinja2 文档：https://jinja.palletsprojects.com/
- Matplotlib 文档：https://matplotlib.org/
- 7-Zip 官方文档：https://www.7-zip.org/

---

**文档版本**：1.0  
**创建日期**：2026-05-24  
**更新日期**：2026-05-24

# MVP CMake + vcpkg 安装任务理解确认

**生成时间**: 2026-05-23 18:55  
**确认工程师**: 构建环境工程师  
**任务标签**: mvp-setup-cmake-vcpkg

---

## 任务理解

我是构建环境工程师，负责 AI ThinApp Portable Launchpad Platform 项目的构建环境完善任务。

**核心目标**: 安装 CMake 和 vcpkg，完成构建环境配置，使项目能够完整构建。

---

## 当前构建环境状态（已验证）

| 组件 | 状态 | 版本/路径 | 备注 |
|------|------|-----------|------|
| VS2026 | ✅ 已安装 | 18.5.3 | 需要通过 vcvars64.bat 加载环境 |
| MSVC 编译器 | ✅ 可用 | 19.50.35730 (x64) | cl.exe 可正常调用 |
| Windows SDK | ✅ 已安装 | 10.0.26100.0 | 目标版本已安装 |
| CMake | ❌ 未安装 | - | **需要安装** |
| vcpkg | ❌ 未安装 | - | **需要安装** |

**构建环境准备度**: 60% (3/5 组件已就绪)

---

## 任务分解

### 第一步：确认任务理解（当前步骤）

- [x] 读取 `docs/MVP-BUILD-ENV-VERIFICATION.md`
- [x] 确认当前构建环境状态
- [x] 输出任务理解确认到 `docs/MVP-CMAKE-VCPKG-SETUP-CONFIRM.md`

### 第二步：安装 CMake

**目标**: 安装 CMake 3.28+（最新稳定版），配置到系统 PATH

**具体任务**:
1. 下载 CMake 安装包（Windows x64 .msi 格式）
2. 命令行静默安装（/quiet /norestart）
3. 验证 CMake 安装（cmake --version）
4. 修改 `CMakeLists.txt`（设置最低版本要求）

**验收标准**:
- ✅ CMake 版本 ≥ 3.28.0
- ✅ CMake 已添加到系统 PATH
- ✅ CMake 可与 VS2026 配合使用

### 第三步：安装 vcpkg

**目标**: 安装 vcpkg 包管理器，配置到系统 PATH，安装项目依赖包

**具体任务**:
1. 克隆 vcpkg 仓库到 `C:\vcpkg`
2. 执行 bootstrap-vcpkg.bat
3. 配置 vcpkg 到系统 PATH
4. 安装项目依赖包（MinHook、nlohmann/json、zlib）
5. 集成 vcpkg 到 CMake 项目

**验收标准**:
- ✅ vcpkg 版本 ≥ 2024.11.16
- ✅ vcpkg 已添加到系统 PATH
- ✅ MinHook 包已安装
- ✅ nlohmann/json 包已安装
- ✅ zlib 包已安装

### 第四步：验证完整构建环境

**目标**: 验证 VS2026 + CMake + vcpkg 完整构建环境可用

**具体任务**:
1. 运行 `tools\build\build_all.bat`（完整构建脚本）
2. 检查构建输出（hook_engine.dll、launchpad.exe）
3. 运行测试程序（test_hook_x64.exe）

**验收标准**:
- ✅ 完整构建成功（0 错误，0 警告）
- ✅ 所有目标文件已生成
- ✅ 测试程序可运行（无崩溃，输出正确）

### 第五步：输出构建环境完善报告

**目标**: 创建 `docs/MVP-BUILD-ENV-SETUP-REPORT.md`

**报告内容**:
1. 任务完成情况
2. 构建环境状态
3. 安装日志
4. 已知限制
5. 下一步行动

---

## 交付物清单

| 文件 | 路径 | 状态 |
|------|------|------|
| 任务理解确认 | `docs/MVP-CMAKE-VCPKG-SETUP-CONFIRM.md` | ✅ 已完成 |
| CMake 安装验证 | `docs/MVP-CMAKE-VERIFICATION.md` | 待完成 |
| vcpkg 安装验证 | `docs/MVP-VCPKG-VERIFICATION.md` | 待完成 |
| 依赖包安装日志 | `docs/MVP-VCPKG-DEPENDENCIES.md` | 待完成 |
| 完整构建验证 | `docs/MVP-BUILD-VERIFICATION-FINAL.md` | 待完成 |
| 构建环境完善报告 | `docs/MVP-BUILD-ENV-SETUP-REPORT.md` | 待完成 |

---

## 关键注意事项

1. **所有文件使用 UTF-8 编码（无 BOM）**
2. **代码注释用中文**
3. **若遇到阻塞（如下载失败、安装失败），立即记录到 `docs/MVP-BLOCKERS.md` 并上报 PM**
4. **优先保证 CMake 和 vcpkg 安装成功，再安装依赖包**
5. **使用 PowerShell 脚本自动化安装过程（避免手动操作）**
6. **验证构建环境时，优先验证 x64 平台（Win32 平台可选）**
7. **如果网络下载速度慢，考虑使用国内镜像（如清华大学 TUNA 镜像）**

---

## 安装策略

### CMake 安装策略
- 使用官方下载：https://cmake.org/download/
- 选择 Windows x64 .msi 格式安装包
- 版本：CMake 3.28.0 或更高
- 静默安装参数：`/quiet /norestart`
- 勾选 "Add CMake to the system PATH for all users"

### vcpkg 安装策略
- 克隆官方仓库：https://github.com/microsoft/vcpkg
- 安装路径：`C:\vcpkg`
- 执行 bootstrap-vcpkg.bat
- 添加 `C:\vcpkg` 到系统 PATH
- 使用国内镜像加速（如需要）

### 依赖包安装策略
- 优先安装必需包：MinHook、nlohmann/json、zlib
- Qt6 作为可选包（大型依赖，按需安装）
- 使用 x64-windows 三元组
- 如网络慢，配置国内镜像

---

## 下一步行动

1. **立即开始第二步：安装 CMake**
   - 下载 CMake 安装包
   - 执行静默安装
   - 验证安装结果

2. **完成后继续第三步：安装 vcpkg**
   - 克隆 vcpkg 仓库
   - 执行 bootstrap
   - 安装依赖包

3. **最后执行第四步：验证完整构建环境**
   - 运行完整构建脚本
   - 检查构建输出
   - 运行测试程序

---

## 任务确认声明

我已经完整理解任务要求，确认当前构建环境状态，准备好执行 CMake 和 vcpkg 安装任务。

**准备开始工作**。

---

**确认文档结束**

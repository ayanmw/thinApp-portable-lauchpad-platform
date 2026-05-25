# AI ThinApp Portable Launchpad Platform

> 版本：v0.1.0 MVP
> 更新日期：2026-05-24
> 个人软件便携化平台——每个软件自带独立沙箱，所有依赖、配置、数据全在软件目录下，重装系统后复制文件夹即可恢复全部使用状态。

[![CI](https://github.com/anmingwei/ai-thinApp-portable-lauchpad-platform/actions/workflows/ci.yml/badge.svg)](https://github.com/anmingwei/ai-thinApp-portable-lauchpad-platform/actions/workflows/ci.yml)
[![License: GPL v3](https://img.shields.io/badge/License-GPL%20v3-blue.svg)](LICENSE)
[![GitHub Release](https://img.shields.io/github/v/release/anmingwei/ai-thinApp-portable-lauchpad-platform?include_prereleases)](https://github.com/anmingwei/ai-thinApp-portable-lauchpad-platform/releases)
[![GitHub Issues](https://img.shields.io/github/issues/anmingwei/ai-thinApp-portable-lauchpad-platform)](https://github.com/anmingwei/ai-thinApp-portable-lauchpad-platform/issues)
[![GitHub Discussions](https://img.shields.io/github/discussions/anmingwei/ai-thinApp-portable-lauchpad-platform)](https://github.com/anmingwei/ai-thinApp-portable-lauchpad-platform/discussions)

---

## 目录

- [项目定位](#项目定位)
- [核心功能](#核心功能)
- [技术架构](#技术架构)
- [快速开始](#快速开始)
- [项目结构](#项目结构)
- [贡献指南](#贡献指南)
- [社区](#社区)
- [FAQ](#faq)
- [许可证](#许可证)

---

## 项目定位

- **核心目标**：通过底层 API Hook 实现文件系统/注册表虚拟化，让每个应用的所有运行状态完全隔离在自身目录内
- **用户体验**：Launchpad 一键面板管理所有应用，可替换 Windows 开始菜单
- **差异化**：每个软件独立沙箱（vs Sandboxie 多软件共享沙箱），便携化更彻底
- **开源协议**：GPL v3，GitHub 公开仓库

## 核心功能

### 1. 应用虚拟化引擎
- **文件系统虚拟化**：拦截文件 API，将写入重定向到应用沙箱目录
- **注册表虚拟化**：拦截注册表 API，将写入重定向到虚拟 hive 文件
- **进程隔离**：子进程自动继承 Hook，实现多层隔离
- **Hook 引擎**：基于 MinHook 的用户态 Inline Hook

### 2. 应用打包工具
- **应用捕获**：快照差异生成 .vapp 包
- **.vapp 包格式**：包含文件数据、注册表 hive、元数据
- **包编辑器**：查看/编辑/合并 .vapp 包

### 3. Launchpad 客户端
- **应用库管理**：卡片视图/列表视图，搜索/分类/状态指示
- **应用启动器**：一键启动，沙箱/真实模式切换，多实例管理
- **软件商店**：社区 .vapp 包浏览/下载/安装
- **设置中心**：全局 Hook 开关，沙箱路径配置，外观设置

---

## 技术架构

```
Launchpad UI (Win32 / Electron)
        │
        ▼
  应用管理器 (App Manager)
        │
        ▼
  核心运行时容器 (每应用独立)
  ┌─────────────────────────────┐
  │  API Hook 引擎 (用户态 V1) │
  │  VFS 虚拟文件系统           │
  │  VReg 虚拟注册表           │
  │  进程隔离 + 子进程继承      │
  └─────────────────────────────┘
        │
        ▼
  虚拟包格式 (.vapp)
  [header][file-tree][registry-hive][manifest]
```

---

## 快速开始 (Quick Start)

### 环境要求

- **操作系统**：Windows 10 22H2+ / Windows 11 23H2+
- **编译器**：Visual Studio 2022 (MSVC 17.0+)，勾选"使用 C++ 的桌面开发"
- **CMake**：3.20+
- **Windows SDK**：10.0.22621.0+
- **第三方依赖**：MinHook (自动下载)，zlib，RapidJSON
- **Python**：3.x (用于脚本)

### 编译步骤

```bash
# 1. 克隆仓库
git clone https://github.com/anmingwei/ai-thinApp-portable-lauchpad-platform.git
cd ai-thinApp-portable-lauchpad-platform

# 2. 运行构建脚本（自动下载依赖、生成项目、编译）
.\build_all.bat

# 3. 编译输出在 build\ 目录
# - 引擎：build\src\engine\*.dll / *.exe
# - Launchpad：build\src\launchpad\ai-thinapp.exe
```

**可选：使用 Visual Studio IDE 编译**
1. 运行 `build_all.bat` 生成项目文件
2. 打开 `build\ai-thinapp.sln`
3. 选择配置（Debug/Release，x64/Win32）
4. 生成解决方案（Build Solution）

### 运行测试

```bash
# 运行引擎单元测试
.\tests\engine\test_hook.exe
.\tests\engine\test_vfs.exe
.\tests\engine\test_vreg.exe

# 运行集成测试（需要管理员权限）
.\tests\integration\test_app_launch.exe
```

---

## 项目结构

```
D:\anmw_work_proj\ai-thinApp-portable-lauchpad-platform\
├── docs/                      # 项目文档
│   ├── SPEC.md                # 产品规格说明书
│   ├── ARCHITECTURE.md        # 技术架构文档
│   ├── POC-PLAN.md            # POC 验证计划
│   ├── RISK-REGISTER.md       # 风险登记册
│   ├── UX-RESEARCH.md         # 竞品交互范式研究
│   ├── UX-DESIGN.md           # 交互设计文档
│   ├── SANDBOX-VISUAL-IDENTITY.md  # 沙箱视觉标识方案
│   ├── DEV-GUIDE.md           # 开发者指南（本仓库）
│   ├── FAQ.md                 # 常见问题解答
│   └── DOC-CONSISTENCY-CHECK.md  # 文档一致性校验报告
├── src/                       # 源代码
│   ├── engine/                # Hook 引擎核心（hook/, vfs/, vreg/, process/）
│   ├── packager/              # 应用捕获 + .vapp 打包
│   ├── launchpad/             # Launchpad 客户端（GUI）
│   └── sync/                  # 云端同步（V2）
├── tests/                     # 测试代码
│   ├── engine/                # 引擎单元测试
│   ├── packager/              # 打包工具测试
│   ├── launchpad/             # Launchpad 测试
│   └── integration/           # 集成测试
├── tools/                     # 工具脚本
│   ├── build_all.bat          # 一键构建脚本
│   ├── setup_dev_env.ps1      # 开发环境配置脚本
│   └── ...
├── assets/                    # 资源文件（图标、图片等）
├── third_party/               # 第三方依赖（MinHook, zlib 等）
├── .github/                   # GitHub 配置（CI/CD, Issue 模板等）
├── .gitignore                 # Git 忽略规则
├── CMakeLists.txt              # CMake 根配置文件
├── LICENSE                    # GPL v3 许可证
├── CHANGELOG.md               # 更新日志
└── README.md                  # 本文件
```

### 目录说明

| 目录/文件 | 用途 |
|----------|------|
| `docs/` | 所有项目文档，包括规格、架构、设计、风险登记册等 |
| `src/engine/` | Hook 引擎核心代码，包括 API Hook、VFS、VReg、进程隔离 |
| `src/packager/` | 应用捕获工具和 .vapp 包打包逻辑 |
| `src/launchpad/` | Launchpad 客户端 GUI 代码 |
| `src/sync/` | 云端同步功能（V2 阶段实现） |
| `tests/` | 所有测试代码，包括单元测试和集成测试 |
| `tools/` | 构建脚本、环境配置脚本等工具 |
| `assets/` | 图标、图片、UI 资源等 |
| `third_party/` | 第三方依赖库（MinHook, zlib, RapidJSON 等） |

---

## 贡献指南 (Contributing)

我们欢迎任何形式的贡献！请先阅读本指南，确保贡献过程顺利。

### 如何报告 Bug

1. **检查现有 Issue**：先在 [GitHub Issues](https://github.com/anmingwei/ai-thinApp-portable-lauchpad-platform/issues) 中搜索，确认 Bug 未被报告
2. **创建新 Issue**：点击"New Issue"，选择"Bug Report"模板
3. **提供详细信息**：
   - 复现步骤（尽量详细）
   - 预期行为 vs 实际行为
   - 环境信息（Windows 版本、编译器版本、Commit Hash）
   - 相关日志或截图

### 如何提交 Pull Request

1. **Fork 仓库**：点击 GitHub 页面右上角"Fork"按钮
2. **创建分支**：
   ```bash
   git checkout -b feat/your-feature-name  # 新功能
   git checkout -b fix/your-bug-fix       # Bug 修复
   ```
3. **提交更改**：
   - 遵循 [Conventional Commits](https://www.conventionalcommits.org/) 规范
   - 示例：`feat(hook): 添加对 NtDeleteFile 的 Hook 支持`
4. **推送分支**：`git push origin feat/your-feature-name`
5. **创建 PR**：
   - 填写 PR 模板（描述、关联 Issue、测试计划）
   - 等待 Code Review
   - 根据 Review 意见修改

### 代码规范

#### C++ 编码规范
- **标准**：C++17
- **命名**：
  - 类名：大驼峰（`HookEngine`）
  - 函数名：小驼峰（`installHook`）
  - 变量名：小驼峰（`hookCount`）
  - 常量名：全大写下划线（`MAX_HOOK_COUNT`）
  - 成员变量：小驼峰 + 下划线后缀（`hook_count_`）
- **注释**：使用中文注释，解释"为什么"而不是"是什么"
- **格式**：使用 clang-format，配置文件 `.clang-format`（待添加）
- **文件编码**：UTF-8（无 BOM）

#### Git 提交规范
遵循 [Conventional Commits](https://www.conventionalcommits.org/)：

```
<类型>(<范围>): <主题>

[可选正文]

[可选脚注]
```

**类型**：
- `feat`：新功能
- `fix`：Bug 修复
- `docs`：文档更新
- `style`：代码格式（不影响功能）
- `refactor`：重构（不是新功能也不是 Bug 修复）
- `perf`：性能优化
- `test`：测试相关
- `chore`：构建/工具链相关

**示例**：
```
feat(hook): 添加对 NtDeleteFile 的 Hook 支持

- 在 hook_engine.cpp 中添加 NtDeleteFile Hook
- 更新测试用例 test_hook.cpp
- 更新文档 ARCHITECTURE.md

Closes #123
```

### 代码审查流程

1. **自动检查**：CI/CD 自动运行编译、单元测试、代码格式检查
2. **人工审查**：至少 1 名核心贡献者 Review 通过后，才能合并
3. **修改**：根据 Review 意见修改代码，然后重新提交 Review

---

## 社区 (Community)

### GitHub 仓库
[https://github.com/anmingwei/ai-thinApp-portable-lauchpad-platform](https://github.com/anmingwei/ai-thinApp-portable-lauchpad-platform)

### 讨论区
[GitHub Discussions](https://github.com/anmingwei/ai-thinApp-portable-lauchpad-platform/discussions) - 提问、建议、想法交流

### 问题反馈
[GitHub Issues](https://github.com/anmingwei/ai-thinApp-portable-lauchpad-platform/issues) - Bug 报告、功能请求

### 邮件列表
暂无（计划建立）

---

## FAQ

### 为什么选择 GPL v3 协议？

GPL v3 确保代码永远开源，任何人都可以自由使用、修改、分发本项目的代码，但衍生项目也必须开源。这符合我们的开源理念：透明、共享、社区驱动。

### 杀软误报怎么办？

由于本项目使用 API Hook 技术（类似病毒行为），可能会被杀软误报。解决方法：
1. **添加信任**：将 `ai-thinapp.exe` 和 `hook_engine.dll` 添加到杀软白名单
2. **代码签名**：我们将申请代码签名证书，减少误报
3. **开源透明**：所有代码在 GitHub 公开，任何人都可以审计

### 支持 Windows 7 吗？

不支持。本项目目标用户是 Windows 10/11 个人用户，Windows 7 已停止支持，不再考虑兼容性。

### 如何贡献应用打包配置？

1. 在 `community/` 目录下创建 `{app-name}.json`（应用打包配置）
2. 提交 Pull Request
3. 我们将审核配置的正确性，然后合并

---

## 许可证 (License)

本项目采用 **GNU General Public License v3.0** (GPL v3) 协议。

详见 [LICENSE](LICENSE) 文件。

---

## 开发阶段

| 阶段 | 周期 | 目标 |
|------|------|------|
| POC | 4周 | Hook 引擎验证 + 三应用跑通 |
| MVP | 10周 | 可发布给硬核用户 |
| 体验完善 | 8周 | 普通用户可上手 |
| 生态与企业 | 待定 | 社区市场 + 企业版 |

---

## 构建

```bash
# 依赖：Visual Studio 2022 + CMake 3.20+
mkdir build && cd build
cmake ..
msbuild ai-thinapp.sln /p:Configuration=Release
```

---

## 致谢

- [MinHook](https://github.com/TsudaKageyu/minhook) - 轻量级 API Hook 框架
- [Sandboxie Plus](https://github.com/sandboxie-plus/Sandboxie) - 沙箱技术参考
- [Cameyo](https://www.cameyo.com/) - 应用虚拟化参考
- [VMware ThinApp](https://www.vmware.com/products/thinapp) - 企业级应用虚拟化参考

---

**最后更新**：2026-05-23  
**维护者**：AI ThinApp Portable Launchpad Platform 开源社区

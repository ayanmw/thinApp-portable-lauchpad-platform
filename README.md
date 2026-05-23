# AI ThinApp Portable Launchpad Platform

> 个人软件便携化平台——每个软件自带独立沙箱，所有依赖、配置、数据全在软件目录下，重装系统后复制文件夹即可恢复全部使用状态。

## 项目定位

- **核心目标**：通过底层 API Hook 实现文件系统/注册表虚拟化，让每个应用的所有运行状态完全隔离在自身目录内
- **用户体验**：Launchpad 一键面板管理所有应用，可替换 Windows 开始菜单
- **差异化**：每个软件独立沙箱（vs Sandboxie 多软件共享沙箱），便携化更彻底
- **开源协议**：GPL v3，GitHub 公开仓库

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

## 目录结构

```
D:\anmw_work_proj\ai-thinApp-portable-lauchpad-platform\
├── docs/              # 规格、架构、风险登记册
├── src/
│   ├── engine/        # Hook 引擎核心
│   ├── packager/      # 应用捕获 + .vapp 打包
│   ├── launchpad/     # Launchpad 客户端
│   └── sync/          # 云端同步 (V2)
├── tests/
├── tools/
├── assets/
├── third_party/       # MinHook 等第三方依赖
├── LICENSE
└── README.md
```

## 开发阶段

| 阶段 | 周期 | 目标 |
|------|------|------|
| POC | 4周 | Hook 引擎验证 + 三应用跑通 |
| MVP | 10周 | 可发布给硬核用户 |
| 体验完善 | 8周 | 普通用户可上手 |
| 生态与企业 | 待定 | 社区市场 + 企业版 |

## 构建

```bash
# 依赖：Visual Studio 2022 + CMake 3.20+
mkdir build && cd build
cmake ..
msbuild ai-thinapp.sln /p:Configuration=Release
```

## 许可证

GPL v3 — 详见 LICENSE 文件。

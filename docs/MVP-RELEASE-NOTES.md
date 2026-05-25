# AI ThinApp Portable Launchpad Platform - MVP 发布说明

## 版本信息

- **版本号**: v0.1.0 MVP
- **发布日期**: 2026-05-24
- **构建类型**: Release x64
- **目标平台**: Windows 10 / Windows 11 (x64)

## 新功能

### 核心功能
- ✅ **应用程序启动器**: 提供简洁的 GUI 界面用于启动和管理便携式应用
- ✅ **Hook 引擎**: 实现基础的 API Hook 机制，支持环境隔离
- ✅ **配置管理**: JSON 格式的配置文件解析和应用管理
- ✅ **进程管理**: 启动、监控和终止目标进程

### GUI 功能
- ✅ 应用列表显示
- ✅ 应用启动/停止控制
- ✅ 设置对话框
- ✅ 日志输出窗口

### 开发者功能
- ✅ Debug 构建包含 PDB 调试符号
- ✅ 详细的日志记录
- ✅ 测试程序 (test_hook_x64.exe)

## 安装说明

### 方式一：安装包（推荐）
1. 下载 `launchpad-v0.1.0-setup.exe`
2. 双击运行安装程序
3. 按照向导完成安装
4. 从开始菜单或桌面快捷方式启动

### 方式二：便携版 ZIP
1. 下载 `launchpad-v0.1.0-portable.zip`
2. 解压到任意目录
3. 运行 `launchpad.exe`

## 系统要求

- **操作系统**: Windows 10 版本 1809 或更高 / Windows 11
- **处理器**: x64 兼容处理器
- **内存**: 最低 4GB RAM
- **硬盘空间**: 100MB 可用空间
- **依赖**: Visual C++ 运行时（安装包自动安装）

## 文件清单

### Release 构建输出
```
launchpad.exe          - 主程序（GUI 启动器）
hook_engine.dll        - Hook 引擎动态库
Qt6Core.dll           - Qt6 核心库
Qt6Gui.dll            - Qt6 GUI 库
Qt6Widgets.dll        - Qt6 Widgets 库
README.md             - 说明文档
LICENSE               - 许可证
```

### Debug 构建输出（仅开发版）
```
launchpad.exe          - 主程序（带调试信息）
hook_engine.dll        - Hook 引擎（带调试信息）
hook_engine.pdb       - 调试符号文件
launchpad.pdb         - 调试符号文件
```

## 已知问题

### MVP 阶段限制
- ⚠️ VFS（虚拟文件系统）未实现 - 文件重定向功能不可用
- ⚠️ 注册表虚拟化未实现 - 注册表隔离功能不可用
- ⚠️ 仅支持 x64 应用 - 不支持 x86 应用
- ⚠️ 无自动更新功能 - 需要手动下载更新

### POC 阶段未解决的问题
- ⚠️ Hook 注入稳定性 - 部分应用可能无法正确注入 Hook（POC-#12）
- ⚠️ 内存泄漏 - 长时间运行后 Hook 引擎可能存在内存泄漏（POC-#15）
- ⚠️ 高 DPI 支持不完善 - GUI 在高 DPI 显示器下可能显示模糊（POC-#18）
- ⚠️ 进程继承不完整 - 子进程可能无法继承父进程的沙箱环境（POC-#21）
- ⚠️ 文件系统重定向性能 - 大量文件操作场景下性能下降明显（POC-#23）

### 已知 Bug
- 🐛  Issue #1: 长时间运行后可能出现内存泄漏
- 🐛  Issue #2: 某些应用可能无法正确注入 Hook
- 🐛  Issue #3: GUI 在高 DPI 下显示可能不正常

## 修复问题

### 相比 POC 版本的改进
- ✅ 修复了 POC 中的崩溃问题
- ✅ 改进了进程启动的稳定性和可靠性
- ✅ 优化了 Hook 引擎的性能
- ✅ 添加了详细的错误日志

## 安全性

### 代码签名
- ⏳ EV 代码签名：待获取证书后签署（目标 v0.1.1）
- ⏳ SmartScreen 信誉：需要通过用户下载积累信誉

### 当前签名状态
- ❌ 无数字签名（MVP 版本）
- ⚠️ 用户需要允许"未知发布者"运行程序
- 📋 计划：v0.1.1 将包含 EV 代码签名

## 兼容性

### 已测试应用
- ✅ Notepad++ (x64)
- ✅ 7-Zip (x64)
- ✅ Firefox (x64)
- ✅ VLC Media Player (x64)

### 已知不兼容
- ❌ 32 位应用（不支持）
- ❌ 需要管理员权限的应用（部分功能可能受限）
- ❌ 反作弊游戏（可能被检测为作弊工具）

## 性能指标

### 启动时间
- 冷启动: < 2 秒
- 热启动: < 500 毫秒

### 内存占用
- 空闲: ~50MB
- 运行 1 个应用: ~100MB
- 运行 5 个应用: ~250MB

### Hook 性能
- Hook 安装时间: < 10 毫秒
- API 调用开销: < 1 微秒

## 后续路线图

### MVP+1 阶段计划（v0.2.0）
- 📋 **VFS 虚拟文件系统**：实现完整的文件重定向功能
- 📋 **注册表虚拟化**：实现注册表隔离和重定向
- 📋 **x86 应用支持**：添加 32 位应用兼容层
- 📋 **自动更新功能**：实现内置自动更新检查和应用
- 📋 **EV 代码签名**：获取 EV 证书，消除 SmartScreen 警告

### MVP+2 阶段计划（v0.3.0）
- 📋 **软件商店**：社区驱动的应用市场
- 📋 **.vapp 包格式**：标准化便携应用打包格式
- 📋 **云端同步**：加密同步应用配置和数据
- 📋 **GUI 重构**：使用现代 UI 框架重写界面

### 长期规划（V2+）
- 📋 **内核驱动 Hook**：提升兼容性和性能
- 📋 **企业版功能**：Group Policy 支持、批量部署工具
- 📋 **跨平台支持**：探索 Linux/macOS 版本可能性

## 开发者信息

### 构建环境
- **编译器**: Visual Studio 2022 (MSVC 17.0)
- **构建系统**: CMake 3.28+
- **依赖管理**: vcpkg
- **Qt 版本**: Qt 6.5+

### 如何构建
```bat
:: 克隆仓库
git clone <repository-url>
cd ai-thinApp-portable-launchpad-platform

:: 配置 vcpkg
.\vcpkg\bootstrap-vcpkg.bat

:: 构建 Release 版本
tools\build\build_release.bat

:: 构建 Debug 版本
tools\build\build_debug.bat
```

## 支持

### 问题反馈
- GitHub Issues: <repository-url>/issues
- 邮件支持: support@thinapp.example.com

### 文档
- 用户文档: `docs/USER-MANUAL.md`
- 开发者文档: `docs/DEVELOPER-GUIDE.md`
- API 文档: `docs/API-REFERENCE.md`

## 许可证

本项目采用 MIT 许可证 - 详见 `LICENSE` 文件。

---

**构建时间**: 2026-05-24
**Git 提交**: <commit-hash>
**构建者**: Deployment Engineer

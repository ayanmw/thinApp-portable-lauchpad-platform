# 构建系统说明

> 本文档说明如何初始化 Git 仓库、配置构建环境和编译项目。

## 1. Git 仓库初始化（手动步骤）

本项目目录**尚未初始化 Git 仓库**，请手动执行以下步骤：

### 1.1 初始化 Git 仓库

```bash
# 进入项目目录
cd /d/anmw_work_proj/ai-thinApp-portable-lauchpad-platform

# 初始化 Git 仓库
git init

# 添加所有文件到暂存区
git add .

# 提交初始版本
git commit -m "Initial commit: project structure and build system"
```

### 1.2 关联远程仓库（可选）

```bash
# 关联 GitHub 远程仓库
git remote add origin https://github.com/anmingwei/ai-thinApp-portable-lauchpad-platform.git

# 推送初始提交
git push -u origin main
```

---

## 2. 构建环境要求

### 2.1 必需软件

| 软件 | 版本要求 | 下载地址 |
|------|----------|----------|
| Visual Studio 2022 | Community/Professional/Enterprise | https://visualstudio.microsoft.com/ |
| CMake | 3.20+ | https://cmake.org/download/ |
| Windows SDK | 10.0.22621.0+ | 随 Visual Studio 安装 |

### 2.2 Visual Studio 工作负载

安装 Visual Studio 2022 时，选择以下工作负载：
- **使用 C++ 的桌面开发** (Desktop development with C++)
- **Windows 10/11 SDK**

### 2.3 验证环境

打开 **x64 Native Tools Command Prompt for VS 2022**，运行：

```bash
# 验证 MSVC 编译器
cl

# 验证 CMake
cmake --version

# 验证 NMake
nmake /?

# 验证 Windows SDK
rc /?
```

---

## 3. 第三方依赖

### 3.1 MinHook 库

本项目使用 **MinHook** 作为 API Hook 框架。

#### 方式一：使用已提供的 minhook.zip

项目目录已包含 `third_party/minhook.zip`，构建系统会自动解压。

#### 方式二：手动下载

1. 访问 MinHook 发布页面：https://github.com/TsudaKageyu/minhook/releases
2. 下载最新版本的源码压缩包
3. 解压到 `third_party/minhook` 目录

#### 方式三：使用 vcpkg

```bash
# 安装 vcpkg（如果尚未安装）
git clone https://github.com/microsoft/vcpkg.git
.\vcpkg\bootstrap-vcpkg.bat

# 安装 MinHook
.\vcpkg\vcpkg install minhook
```

---

## 4. 构建项目

### 4.1 使用构建脚本（推荐）

项目提供了三个构建脚本，位于 `tools\build\` 目录：

#### 4.1.1 编译所有目标

```bash
cd tools\build
build_all.bat
```

此脚本会编译以下配置：
- x64 Debug
- x64 Release
- Win32 Debug
- Win32 Release

#### 4.1.2 仅编译 Hook 引擎

```bash
cd tools\build
build_hook.bat [架构] [配置]

# 示例
build_hook.bat x64 Debug       # 编译 x64 Debug
build_hook.bat Win32 Release   # 编译 Win32 Release
build_hook.bat                 # 默认 x64 Debug
```

#### 4.1.3 清理编译产物

```bash
cd tools\build
clean.bat
```

### 4.2 手动构建

#### 4.2.1 配置 CMake

```bash
# 创建构建目录
mkdir build\x64\Debug
cd build\x64\Debug

# 运行 CMake 配置
cmake ..\..\.. -G "NMake Makefiles" -DCMAKE_BUILD_TYPE=Debug
```

#### 4.2.2 编译

```bash
# 使用 NMake 编译
nmake

# 或使用 CMake 编译
cmake --build .
```

#### 4.2.3 清理

```bash
# 删除构建目录
rmdir /s /q build
```

---

## 5. 项目结构

### 5.1 目录说明

```
ai-thinApp-portable-lauchpad-platform\
├── .gitignore                  # Git 忽略规则
├── CMakeLists.txt              # 根 CMake 配置
├── third_party\               # 第三方依赖
│   ├── minhook.zip            # MinHook 源码包
│   └── CMakeLists.txt         # 第三方依赖构建配置
├── src\
│   ├── engine\                # Hook 引擎核心
│   │   ├── common\           # 公共工具函数
│   │   ├── hook\             # Hook 引擎
│   │   ├── vfs\              # 虚拟文件系统
│   │   ├── vreg\             # 虚拟注册表
│   │   └── process\          # 进程隔离
│   ├── packager\             # 应用打包工具
│   ├── launchpad\            # Launchpad 客户端
│   └── sync\                 # 云端同步（V2）
├── tests\                     # 测试模块
│   ├── engine\               # 引擎测试
│   ├── compatibility\        # 兼容性测试
│   └── performance\          # 性能测试
├── tools\
│   ├── build\                # 构建脚本
│   │   ├── build_all.bat
│   │   ├── build_hook.bat
│   │   └── clean.bat
│   └── ci\                   # CI/CD 工具
└── docs\                     # 文档
    ├── ARCHITECTURE.md       # 技术架构文档
    ├── POC-PLAN.md           # POC 验证计划
    └── SPEC.md               # 产品规格说明书
```

---

## 6. 编译输出

### 6.1 输出目录结构

```
build\
├── x64\
│   ├── Debug\
│   │   ├── bin\              # 可执行文件和 DLL
│   │   ├── lib\              # 静态库
│   │   └── pdb\             # 调试符号
│   └── Release\
└── Win32\
    ├── Debug\
    └── Release\
```

### 6.2 目标文件

| 目标 | 类型 | 说明 |
|------|------|------|
| engine_common | 静态库 (.lib) | 公共工具函数 |
| engine_hook | 静态库 (.lib) | Hook 引擎 |
| engine_vfs | 静态库 (.lib) | 虚拟文件系统 |
| engine_vreg | 静态库 (.lib) | 虚拟注册表 |
| engine_process | 静态库 (.lib) | 进程隔离 |

---

## 7. 常见问题

### 7.1 编译失败：找不到 vcvars64.bat

**原因**：Visual Studio 未安装或环境变量未设置。

**解决**：
1. 确认 Visual Studio 2022 已安装（包含 C++ 工作负载）
2. 手动设置 `VCVARS` 环境变量，指向 `vcvars64.bat` 的完整路径
3. 使用 **x64 Native Tools Command Prompt for VS 2022** 运行构建脚本

### 7.2 编译失败：找不到 MinHook

**原因**：MinHook 源码未下载或未正确放置。

**解决**：
1. 检查 `third_party/minhook` 目录是否存在
2. 如果不存在，解压 `third_party/minhook.zip`
3. 或手动下载 MinHook 源码并放置到 `third_party/minhook` 目录

### 7.3 CMake 配置失败

**原因**：CMake 版本过低或生成器选择错误。

**解决**：
1. 升级 CMake 到 3.20+ 版本
2. 使用正确的生成器：`-G "NMake Makefiles"`
3. 清理缓存后重新配置：删除 `CMakeCache.txt` 和 `CMakeFiles\` 目录

---

## 8. 下一步

1. **实现 Hook 引擎**：填充 `src/engine/hook/hook_engine.cpp` 中的桩函数
2. **实现 VFS 模块**：填充 `src/engine/vfs/vfs.cpp` 中的桩函数
3. **实现 VReg 模块**：填充 `src/engine/vreg/vreg.cpp` 中的桩函数
4. **实现 Process 模块**：填充 `src/engine/process/process_inherit.cpp` 中的桩函数
5. **编写单元测试**：在 `tests/engine/` 目录添加单元测试

---

## 9. 参考文档

- [技术架构文档](docs/ARCHITECTURE.md)
- [POC 验证计划](docs/POC-PLAN.md)
- [产品规格说明书](docs/SPEC.md)
- [MinHook 项目](https://github.com/TsudaKageyu/minhook)
- [CMake 官方文档](https://cmake.org/documentation/)

---

**最后更新**：2026-05-23

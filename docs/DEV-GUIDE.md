# 开发者指南 (Developer Guide)

> 本文档为 AI ThinApp Portable Launchpad Platform 项目的开发者提供详细的环境搭建、编译、调试、代码规范、添加新功能、常见问题等指南。

---

## 目录

1. [环境搭建](#1-环境搭建)
2. [编译指南](#2-编译指南)
3. [调试指南](#3-调试指南)
4. [代码规范](#4-代码规范)
5. [添加新功能](#5-添加新功能)
6. [常见问题](#6-常见问题)

---

## 1. 环境搭建

### 1.1 Visual Studio 2022 安装

1. **下载**：从 [Visual Studio 官网](https://visualstudio.microsoft.com/zh-hans/downloads/) 下载 Visual Studio 2022 Community（免费）
2. **安装工作负载**：运行安装程序，勾选以下工作负载：
   - ✅ **使用 C++ 的桌面开发**（必须）
     - 可选组件：
       - ✅ MSVC v143 - VS 2022 C++ x64/x86 生成工具
       - ✅ Windows 10 SDK / Windows 11 SDK
       - ✅ C++ CMake 工具
       - ✅ 用于 Windows 的 C++ CMake 工具
   - ✅ **.NET 桌面开发**（如果需要 C# 脚本工具）
3. **安装位置**：建议保持默认（`C:\Program Files\Microsoft Visual Studio\2022\Community`）
4. **验证安装**：打开"Developer Command Prompt for VS 2022"，运行 `cl`（应显示 Microsoft C/C++ 编译器版本）

### 1.2 CMake 安装

1. **下载**：从 [CMake 官网](https://cmake.org/download/) 下载 CMake 3.20+（选择 Windows x64 安装包）
2. **安装**：运行安装程序
   - ✅ 勾选"Add CMake to the system PATH for all users"（添加到 PATH）
   - 安装位置：建议保持默认（`C:\Program Files\CMake`）
3. **验证安装**：打开新的命令提示符，运行 `cmake --version`（应显示 CMake 版本）

### 1.3 MinHook 下载与配置

MinHook 是本项目使用的 API Hook 框架（MIT 协议）。

#### 方法一：自动下载（推荐）
运行构建脚本 `build_all.bat`，脚本会自动下载 MinHook 到 `third_party/minhook/`。

#### 方法二：手动下载
1. **下载**：从 [MinHook GitHub](https://github.com/TsudaKageyu/minhook) 下载最新 Release（如 `minhook-1.3.3.zip`）
2. **解压**：解压到 `third_party/minhook/` 目录
3. **编译**（如果需要）：
   ```bat
   cd third_party/minhook/build
   cmake ..
   msbuild minhook.sln /p:Configuration=Release
   ```

#### 注意：协议兼容性风险
MinHook 使用 MIT 协议，本项目使用 GPL v3 协议。静态链接 MinHook 可能存在协议冲突风险。
- **POC/MVP 阶段**：使用 MinHook，等待法律确认
- **若冲突**：切换到自研 Inline Hook 或 Microsoft Detours

### 1.4 Python 3.x 安装

Python 用于运行项目脚本（如自动化测试、代码生成等）。

1. **下载**：从 [Python 官网](https://www.python.org/downloads/) 下载 Python 3.8+（选择 Windows 安装包）
2. **安装**：
   - ✅ 勾选"Add Python to PATH"
   - ✅ 勾选"Install pip"
3. **验证安装**：打开命令提示符，运行 `python --version`（应显示 Python 版本）

### 1.5 其他依赖

以下依赖由 CMake 自动下载或从 `third_party/` 目录引用：
- **zlib**：用于 .vapp 包压缩
- **RapidJSON**：用于 JSON 解析（元数据）
- **(Electron)**：用于 Launchpad GUI（MVP 阶段决定）

---

## 2. 编译指南

### 2.1 命令行编译（推荐）

#### 方法一：使用 build_all.bat（最简单）
```bat
# 克隆仓库
git clone https://github.com/anmingwei/ai-thinApp-portable-lauchpad-platform.git
cd ai-thinApp-portable-lauchpad-platform

# 运行一键构建脚本（自动下载依赖、生成项目、编译）
.\build_all.bat
```

`build_all.bat` 会自动完成以下步骤：
1. 检查 CMake、Visual Studio 是否安装
2. 下载 MinHook、zlib 等第三方依赖
3. 创建 `build/` 目录，运行 CMake 生成项目文件
4. 编译项目（Release x64）

#### 方法二：手动 CMake 编译
```bat
# 克隆仓库
git clone https://github.com/anmingwei/ai-thinApp-portable-lauchpad-platform.git
cd ai-thinApp-portable-lauchpad-platform

# 创建构建目录
mkdir build
cd build

# 生成项目文件（Visual Studio 2022, x64）
cmake .. -G "Visual Studio 17 2022" -A x64

# 编译（Release）
msbuild ai-thinapp.sln /p:Configuration=Release /p:Platform=x64

# 编译（Debug）
msbuild ai-thinapp.sln /p:Configuration=Debug /p:Platform=x64
```

### 2.2 Visual Studio IDE 编译

1. **生成项目文件**：
   ```bat
   # 方法一：运行 build_all.bat（会自动生成 .sln 文件）
   .\build_all.bat

   # 方法二：手动 CMake 生成
   mkdir build
   cd build
   cmake .. -G "Visual Studio 17 2022" -A x64
   ```
2. **打开项目**：在 `build/` 目录中找到 `ai-thinapp.sln`，双击打开（或右键 → 打开方式 → Visual Studio 2022）
3. **选择配置**：
   - 配置：Debug / Release
   - 平台：x64 / Win32
4. **编译**：菜单栏 → 生成 → 生成解决方案（或按 `Ctrl+Shift+B`）
5. **输出目录**：
   - Debug：`build\src\engine\Debug\`
   - Release：`build\src\engine\Release\`

### 2.3 编译选项说明

| 配置 | 平台 | 说明 |
|------|------|------|
| Debug | x64 | 调试版本，包含调试信息，优化关闭，用于开发调试 |
| Debug | Win32 | 调试版本（32 位），用于测试 32 位应用兼容性 |
| Release | x64 | 发布版本，优化开启，调试信息去除，用于生产环境 |
| Release | Win32 | 发布版本（32 位），用于生产环境（32 位应用） |

### 2.4 编译故障排除

#### 错误："MinHook.h" 找不到
- **原因**：MinHook 未正确下载或路径未配置
- **解决**：
  1. 手动下载 MinHook，解压到 `third_party/minhook/`
  2. 检查 `CMakeLists.txt` 中 MinHook 的路径配置

#### 错误：CMake 找不到 Visual Studio
- **原因**：Visual Studio 未安装或环境变量未配置
- **解决**：
  1. 确认已安装"使用 C++ 的桌面开发"工作负载
  2. 打开"Developer Command Prompt for VS 2022"运行 CMake

#### 错误：链接错误（LNK2019 未解析的外部符号）
- **原因**：某些库未正确链接
- **解决**：
  1. 检查 `CMakeLists.txt` 中的 `target_link_libraries`
  2. 确认第三方依赖已正确编译

---

## 3. 调试指南

### 3.1 使用 Visual Studio 调试

#### 方法一：启动调试（F5）
1. **设置启动项目**：在解决方案资源管理器中，右键点击目标项目（如 `test_hook`）→ 设置为启动项目
2. **设置调试配置**：工具栏 → 配置选择"Debug"，平台选择"x64"
3. **启动调试**：按 `F5`（或菜单栏 → 调试 → 开始调试）
4. **设置断点**：在代码编辑器中，点击行号左侧（或按 `F9`）

#### 方法二：附加到进程
适合调试已启动的应用：
1. **启动应用**（如 `notepad++.exe`）
2. **附加调试器**：Visual Studio → 调试 → 附加到进程
3. **选择进程**：在"可用进程"列表中找到目标进程（如 `notepad++.exe`）
4. **选择代码类型**：勾选"本机"，点击"附加"
5. **设置断点**：在 Hook DLL 源码中设置断点（需加载符号文件）

### 3.2 使用 x64dbg / x32dbg 调试 Hook 引擎

x64dbg（64 位）和 x32dbg（32 位）是用户态调试器，适合调试 Hook 引擎的底层逻辑。

#### 使用步骤
1. **下载**：从 [x64dbg 官网](https://x64dbg.com/) 下载并安装
2. **打开可执行文件**：x64dbg → File → Open → 选择目标程序（如 `test_hook.exe`）
3. **设置断点**：
   - 在模块（如 `hook_engine.dll`）中找到目标函数（如 `InstallHook`）
   - 右键 → Breakpoint → Toggle（或按 `F2`）
4. **运行**：按 `F9`（运行），程序会在断点处暂停
5. **单步调试**：
   - `F7`：单步进入（Step Into）
   - `F8`：单步跳过（Step Over）
6. **查看内存/寄存器**：x64dbg 界面中可查看寄存器、内存、堆栈等

#### 调试 Hook 引擎的技巧
- **在 Hook 函数中设置断点**：当应用调用被 Hook 的 API 时，调试器会中断
- **查看参数**：在断点处，查看寄存器（`RCX`, `RDX`, `R8`, `R9` 对应第 1-4 个参数）
- **修改参数**：可以手动修改寄存器或内存，改变 API 行为

### 3.3 使用 Process Monitor 监视文件/注册表操作

Process Monitor (ProcMon) 是 Sysinternals 套件中的工具，可以实时监视文件系统、注册表、进程/线程活动。

#### 使用步骤
1. **下载**：从 [Microsoft Learn](https://learn.microsoft.com/en-us/sysinternals/downloads/procmon) 下载 Process Monitor
2. **运行**：以管理员权限运行 `Procmon.exe`
3. **过滤事件**：
   - 菜单栏 → Filter → Filter (Ctrl+L)
   - 添加过滤条件：
     - Process Name → `contains` → `notepad++` → `Include`
     - Operation → `is` → `WriteFile` → `Include`
4. **查看事件**：主界面显示符合条件的事件，包括：
   - Time of Day（时间）
   - Process Name（进程名）
   - PID（进程 ID）
   - Operation（操作类型）
   - Path（文件路径/注册表路径）
   - Result（结果）
5. **查看调用栈**：双击某个事件 → Stack（调用栈，可查看是哪个 DLL/函数发起的操作）

#### 使用 ProcMon 验证 Hook 重定向
1. **设置过滤**：过滤目标进程（如 `notepad++.exe`）
2. **执行文件写入操作**：在应用中保存文件
3. **查看路径**：在 ProcMon 中查看 `WriteFile` 操作的 Path 列
   - **未 Hook**：Path 显示真实路径（如 `C:\Windows\Temp\test.txt`）
   - **已 Hook**：Path 显示重定向后的路径（如 `D:\ThinAppApps\Notepad++\VFS\C\Windows\Temp\test.txt`）

### 3.4 日志输出

#### OutputDebugString（Windows 原生）
```cpp
// 在代码中添加日志输出
#include <windows.h>

void InstallHook() {
    OutputDebugStringW(L"[HookEngine] 开始安装 Hook...\n");
    // ...
    if (success) {
        OutputDebugStringW(L"[HookEngine] Hook 安装成功\n");
    } else {
        OutputDebugStringW(L"[HookEngine] Hook 安装失败\n");
    }
}
```

**查看日志**：
- **DebugView**（Sysinternals 工具）：可以实时查看 `OutputDebugString` 的输出
- **Visual Studio 输出窗口**：调试时，日志会显示在"输出"窗口（调试 → 窗口 → 输出）

#### 文件日志（更持久）
```cpp
#include <fstream>
#include <string>

void LogToFile(const std::wstring& message) {
    std::wofstream logFile("hook_engine.log", std::ios::app);
    if (logFile.is_open()) {
        logFile << L"[HookEngine] " << message << std::endl;
    }
}

void InstallHook() {
    LogToFile(L"开始安装 Hook...");
    // ...
}
```

**查看日志**：打开 `hook_engine.log` 文件（在可执行文件同目录）

---

## 4. 代码规范

### 4.1 C++ 编码规范

#### 命名规范
| 类型 | 规范 | 示例 |
|------|------|------|
| 类名 | 大驼峰（PascalCase） | `HookEngine`, `VirtualFileSystem` |
| 函数名 | 小驼峰（camelCase） | `installHook`, `redirectPath` |
| 变量名 | 小驼峰（camelCase） | `hookCount`, `redirectPath` |
| 成员变量 | 小驼峰 + 下划线后缀 | `hook_count_`, `redirect_path_` |
| 常量名 | 全大写下划线 | `MAX_HOOK_COUNT`, `REDIRECT_PATH_LEN` |
| 命名空间 | 全小写下划线 | `thinapp::hook`, `thinapp::vfs` |
| 宏定义 | 全大写下划线 | `HOOK_ENGINE_EXPORT`, `VFS_MAX_PATH` |

#### 注释规范
- **使用中文注释**：解释"为什么"而不是"是什么"
- **函数注释**：使用 Doxygen 风格
  ```cpp
  /**
   * @brief 安装 API Hook
   * @param api_name 目标 API 名称（如 "NtCreateFile"）
   * @param hook_func 钩子函数地址
   * @param original_func 输出参数，保存原始函数地址
   * @return true 表示安装成功，false 表示失败
   * @note 本函数线程安全，但需要提前初始化 MinHook
   */
  bool InstallHook(const char* api_name, void* hook_func, void** original_func);
  ```
- **代码注释**：解释复杂逻辑、算法、 workaround
  ```cpp
  // 为什么用 while 而不是 if？
  // 因为 Hook 可能被其他软件多次覆盖，需要循环检查并重新安装
  while (hook_overritten && retry_count < MAX_RETRY) {
      ReinstallHook();
      retry_count++;
  }
  ```

#### 格式规范
- **缩进**：4 个空格（不用 Tab）
- **大括号风格**：K&R 风格（左大括号不换行）
  ```cpp
  if (condition) {
      do_something();
  } else {
      do_otherthing();
  }
  ```
- **行宽**：不超过 120 字符（方便分屏查看）
- **文件编码**：UTF-8（无 BOM）
- **换行符**：LF（`\n`，Unix 风格）

#### 其他规范
- **RAII 原则**：资源管理使用 RAII（如 `std::unique_ptr`, `std::lock_guard`）
- **避免使用全局变量**：使用依赖注入或单例模式
- **异常处理**：使用 C++ 异常（`try/catch`），但 Hook 函数内避免使用（可能导致兼容性问题）
- **内存管理**：优先使用智能指针，避免裸指针

### 4.2 Git 提交规范

遵循 [Conventional Commits](https://www.conventionalcommits.org/) 规范：

```
<类型>(<范围>): <主题>

[可选正文]

[可选脚注]
```

#### 类型（Type）
| 类型 | 说明 | 示例 |
|------|------|------|
| `feat` | 新功能 | `feat(hook): 添加 NtDeleteFile Hook` |
| `fix` | Bug 修复 | `fix(vfs): 修复长路径重定向失败` |
| `docs` | 文档更新 | `docs(README): 更新快速开始章节` |
| `style` | 代码格式（不影响功能） | `style(engine): 统一缩进为 4 空格` |
| `refactor` | 重构（不是新功能也不是 Bug 修复） | `refactor(vreg): 重构注册表 hive 格式` |
| `perf` | 性能优化 | `perf(hook): 优化 Hook 安装速度` |
| `test` | 测试相关 | `test(engine): 添加 Hook 引擎单元测试` |
| `chore` | 构建/工具链相关 | `chore(build): 更新 CMake 配置` |

#### 范围（Scope）
可选，表示修改的模块：
- `hook`：Hook 引擎
- `vfs`：虚拟文件系统
- `vreg`：虚拟注册表
- `process`：进程隔离
- `packager`：应用打包工具
- `launchpad`：Launchpad 客户端
- `docs`：文档
- `build`：构建系统

#### 主题（Subject）
- 简明扼要，不超过 50 字符
- 使用英文或中文（统一即可，本项目用中文）
- 不以句号结尾

#### 正文（Body）
可选，详细说明：
- 解释"为什么"要做这个修改
- 与之前行为的对比
- 每行不超过 72 字符

#### 脚注（Footer）
可选，关联 Issue、PR：
- `Closes #123`：关闭 Issue #123
- `Refs #456`：关联 Issue #456

#### 示例
```
feat(hook): 添加对 NtDeleteFile 的 Hook 支持

- 在 hook_engine.cpp 中添加 NtDeleteFile Hook
- 更新测试用例 test_hook.cpp
- 更新文档 ARCHITECTURE.md（添加 NtDeleteFile 到 API 列表）

Closes #123
```

### 4.3 代码审查流程

#### 审查清单
审查者应使用以下清单审查代码：
1. **功能正确性**：代码是否实现了预期功能？是否有 Bug？
2. **代码规范**：是否遵循命名、注释、格式规范？
3. **性能**：是否有性能问题（如不必要的拷贝、死循环、低效算法）？
4. **安全性**：是否有安全漏洞（如缓冲区溢出、SQL 注入、命令注入）？
5. **测试**：是否有足够的单元测试？测试是否覆盖边界情况？
6. **文档**：是否有必要的注释？是否更新了相关文档？

#### 审查流程
1. **创建 PR**：开发者创建 Pull Request，填写 PR 模板
2. **自动检查**：CI/CD 自动运行编译、单元测试、代码格式检查
3. **人工审查**：至少 1 名核心贡献者 Review 代码
   - 提出修改意见（行内评论）
   - 批准（Approve）或请求修改（Request Changes）
4. **修改代码**：开发者根据 Review 意见修改代码，然后重新提交 Review
5. **合并**：当所有检查通过且至少 1 名审查者批准后，合并 PR

---

## 5. 添加新功能

### 5.1 如何添加新的 Hook API

#### 步骤
1. **确定目标 API**：
   - 查阅 [Microsoft Docs](https://docs.microsoft.com/) 了解 API 的函数签名、参数、返回值
   - 确定 API 所在的 DLL（如 `ntdll.dll`, `kernel32.dll`, `shell32.dll`）
2. **编写钩子函数**：
   ```cpp
   // 原始函数指针
   static NtCreateFile_t pOriginalNtCreateFile = nullptr;

   // 钩子函数（与原始函数签名一致）
   NTSTATUS WINAPI HookNtCreateFile(
       PHANDLE FileHandle,
       ACCESS_MASK DesiredAccess,
       POBJECT_ATTRIBUTES ObjectAttributes,
       PIO_STATUS_BLOCK IoStatusBlock,
       PLARGE_INTEGER AllocationSize,
       ULONG FileAttributes,
       ULONG ShareAccess,
       ULONG CreateDisposition,
       ULONG CreateOptions,
       PVOID EaBuffer,
       ULONG EaLength) {
       // 1. 检查是否需要重定向
       if (NeedRedirect(ObjectAttributes)) {
           // 2. 修改路径参数（重定向到沙箱路径）
           RedirectObjectAttributes(ObjectAttributes);
       }
       // 3. 调用原始函数
       return pOriginalNtCreateFile(...);
   }
   ```
3. **安装 Hook**：
   ```cpp
   #include <MinHook.h>

   void InstallNtCreateFileHook() {
       // 初始化 MinHook（如果未初始化）
       MH_Initialize();

       // 创建钩子
       MH_CreateHook(
           &NtCreateFile,          // 目标函数地址
           &HookNtCreateFile,      // 钩子函数地址
           (LPVOID*)&pOriginalNtCreateFile  // 保存原始函数地址
       );

       // 启用钩子
       MH_EnableHook(&NtCreateFile);
   }
   ```
4. **卸载 Hook**：
   ```cpp
   void UninstallNtCreateFileHook() {
       MH_DisableHook(&NtCreateFile);
       MH_RemoveHook(&NtCreateFile);
   }
   ```
5. **编写单元测试**：在 `tests/engine/` 中添加测试用例
6. **更新文档**：更新 `ARCHITECTURE.md`（添加 API 到 Hook API 列表）

### 5.2 如何添加新的文件/注册表重定向规则

#### 添加文件系统重定向规则
1. **编辑重定向规则配置文件**（如 `src/engine/vfs/redirect_rules.json`）：
   ```json
   {
     "rules": [
       {
         "real_path": "C:\\Windows\\*",
         "virtual_path": "{AppDir}\\VFS\\C\\Windows\\*"
       },
       {
         "real_path": "C:\\Program Files\\*",
         "virtual_path": "{AppDir}\\VFS\\C\\Program Files\\*"
       }
     ]
   }
   ```
2. **（可选）硬编码规则**：在 `src/engine/vfs/redirect.cpp` 中添加：
   ```cpp
   // 文件系统重定向规则
   const std::vector<RedirectRule> kFileSystemRules = {
       {L"C:\\Windows\\*", L"{AppDir}\\VFS\\C\\Windows\\*"},
       {L"C:\\Program Files\\*", L"{AppDir}\\VFS\\C\\Program Files\\*"},
       // 添加新规则...
   };
   ```
3. **测试重定向**：编写测试用例，验证新规则是否生效

#### 添加注册表重定向规则
1. **编辑重定向规则配置文件**（如 `src/engine/vreg/redirect_rules.json`）：
   ```json
   {
     "rules": [
       {
         "real_path": "HKLM\\Software\\*",
         "virtual_path": "registry.hive\\HKLM\\Software\\*"
       }
     ]
   }
   ```
2. **（可选）硬编码规则**：在 `src/engine/vreg/redirect.cpp` 中添加
3. **测试重定向**：编写测试用例，验证新规则是否生效

### 5.3 如何添加新的测试用例

#### 单元测试（针对函数/类）
1. **选择测试框架**：本项目使用 Google Test（推荐）或 Catch2
2. **创建测试文件**：在 `tests/engine/` 目录下创建 `test_xxx.cpp`
3. **编写测试用例**：
   ```cpp
   #include <gtest/gtest.h>
   #include "hook_engine.h"

   // 测试用例：安装 Hook 成功
   TEST(HookEngineTest, InstallHookSuccess) {
       // 1. 准备（Setup）
       HookEngine engine;

       // 2. 执行（Act）
       bool result = engine.InstallHook("NtCreateFile", HookNtCreateFile);

       // 3. 断言（Assert）
       EXPECT_TRUE(result);
       EXPECT_TRUE(engine.IsHookInstalled("NtCreateFile"));
   }

   // 测试用例：Hook 安装失败（API 不存在）
   TEST(HookEngineTest, InstallHookFail_ApiNotFound) {
       HookEngine engine;
       bool result = engine.InstallHook("NonExistentApi", nullptr);
       EXPECT_FALSE(result);
   }
   ```
4. **编译并运行测试**：
   ```bat
   # 编译测试
   cd build
   msbuild tests\engine\test_hook.vcxproj /p:Configuration=Debug

   # 运行测试
   .\tests\engine\Debug\test_hook.exe
   ```

#### 集成测试（针对模块交互）
1. **创建测试文件**：在 `tests/integration/` 目录下创建 `test_xxx.cpp`
2. **编写测试用例**：
   ```cpp
   #include <gtest/gtest.h>
   #include "hook_engine.h"
   #include "vfs.h"

   // 测试用例：Hook 文件系统 API 后，文件写入被重定向
   TEST(IntegrationTest, FileWriteRedirected) {
       // 1. 安装 Hook
       HookEngine engine;
       engine.InstallHook("NtCreateFile", HookNtCreateFile);
       engine.InstallHook("NtWriteFile", HookNtWriteFile);

       // 2. 创建文件（真实路径）
       HANDLE hFile = CreateFileW(
           L"C:\\Windows\\Temp\\test.txt",
           GENERIC_WRITE,
           0,
           nullptr,
           CREATE_ALWAYS,
           FILE_ATTRIBUTE_NORMAL,
           nullptr
       );

       // 3. 验证文件是否被重定向到沙箱目录
       EXPECT_NE(hFile, INVALID_HANDLE_VALUE);
       // ...（检查文件是否出现在 {AppDir}\VFS\C\Windows\Temp\test.txt）

       CloseHandle(hFile);
   }
   ```
3. **编译并运行测试**（需要管理员权限，因为需要注入 Hook DLL）

---

## 6. 常见问题

### 6.1 编译错误："MinHook.h" 找不到

**原因**：MinHook 未正确下载或路径未配置

**解决方法**：
1. **手动下载 MinHook**：
   - 从 [MinHook GitHub](https://github.com/TsudaKageyu/minhook) 下载最新 Release
   - 解压到 `third_party/minhook/` 目录
2. **检查 CMake 配置**：
   - 打开 `CMakeLists.txt`，确认 MinHook 路径正确
   - 示例：
     ```cmake
     # MinHook 路径
     set(MINHOOK_DIR "${CMAKE_SOURCE_DIR}/third_party/minhook")
     include_directories(${MINHOOK_DIR}/include)
     ```
3. **重新运行 CMake**：
   ```bat
   rmdir /s /q build
   mkdir build
   cd build
   cmake ..
   ```

### 6.2 运行时错误：Hook 安装失败

**原因**：
1. Hook 引擎未正确初始化（MinHook 未初始化）
2. 目标 API 不存在（函数名错误、DLL 未加载）
3. 权限不足（需要注入到其他进程，需要管理员权限）

**解决方法**：
1. **检查 MinHook 初始化**：
   ```cpp
   // 在安装 Hook 之前，确保已调用 MH_Initialize()
   if (MH_Initialize() != MH_OK) {
       OutputDebugStringW(L"MinHook 初始化失败\n");
       return false;
   }
   ```
2. **检查目标 API 地址**：
   ```cpp
   // 使用 GetProcAddress 获取 API 地址
   HMODULE hNtdll = GetModuleHandleW(L"ntdll.dll");
   if (!hNtdll) {
       OutputDebugStringW(L"ntdll.dll 未加载\n");
       return false;
   }

   void* pNtCreateFile = GetProcAddress(hNtdll, "NtCreateFile");
   if (!pNtCreateFile) {
       OutputDebugStringW(L"NtCreateFile 未找到\n");
       return false;
   }
   ```
3. **以管理员权限运行**：
   - 右键点击可执行文件 → 以管理员身份运行
   - 在 Visual Studio 中，项目属性 → 链接器 → 清单文件 → UAC 执行级别 → 设为"requireAdministrator"

### 6.3 兼容性错误：应用 X 无法运行

**原因**：
1. 应用使用直接 Syscall（绕过 ntdll.dll，导致 Hook 失效）
2. 应用有反调试/反 Hook 机制
3. Hook 逻辑有 Bug（如死循环、内存泄漏）
4. 注册表/文件系统重定向不完整

**解决方法**：
1. **检查应用是否使用直接 Syscall**：
   - 使用 x64dbg 调试应用，查看是否直接调用 `syscall` 指令
   - 如果是，POC 阶段接受此限制；V2 阶段增加内核 Minifilter 驱动
2. **查看日志**：
   - 使用 DebugView 查看 `OutputDebugString` 输出的日志
   - 查看文件日志（`hook_engine.log`）
3. **简化 Hook 逻辑**：
   - 暂时禁用某些 Hook，排查是哪个 Hook 导致的问题
   - 使用 Printf 调试（在钩子函数中添加 `printf` 输出）
4. **参考兼容性规则库**：
   - 查看 `docs/COMPATIBILITY.md`（待创建），了解已知兼容性问题
   - 为应用 X 添加自定义规则（白名单/黑名单）

### 6.4 其他常见问题

#### 问题：编译速度慢
- **解决**：
  1. 使用预编译头文件（PCH）
  2. 使用并行编译（`msbuild /m`）
  3. 升级硬件（SSD、更多内存）

#### 问题：Visual Studio IntelliSense 报错（但编译通过）
- **解决**：
  1. 删除 `.vs/` 目录，重新打开项目
  2. 菜单栏 → 工具 → 选项 → 文本编辑器 → C/C++ → 高级 → 禁用 IntelliSense，然后重新启用

#### 问题：Git 行尾符问题（LF/CRLF）
- **解决**：
  1. 在项目根目录创建 `.gitattributes` 文件：
     ```
     * text=auto
     *.cpp text
     *.h text
     *.md text
     *.bat text eol=crlf
     ```
  2. 重新规范化行尾符：
     ```bat
     git add --renormalize .
     git commit -m "chore: 规范化行尾符"
     ```

---

## 附录

### A. 有用的资源

- **MinHook 文档**：[https://github.com/TsudaKageyu/minhook](https://github.com/TsudaKageyu/minhook)
- **Microsoft Docs (Win32 API)**：[https://docs.microsoft.com/windows/win32/api/](https://docs.microsoft.com/windows/win32/api/)
- **x64dbg 文档**：[https://x64dbg.com/docs/](https://x64dbg.com/docs/)
- **Process Monitor 教程**：[https://learn.microsoft.com/en-us/sysinternals/downloads/procmon](https://learn.microsoft.com/en-us/sysinternals/downloads/procmon)
- **Google Test 文档**：[https://google.github.io/googletest/](https://google.github.io/googletest/)

### B. 联系方式

- **GitHub Issues**：[https://github.com/anmingwei/ai-thinApp-portable-lauchpad-platform/issues](https://github.com/anmingwei/ai-thinApp-portable-lauchpad-platform/issues)
- **GitHub Discussions**：[https://github.com/anmingwei/ai-thinApp-portable-lauchpad-platform/discussions](https://github.com/anmingwei/ai-thinApp-portable-lauchpad-platform/discussions)

---

**最后更新**：2026-05-23  
**维护者**：AI ThinApp Portable Launchpad Platform 开源社区

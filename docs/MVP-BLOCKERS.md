# MVP 阻塞问题

**创建时间**: 2026-05-23 18:40  
**报告工程师**: Windows C++ 构建工程师

---

## 阻塞问题 1: 源代码有严重 bug，导致编译失败

**严重程度**: 🔴 阻塞（无法继续构建）

**发现时间**: 2026-05-23 18:32

**影响范围**: Hook DLL 编译（MVP 阶段核心组件）

**问题描述**:

尝试编译 Hook DLL 时，遇到 100+ 个编译错误，主要原因是源代码有严重的组织错误和命名空间使用错误。

### 具体 bug 列表

#### 1. 头文件组织错误

**文件**: `src/engine/hook/hook_engine.h`

**错误 1.1**: 类型定义顺序错误
- `CAPTURE_EVENT_TYPE` 和 `CAPTURE_RECORD` 在第 315-337 行定义
- 但在第 125/156/186 行就被使用了
- **C++ 要求类型在使用前必须定义**

**错误 1.2**: `extern "C"` 块位置错误
- 第 8-11 行的 `extern "C" {` 包含了 C++ 代码（命名空间、类定义）
- `extern "C"` 只能用于 C 函数声明，不能包含 C++ 的命名空间和类
- 导致 C 函数声明（`HookEngine_GetCaptureRecords` 等）被包裹在 `extern "C"` 中，但使用了 C++ 命名空间中的类型（`CAPTURE_RECORD`）

**错误 1.3**: 缺少必要的头文件
- 使用了 `std::vector`（第 186 行），但没有 `#include <vector>`

#### 2. 命名空间使用错误

**文件**: `src/engine/hook/hook_engine.h` 和 `src/engine/hook/hook_engine.cpp`

**错误 2.1**: 前向声明创建了不同的类型
- `hook_engine.h` 中在 `ThinApp::Engine::Hook` 命名空间中使用 `class VfsCache;` 前向声明
- 但 `VfsCache` 实际定义在 `ThinApp::Engine::VFS` 命名空间中
- 这导致 `std::unique_ptr<class VfsCache>`（在 `Hook` 命名空间中）与 `std::unique_ptr<ThinApp::Engine::VFS::VfsCache>`（在 `VFS` 命名空间中）是**不同的类型**，无法互相赋值

**错误 2.2**: 源文件中的类型使用不一致
- `hook_engine.cpp` 中使用了 `ThinApp::Engine::VFS::VfsCache` 和 `ThinApp::Engine::VFS::PathRedirect`
- 但 `hook_engine.h` 中声明的是 `std::unique_ptr<class VfsCache>`（前向声明在 `Hook` 命名空间中）
- 导致类型不匹配编译错误

#### 3. 静态成员函数定义错误

**文件**: `src/engine/hook/hook_engine.cpp`

**错误 3.1**: 静态成员函数未定义或名称错误
- `HookEngine` 类中声明了静态成员函数（如 `HookCheckThreadProc`）
- 但定义时可能使用了错误的名称（如 `HookCheckThreadProc_Static`）
- 或者定义为非静态成员函数
- 导致 "identifier not found" 错误

#### 4. const 正确性错误

**文件**: `src/engine/hook/hook_engine.cpp`

**错误 4.1**: `EnterCriticalSection` 和 `LeaveCriticalSection` 需要非常量 `CRITICAL_SECTION*`
- 但可能在 const 成员函数中调用，导致 `this` 指针是 const，从而 `capture_records_lock_` 也是 const
- 无法将 `const CRITICAL_SECTION*` 转换为 `LPCRITICAL_SECTION`
- **解决方案**: 将 `capture_records_lock_` 声明为 `mutable`

---

### 尝试过的修复

#### 修复 1: 调整类型定义顺序（部分成功）

**修改内容**:
1. 将 `CAPTURE_EVENT_TYPE` 和 `CAPTURE_RECORD` 的定义移动到 `HookEngine` 类定义之前
2. 添加 `#include <vector>`
3. 修正 `extern "C"` 块的位置

**结果**: 仍然失败

**原因**: 
- C 函数 `HookEngine_GetCaptureRecords` 现在可以看到了 `CAPTURE_RECORD` 的定义
- 但是 `CAPTURE_RECORD` 定义在 `ThinApp::Engine::Hook` 命名空间中
- C 函数无法访问 C++ 命名空间中的类型
- 需要将 `CAPTURE_EVENT_TYPE` 和 `CAPTURE_RECORD` 的定义移动到**全局命名空间**中

#### 修复 2: 修正命名空间问题（未进行）

**原因**: 这需要修改多个头文件和源文件，涉及项目架构，超出了构建工程师的职责范围。

---

### 建议解决方案

#### 方案 1: 开发团队修复源代码 bug（推荐）

**负责人**: 开发团队

**工作量**: 中等（2-4 小时）

**步骤**:
1. 修正 `hook_engine.h` 中的类型定义顺序
2. 将 `CAPTURE_EVENT_TYPE` 和 `CAPTURE_RECORD` 的定义移动到全局命名空间中（在 `extern "C"` 块之前）
3. 在 `hook_engine.h` 中添加 `#include "vfs/vfs_cache.h"` 和 `#include "vfs/path_redirect.h"`，替换前向声明 `class VfsCache;` 和 `class PathRedirect;`
4. 修正 `hook_engine.cpp` 中的静态成员函数定义，确保名称与头文件中的声明一致
5. 将 `capture_records_lock_` 声明为 `mutable`，以修复 const 正确性问题

#### 方案 2: 使用条件编译绕过错误（临时方案）

**负责人**: 构建工程师

**工作量**: 大（4-8 小时）

**步骤**:
1. 使用宏定义或条件编译来"修补"头文件
2. 在编译命令中添加 `-D` 选项来定义宏，绕过某些错误
3. 这只是一个临时方案，不能解决根本问题

**不推荐原因**: 
- 只是掩盖了问题，没有真正修复 bug
- 可能导致运行时错误
- 增加了维护成本

---

## 下一步行动

### 立即需要完成的任务

1. **开发团队修复源代码 bug**
   - 修正 `hook_engine.h` 中的类型定义顺序和 `extern "C"` 块
   - 修正 `VfsCache` 和 `PathRedirect` 的命名空间问题
   - 修正 `hook_engine.cpp` 中的静态成员函数定义
   - 修正 const 正确性问题

2. **构建工程师继续构建环境搭建**（等待源代码修复）
   - 安装 CMake
   - 安装 vcpkg
   - 使用 vcpkg 安装依赖（MinHook、nlohmann/json、zlib）
   - 更新 CMakeLists.txt 以正确引用 vcpkg 依赖
   - 重新尝试编译

### 阻塞解除条件

- [ ] `hook_engine.h` 中的类型定义顺序已修正
- [ ] `extern "C"` 块已正确放置
- [ ] `VfsCache` 和 `PathRedirect` 的命名空间问题已修正
- [ ] `hook_engine.cpp` 可以成功编译（0 个错误）

---

## 上报给 PM

**上报时间**: 2026-05-23 18:40

**上报内容**:

PM 您好，

我是 Windows C++ 构建工程师，负责 AI ThinApp Portable Launchpad Platform 项目的 MVP 阶段构建环境搭建。

我在尝试编译 Hook DLL 时遇到了**阻塞性问题**：源代码有严重的 bug，导致编译失败（100+ 个错误）。

主要问题包括：
1. 头文件组织错误（类型定义顺序错误、`extern "C"` 块位置错误、缺少头文件）
2. 命名空间使用错误（`VfsCache` 和 `PathRedirect` 的命名空间不匹配）
3. 静态成员函数定义错误
4. const 正确性问题

我已经尝试修复了部分问题（调整了类型定义顺序、添加了 `#include <vector>`、修正了 `extern "C"` 块的位置），但仍然无法编译通过。

**这些问题需要开发团队来修复**，因为涉及项目架构和源代码组织，超出了构建工程师的职责范围。

我已详细记录了编译错误和尝试过的修复，见：
- `docs/MVP-BUILD-LOG.md`（构建日志）
- `docs/MVP-BLOCKERS.md`（本文档）

**建议 PM**：
1. 将源代码 bug 分配给开发团队修复
2. 修复完成后，通知我继续构建环境搭建
3. 或者，如果时间紧迫，可以先使用临时方案（如条件编译）绕过某些错误，但需要在后续 sprint 中安排时间彻底修复

等待您的指示。

---

**文档结束**

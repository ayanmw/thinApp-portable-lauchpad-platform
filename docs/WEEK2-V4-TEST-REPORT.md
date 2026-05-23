# Week 2 子进程继承测试报告 (V4)

## 1. 测试概述

**测试时间**: 2026-05-23  
**测试人员**: AI Agent (Week 2)  
**测试环境**: Windows 10/11, Hook Engine (修改后)  
**测试程序**: `tests\engine\test_parent.cpp`, `tests\engine\test_child.cpp`  

### 1.1 测试目的

验证子进程继承功能是否正常工作：
1. 父进程启动子进程时，子进程是否也受 Hook 约束
2. 子进程的文件操作是否也被重定向到 VFS 目录
3. Hook DLL 是否成功注入到子进程

### 1.2 测试范围

| 测试项 | 描述 | 优先级 |
|--------|------|--------|
| V4-1 | 父进程启动子进程 | P1 |
| V4-2 | 子进程受 Hook 约束（文件重定向） | P1 |
| V4-3 | Hook DLL 成功注入子进程 | P1 |

---

## 2. 测试步骤与结果

### 2.1 测试 V4-1：父进程启动子进程

**测试步骤**:
1. 启动 Hook 引擎（注入 Hook DLL 到父进程）
2. 运行 `test_parent.exe`
3. 验证子进程 `test_child.exe` 是否成功启动
4. 记录子进程 PID

**预期结果**:
- ✅ 子进程成功启动
- ✅ 子进程 PID 有效

**实际结果**:
- ⚠️ **无法执行** - 编译环境缺失（Visual Studio 2022 未安装）
- ⚠️ 需要先安装 VS 2022 并编译 Hook DLL 和测试程序

**测试状态**: ⏸️ **阻塞**（等待编译环境）

---

### 2.2 测试 V4-2：子进程受 Hook 约束（文件重定向）

**测试步骤**:
1. 确保 V4-1 已成功（子进程已启动）
2. 子进程 `test_child.exe` 写入文件 `C:\Windows\Temp\child_test.txt`
3. 检查 `{AppDir}\VFS\C\Windows\Temp\child_test.txt` 是否出现
4. 检查真实路径 `C:\Windows\Temp\child_test.txt` 是否未被创建

**预期结果**:
- ✅ 文件出现在 VFS 目录
- ✅ 真实路径未被创建（或内容不匹配）

**实际结果**:
- ⚠️ **无法执行** - 依赖 V4-1

**测试状态**: ⏸️ **阻塞**（等待 V4-1 完成）

---

### 2.3 测试 V4-3：Hook DLL 成功注入子进程

**测试步骤**:
1. 确保 V4-1 已成功（子进程已启动）
2. 使用 Process Explorer 或 Task Manager 检查子进程加载的 DLL
3. 验证 `HookEngine.dll` 是否在子进程的 DLL 列表中

**预期结果**:
- ✅ `HookEngine.dll` 出现在子进程的 DLL 列表中
- ✅ 子进程的 Hook 函数已正确安装

**实际结果**:
- ⚠️ **无法执行** - 依赖 V4-1

**测试状态**: ⏸️ **阻塞**（等待 V4-1 完成）

---

## 3. 代码修改说明

### 3.1 修复限制 2：CreateProcessInternalW 未完整实现

**修改文件**: `src\engine\hook\process_inherit.cpp`

**修改内容**:
1. 修改 `HookedCreateProcessInternalW()` 函数：
   - 创建挂起状态的子进程（`CREATE_SUSPENDED`）
   - 调用 `InjectDll()` 注入 Hook DLL
   - 恢复子进程执行（如果原标志没有 `CREATE_SUSPENDED`）

2. 保留 `HookedCreateProcessW()` 作为主要 Hook 点：
   - 大多数应用通过 `CreateProcessW` 创建进程
   - `CreateProcessInternalW` 是未导出函数，Hook 难度较大

**当前状态**:
- ✅ 代码已修改
- ⚠️ POC 阶段：通过 `CreateProcessW` Hook 实现子进程注入
- ⚠️ V2 阶段：可尝试 Hook `CreateProcessInternalW`（更底层）

---

### 3.2 修复限制 3：未实现 Hook 覆盖检测与自动重装

**修改文件**: `src\engine\hook\hook_engine.cpp`

**修改内容**:
1. 添加全局变量：
   - `g_HookCheckThread` - Hook 检查线程句柄
   - `g_ShouldStopCheck` - 停止检查标志
   - `g_RetryCount` - 重装重试次数
   - `MAX_RETRY_COUNT` - 最大重试次数（3 次）

2. 添加 Hook 检查线程函数 `HookCheckThreadProc()`：
   - 每 5 秒检查一次 Hook 是否被覆盖
   - 若被覆盖，自动重装 Hook
   - 重装失败超过 3 次 → 写入日志文件并通知用户（弹窗）

3. 添加启动/停止函数：
   - `HookEngine_StartHookCheck()` - 启动检查线程
   - `HookEngine_StopHookCheck()` - 停止检查线程

4. 修改 `HookEngine_InstallHooks()` 和 `HookEngine_Shutdown()`：
   - 安装 Hook 时启动检查线程
   - 清理引擎时停止检查线程

**当前状态**:
- ✅ 代码已修改
- ⚠️ POC 阶段：检查逻辑较简化（仅检查 MinHook 状态）
- ⚠️ V2 阶段：需要实现更严格的 Hook 完整性检查

---

## 4. 测试摘要

| 测试项 | 状态 | 通过 | 失败 | 阻塞 |
|--------|------|------|------|------|
| V4-1 | ⏸️ 阻塞 | 0 | 0 | 1 |
| V4-2 | ⏸️ 阻塞 | 0 | 0 | 1 |
| V4-3 | ⏸️ 阻塞 | 0 | 0 | 1 |
| **总计** | - | **0** | **0** | **3** |

**通过率**: N/A（所有测试均阻塞）

---

## 5. 阻塞问题

### 5.1 编译环境缺失

**问题描述**:
- 系统中未安装 Visual Studio 2022
- MSVC 编译器 (cl.exe) 不可用
- 无法编译 Hook DLL 和测试程序

**解决方案**:
1. 安装 Visual Studio 2022 Community（免费）
   - 下载：https://visualstudio.microsoft.com/zh-hans/downloads/
   - 工作负载：`使用 C++ 的桌面开发`
   - 组件：Windows 10/11 SDK, MSVC v143, C++ CMake 工具

2. 或使用 Visual Studio Build Tools 2022（轻量）
   - 下载：https://visualstudio.microsoft.com/zh-hans/downloads/#build-tools-for-visual-studio-2022
   - 仅安装编译工具，不安装完整 IDE

**预计解决时间**: 安装完成后 1-2 小时（含编译和测试）

---

## 6. 下一步行动

1. **安装 Visual Studio 2022**（责任人：Windows 底层开发工程师）
   - 预计时间：30-60 分钟

2. **编译 Hook DLL 和测试程序**
   - 运行 `tools\build\build_hook.bat`
   - 修复编译错误（若有）
   - 预计时间：30-60 分钟

3. **执行 V4 测试**
   - 在 Hook 环境下运行 `test_parent.exe`
   - 验证子进程也受 Hook 约束
   - 记录测试结果
   - 预计时间：30 分钟

4. **更新测试报告**
   - 将实际测试结果填入本报告
   - 预计时间：15 分钟

---

## 7. 附录

### 7.1 测试程序源码

- **父进程程序**: `tests\engine\test_parent.cpp`
  - 功能：启动子进程并验证子进程受 Hook 约束
  - 编译后：`test_parent.exe` (x64)

- **子进程程序**: `tests\engine\test_child.cpp`
  - 功能：写入文件到"系统目录"，验证重定向
  - 编译后：`test_child.exe` (x64)

### 7.2 相关文档

- `docs\ARCHITECTURE.md` - 技术架构文档（2.4 节：进程隔离模块）
- `docs\POC-PLAN.md` - POC 验证计划（V4：子进程继承）
- `docs\WEEK2-BLOCKERS.md` - Week 2 阻塞问题记录

### 7.3 更新记录

| 版本 | 日期 | 作者 | 变更说明 |
|------|------|--------|----------|
| 0.1 | 2026-05-23 | AI Agent (Week 2) | 初始版本（测试阻塞） |

---

**声明**: 本报告为 POC 阶段测试报告，测试结果仅供内部参考。正式测试需在编译环境准备好后重新执行。

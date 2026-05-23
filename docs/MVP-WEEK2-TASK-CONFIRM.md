# MVP Week 2 任务理解确认

> 本文档确认对 AI ThinApp Portable Launchpad Platform 项目 MVP Week 2 开发任务的理解。
> 日期：2026-05-23 | 负责人：Windows C++ 底层开发工程师

---

## 1. 任务概述

Week 2 的主要目标是实现 Hook 覆盖检测增强和注册表重定向完善，提升系统的稳定性和功能性。

### 1.1 Hook 覆盖检测增强（Day 1-3）

**目标**：实现多检测点 + 指数退避 + 自动重装，提升 Hook 稳定性

**技术方案**（来自设计文档第4章）：
1. **多检测点**：
   - 定时检测：将检测间隔从5秒缩短为1秒（平衡性能和实时性）
   - 事件驱动检测：
     - 进程创建时检测（在`HookedCreateProcessW`中调用检测逻辑）
     - DLL加载时检测（在`DllMain`中调用检测逻辑）
   - 多检测点冗余：同时启用定时检测和事件驱动检测，提高可靠性

2. **指数退避重试**：
   - 第1次重试：等待1秒
   - 第2次重试：等待2秒
   - 第3次重试：等待4秒
   - 第4次重试：等待8秒
   - 最多重试5次，若仍失败则记录错误并停止重试

3. **自动重装流程**：
   - 调用`MH_DisableHook()`禁用被覆盖的钩子
   - 调用`MH_EnableHook()`重新启用钩子
   - 若失败则触发指数退避重试

**性能目标**：
- Hook覆盖检测延迟 < 1秒（事件驱动检测）
- 自动重装成功率 ≥ 95%
- 检测线程CPU占用 < 1%

### 1.2 注册表重定向完善（Day 4-5）

**目标**：在 POC 阶段透传模式基础上，实现完整的注册表重定向

**技术方案**：
1. **CoW（Copy-on-Write）重定向**：
   - 读取时优先查虚拟 Hive，不存在则透传真实注册表
   - 写入时只修改虚拟 Hive

2. **Hive 持久化**：
   - `SaveHive()`：将内存中的 Hive 数据保存到 `{AppDir}\VFS\registry.hive`
   - `LoadHive()`：从 `{AppDir}\VFS\registry.hive` 加载 Hive 数据到内存

3. **集成到 Hook 函数**：
   - 修改`HookedNtCreateKey()`：调用`HiveManager::CoWRedirect()`实现重定向
   - 修改`HookedNtSetValueKey()`：调用`HiveManager::CoWRedirect()`实现重定向
   - 修改`HookedNtOpenKey()`：调用`HiveManager::CoWRedirect()`实现重定向
   - 修改`HookedNtQueryValueKey()`：调用`HiveManager::CoWRedirect()`实现重定向

**性能目标**（设计文档第6章）：
- 注册表重定向延迟（P50）< 0.5ms
- 注册表重定向延迟（P95）< 2ms
- 注册表重定向延迟（P99）< 5ms

---

## 2. 开发任务拆解

### 2.1 Hook 覆盖检测增强（Day 1-3）

| 天数 | 任务 | 产出 |
|------|------|------|
| Day 1 | 修改`hook_engine.h/cpp`，添加成员变量和方法声明 | `src/engine/hook/hook_engine.h/cpp` |
| Day 2 | 实现`CheckHookCoverage()`和`ReloadHook()`方法 | `src/engine/hook/hook_engine.cpp` (更新) |
| Day 3 | 集成Hook覆盖检测到`vreg_engine.cpp`，编写测试用例 | `src/engine/hook/vreg_engine.cpp`、`tests/engine/test_hook_cover.cpp` |

### 2.2 注册表重定向完善（Day 4-5）

| 天数 | 任务 | 产出 |
|------|------|------|
| Day 4 | 修改`hive_manager.h/cpp`，添加`CoWRedirect()`、`SaveHive()`、`LoadHive()`方法 | `src/engine/vreg/hive_manager.h/cpp` |
| Day 5 | 集成CoW重定向到Hook函数，编写测试用例 | `src/engine/hook/vreg_engine.cpp`、`tests/engine/test_vreg_redirect.cpp` |

---

## 3. 验收标准

### 3.1 Hook 覆盖检测增强

- ✅ Hook 覆盖检测延迟 < 1 秒（定时检测）
- ✅ Hook 自动重装成功率 ≥ 95%（模拟覆盖 100 次，统计重装成功次数）
- ✅ 指数退避延迟符合 2^n 规律（n=0,1,2,3,4,5，最大 60 秒）
- ✅ 重装失败超过 3 次 → 写入日志 `logs\hook_cover.log` 并弹窗通知用户
- ✅ 单元测试覆盖率 ≥ 80%

### 3.2 注册表重定向完善

- ✅ 注册表重定向正确率 = 100%（测试 100 个注册表操作，全部重定向到虚拟 Hive）
- ✅ CoW 逻辑正确率 = 100%（读取透传、写入重定向）
- ✅ Hive 持久化正确率 = 100%（卸载 DLL 后重新加载，虚拟注册表值不变）
- ✅ 单元测试覆盖率 ≥ 80%

---

## 4. 技术要点

### 4.1 Hook 覆盖检测实现要点

1. **使用`std::thread` + `std::atomic`实现检测线程**（C++11 标准）
2. **定时检测**：在`CheckHookCoverage()`方法中，使用`std::this_thread::sleep_for(std::chrono::seconds(1))`实现1秒间隔
3. **事件驱动检测**：在 Hook 函数入口处调用`CheckHookCoverage()`
4. **指数退避**：使用`std::this_thread::sleep_for(std::chrono::seconds(delay))`，其中`delay = std::min(1 << failed_count_, 60)`

### 4.2 注册表重定向实现要点

1. **CoW 逻辑**：
   - 读取时：先查虚拟 Hive，若不存在则调用真实注册表 API
   - 写入时：只修改虚拟 Hive，不修改真实注册表
2. **Hive 持久化**：
   - 使用轻量级格式（如JSON或自定义二进制格式）存储 Hive 数据
   - 在`DllMain()`的`DLL_PROCESS_DETACH`中调用`SaveHive()`
   - 在`DllMain()`的`DLL_PROCESS_ATTACH`中调用`LoadHive()`
3. **线程安全**：使用互斥锁保护 Hive 数据（读写锁`SRWLOCK`）

---

## 5. 风险与依赖

### 5.1 依赖

- 依赖MinHook库（版本≥1.3.3）
- 依赖Windows Registry API（如`RegCreateKeyW`、`RegSetValueExW`等）

### 5.2 风险

| 风险描述 | 影响程度 | 缓释措施 |
|----------|----------|----------|
| Hook覆盖检测误报率高 | 低 | 增加检测次数阈值，避免单次检测失败就触发重装 |
| Hive文件IO开销大 | 中 | 使用内存缓存，延迟写入 |
| 注册表重定向逻辑复杂，易出错 | 高 | 增加单元测试覆盖率，使用模糊测试 |

### 5.3 阻塞记录

若存在技术难点（如Hook覆盖检测误报），立即记录到`docs/MVP-BLOCKERS.md`并上报PM。

---

## 6. 下一步行动

1. 开始实现 Hook 覆盖检测增强（Day 1-3）
2. 实现注册表重定向完善（Day 4-5）
3. 编写测试用例，确保单元测试覆盖率 ≥ 80%
4. 输出 Week 2 完成报告

---

## 7. 附录

### 7.1 参考资料

- 设计文档：`docs/MVP-HOOK-ENGINE-DESIGN.md`（第4章 Hook覆盖检测增强设计）
- MinHook官方文档：https://github.com/TsudaKageyu/minhook
- Windows Registry API文档：https://docs.microsoft.com/en-us/windows/win32/api/winreg/

### 7.2 版本历史

| 版本 | 日期 | 作者 | 变更内容 |
|------|------|------|----------|
| 1.0 | 2026-05-23 | Windows C++ 底层开发工程师 | 初始版本，确认Week 2任务理解 |

# MVP Week 3 任务理解确认

> 确认日期：2026-05-23 | 负责人：Windows C++ 底层开发工程师

---

## 1. 任务概述

Week 3 主要完成两个核心功能模块：
1. **子进程继承完善**：支持 5 层子进程继承，处理 WOW64 场景，提升注入成功率 ≥ 99%
2. **性能监控指标实现**：实现 6 类性能监控指标，采集 P50/P95/P99 分位数

---

## 2. 子进程继承完善设计（对应设计文档第 5 章）

### 2.1 技术方案

#### 5 层子进程继承
- 使用 `CreateRemoteThreadEx()` 替代 `CreateRemoteThread()`，支持更多参数（如 WOW64 场景）
- 在 `HookedCreateProcessW` 中，为新创建的子进程注入 Hook DLL
- 子进程的 Hook DLL 加载时，继续为其创建的孙进程注入 Hook DLL
- 递归支持最多 5 层继承

#### WOW64 场景处理
- 调用 `IsWow64Process()` 检测进程位数
- 注入策略：
  - 父进程 32 位 + 子进程 64 位 → 使用 64 位 Hook DLL (`hook_engine_64.dll`)
  - 父进程 64 位 + 子进程 32 位 → 使用 32 位 Hook DLL (`hook_engine_32.dll`)
  - 同位数进程 → 使用相同位数的 Hook DLL
- 同时提供 32 位和 64 位 Hook DLL

#### 异常处理
- 捕获 `CreateRemoteThreadEx()` 失败错误
- 记录错误日志到 `logs\inject.log`（包含进程 ID、错误码）
- 返回错误码给父进程，不崩溃父进程
- 继承深度限制：超过 5 层后不再注入，避免无限递归

### 2.2 验收标准

| 指标 | 目标值 |
|------|--------|
| 单层继承注入成功率 | = 100% (测试 100 次) |
| 多层继承注入成功率 | ≥ 99% (测试 5 层，每层 100 次) |
| WOW64 场景注入成功率 | ≥ 95% (测试 100 次，32 位子进程) |
| 注入失败错误处理 | 正确记录错误日志 |
| 继承层数限制 | 第 6 层未被注入 |
| 单元测试覆盖率 | ≥ 80% |

---

## 3. 性能监控指标设计（对应设计文档第 6 章）

### 3.1 监控指标列表

| 指标类型 | 指标名称 | 计算方式 | 目标值 |
|----------|----------|----------|--------|
| Hook 安装/卸载 | Hook 安装耗时 | 从调用 `HookEngine_InstallHooks()` 到返回的时间 | < 100ms |
| | Hook 卸载耗时 | 从调用 `HookEngine_UninstallHooks()` 到返回的时间 | < 50ms |
| 文件重定向 | 文件重定向延迟 (P50) | 50% 的文件操作延迟低于此值 | < 1ms |
| | 文件重定向延迟 (P95) | 95% 的文件操作延迟低于此值 | < 5ms |
| | 文件重定向延迟 (P99) | 99% 的文件操作延迟低于此值 | < 10ms |
| 注册表重定向 | 注册表重定向延迟 (P50) | 50% 的注册表操作延迟低于此值 | < 0.5ms |
| | 注册表重定向延迟 (P95) | 95% 的注册表操作延迟低于此值 | < 2ms |
| | 注册表重定向延迟 (P99) | 99% 的注册表操作延迟低于此值 | < 5ms |
| 子进程注入 | 子进程注入耗时 (P50) | 50% 的子进程注入耗时低于此值 | < 50ms |
| | 子进程注入耗时 (P95) | 95% 的子进程注入耗时低于此值 | < 100ms |
| | 子进程注入耗时 (P99) | 99% 的子进程注入耗时低于此值 | < 200ms |
| 缓存性能 | VFS 缓存命中率 | 命中次数 / 总访问次数 | ≥ 80% |
| | 路径重定向缓存命中率 | 命中次数 / 总访问次数 | ≥ 90% |

### 3.2 技术实现方案

#### 性能监控类设计
- **单例模式**：`PerfMonitor` 类，全局唯一实例
- **数据存储**：`std::map<std::string, std::vector<uint64_t>>` 存储性能指标
- **线程安全**：使用 `SRWLOCK` 实现读写锁（C++17 标准）
- **分位数计算**：使用 `nth_element` 算法（O(n) 平均复杂度）

#### 埋点位置
- `HookedCreateFileW` 入口和返回处 → 记录文件重定向延迟
- `HookedRegSetValueExW` 入口和返回处 → 记录注册表重定向延迟
- `InjectHookDll()` 入口和返回处 → 记录子进程注入延迟
- `VFS Cache Get()` 方法 → 记录缓存命中率

#### 日志采集
- PowerShell 脚本 `tools/ci/collect_perf_logs.ps1`：定期采集性能指标（每天凌晨 2:00）
- 输出格式：JSON (`perf_logs\2026-05-23.json`)
- 上传到 GitHub Actions artifact（保留 30 天）

### 3.3 验收标准

| 指标 | 目标值 |
|------|--------|
| 性能指标记录正确率 | = 100% (记录 1000 个值，验证正确性) |
| 分位数计算正确率 | = 100% (P50/P95/P99 计算正确) |
| 重置功能正确率 | = 100% (调用 `Reset()` 后，指标已清空) |
| 线程安全 | 10 线程并发记录，无崩溃/死锁，运行 10 分钟 |
| 缓存命中率计算正确率 | = 100% (模拟访问，验证命中率计算正确) |
| 单元测试覆盖率 | ≥ 80% |

---

## 4. 开发计划（Day 1-5）

### Day 1-3：子进程继承完善

| 天数 | 任务 | 产出 |
|------|------|------|
| Day 1 | 修改 `process_inherit.h/cpp`，实现 `InjectHookDll()` 和 `IsWow64Process()` | `src/engine/hook/process_inherit.h/cpp` |
| Day 2 | 修改 `HookedCreateProcessW()`，支持 5 层继承和 WOW64 处理 | `src/engine/hook/process_inherit.cpp` (更新) |
| Day 3 | 编写单元测试 `test_process_inherit.cpp` | `tests/engine/test_process_inherit.cpp` |

### Day 4-5：性能监控指标实现

| 天数 | 任务 | 产出 |
|------|------|------|
| Day 4 | 创建 `perf_monitor.h/cpp`，实现性能监控类 | `src/engine/common/perf_monitor.h/cpp` |
| Day 4 | 修改 `hook_engine.cpp` 和 `vfs_cache.cpp`，集成性能监控 | `src/engine/hook/hook_engine.cpp`、`src/engine/vfs/vfs_cache.cpp` |
| Day 5 | 编写单元测试 `test_perf_monitor.cpp` 和 PowerShell 脚本 | `tests/engine/test_perf_monitor.cpp`、`tools/ci/collect_perf_logs.ps1` |

---

## 5. 关键注意事项

### 5.1 编码规范
- 所有文件使用 **UTF-8 编码（无 BOM）**
- 代码注释用**中文**

### 5.2 技术难点
- `CreateRemoteThreadEx` 在 WOW64 场景下的兼容性
- `SRWLOCK` 读写锁的正确使用（避免死锁）
- `nth_element` 算法的正确实现（处理边界情况）

### 5.3 阻塞上报
- 若遇到技术难点（如 `CreateRemoteThreadEx` 注入失败率高），立即记录到 `docs/MVP-BLOCKERS.md` 并上报 PM

---

## 6. 交付物清单

| 文件 | 路径 | 状态 |
|------|------|------|
| 任务理解确认 | `docs/MVP-WEEK3-TASK-CONFIRM.md` | ✅ 已完成 |
| 修改后的 process_inherit | `src/engine/hook/process_inherit.h/cpp` | 待完成 |
| 修改后的 hook_engine | `src/engine/hook/hook_engine.cpp` | 待完成 |
| 性能监控头文件 | `src/engine/common/perf_monitor.h` | 待完成 |
| 性能监控实现 | `src/engine/common/perf_monitor.cpp` | 待完成 |
| 子进程继承测试 | `tests/engine/test_process_inherit.cpp` | 待完成 |
| 性能监控测试 | `tests/engine/test_perf_monitor.cpp` | 待完成 |
| 性能日志采集脚本 | `tools/ci/collect_perf_logs.ps1` | 待完成 |
| Week 3 完成报告 | `docs/MVP-WEEK3-COMPLETE-REPORT.md` | 待完成 |

---

## 7. 确认声明

本人已完整阅读并理解 `docs/MVP-HOOK-ENGINE-DESIGN.md` 中的"子进程继承完善设计"和"性能监控指标设计"章节，明确 Week 3 的所有开发任务、技术方案、验收标准和注意事项。

**确认人**：Windows C++ 底层开发工程师  
**确认日期**：2026-05-23  
**准备开始开发**：是 ✅

# MVP 性能基准测试准备 - 任务理解确认

## 项目信息

- **项目名称**: AI ThinApp Portable Launchpad Platform
- **项目目录**: D:\anmw_work_proj\ai-thinApp-portable-launchpad-platform
- **当前状态**: MVP 开发阶段已完成（10 个 Week 代码级交付），编译错误尚未验证通过（22 个错误已修复，等待用户手动编译验证）
- **角色**: 性能测试工程师

## MVP 性能目标（从任务描述提取）

| 性能指标 | 目标值 | 说明 |
|---------|--------|------|
| 启动速度 | < 3 秒 | 从双击 launchpad.exe 到主窗口显示 |
| 内存占用 | < 200 MB | 启动后空闲状态 |
| UI 响应 | < 100 ms | 点击按钮到响应 |
| Hook 开销 | < 5% CPU | Hook 引擎额外 CPU 占用 |
| 文件重定向延迟 | < 1 ms | 单次文件操作 |
| 注册表重定向延迟 | < 0.5 ms | 单次注册表操作 |

## 任务范围

### 第一步：读取 MVP 性能目标和已有测试
- [x] 读取 `docs/MVP-PLAN.md` - 已完成，获取了 MVP 计划和当前状态
- [ ] 读取 `docs/MVP-PERF-STARTUP.md` - 文件不存在
- [ ] 读取 `docs/MVP-PERF-MEMORY.md` - 文件不存在
- [ ] 读取 `docs/MVP-PERF-UI.md` - 文件不存在
- [ ] 读取 `docs/SPEC.md` - 文件不存在
- [ ] 读取 `src/engine/common/perf_monitor.h` - 文件不存在，需要创建
- [ ] 读取 `src/engine/common/perf_monitor.cpp` - 文件不存在，需要创建
- [ ] 读取 `tests/launchpad/test_startup_speed.cpp` - 需要检查
- [ ] 读取 `tests/launchpad/test_memory_usage.cpp` - 需要检查
- [ ] 读取 `tests/launchpad/test_ui_responsiveness.cpp` - 需要检查

**结论**: 性能监控模块尚未创建，需要从头创建。

### 第二步：完善性能监控模块
- [ ] 创建/修改 `src/engine/common/perf_monitor.h`
  - [ ] 添加性能指标枚举：StartupTime、MemoryUsage、UIResponseTime、HookOverheadCPU、FileRedirectLatency、RegRedirectLatency
  - [ ] 添加采集方法：RecordStartupTime()、RecordMemoryUsage()、RecordUIResponseTime()、RecordHookOverheadCPU()、RecordFileRedirectLatency()、RecordRegRedirectLatency()
  - [ ] 添加导出方法：ExportToJSON()
- [ ] 创建/修改 `src/engine/common/perf_monitor.cpp`
  - [ ] 实现所有采集方法（使用 Windows API）
  - [ ] 实现导出方法（使用 nlohmann/json 库）
- [ ] 创建/修改 `src/engine/hook/hook_engine.cpp`
  - [ ] 在 DllMain() 的 DLL_PROCESS_ATTACH 中调用 PerfMonitor::Instance().RecordStartupTime()
  - [ ] 在 Hook 函数入口调用 PerfMonitor::Instance().RecordHookOverheadCPU()
  - [ ] 在文件/注册表重定向完成后调用 PerfMonitor::Instance().RecordFileRedirectLatency() / RecordRegRedirectLatency()
- [ ] 创建/修改 `src/launchpad/ui/main_window.cpp`
  - [ ] 在 initUI() 完成后调用 PerfMonitor::Instance().RecordStartupTime()
  - [ ] 在 UI 事件处理函数入口调用 PerfMonitor::Instance().RecordUIResponseTime()

### 第三步：创建性能基准测试脚本
- [ ] 创建 `tests/performance/test_startup_benchmark.bat` - 启动速度基准测试
- [ ] 创建 `tests/performance/test_memory_benchmark.bat` - 内存占用基准测试
- [ ] 创建 `tests/performance/test_ui_responsiveness.bat` - UI 响应基准测试
- [ ] 创建 `tests/performance/test_hook_overhead.bat` - Hook 开销基准测试

### 第四步：创建性能数据分析脚本
- [ ] 创建 `tests/performance/analyze_performance.py` - Python 性能数据分析脚本
- [ ] 创建 `tests/performance/performance_report_template.html` - HTML 报告模板

### 第五步：输出性能基准测试准备报告
- [ ] 创建 `docs/MVP-PERF-BENCHMARK-REPORT.md` - 性能基准测试准备报告

## 已知限制

1. **编译错误尚未验证通过**: 22 个错误已修复，等待用户手动编译验证
2. **无法实际运行性能测试**: 由于编译错误未验证，无法实际运行测试
3. **所有测试为桩实现**: 需要标记 TODO 注释，等待编译通过后实际运行

## 验收标准

1. ✅ 性能指标数量 = 6 个（启动速度、内存占用、UI 响应、Hook 开销、文件延迟、注册表延迟）
2. ✅ 性能数据采集方法实现率 = 100%
3. ✅ 性能基准测试脚本数量 = 4 个
4. ✅ 性能数据分析脚本完成度 = 100%（Python 脚本 + HTML 模板）
5. ✅ 所有文件使用 UTF-8 编码（无 BOM）
6. ✅ 所有文件使用 `qclaw-text-file` 技能写入

## 注意事项

1. **必须使用 `qclaw-text-file` 技能写入所有文件**（禁止直接使用内置 `write` 工具）
2. 所有文件使用 UTF-8 编码（无 BOM）（注：根据 qclaw-text-file 技能规则，Windows 上 .cpp/.h 文件默认使用 UTF-8 无 BOM）
3. 代码注释用中文
4. 若遇到阻塞（如编译错误未修复），立即记录到 `docs/MVP-BLOCKERS.md` 并上报 PM
5. 优先保证性能监控模块正确，再创建测试脚本
6. 性能基准测试脚本使用批处理（`.bat`）
7. 性能数据分析脚本使用 Python（`.py`）
8. **关键**：所有测试为桩实现（TODO 注释标记），等待编译通过后实际运行

## 下一步行动

1. 创建 `src/engine/common/` 目录（如果不存在）
2. 创建性能监控模块（perf_monitor.h 和 perf_monitor.cpp）
3. 创建测试脚本目录（tests/performance/）
4. 创建 4 个性能基准测试脚本
5. 创建性能数据分析脚本和 HTML 模板
6. 输出性能基准测试准备报告

---

**确认时间**: 2026-05-23
**确认人**: 性能测试工程师（子代理）
**状态**: 理解完成，开始执行

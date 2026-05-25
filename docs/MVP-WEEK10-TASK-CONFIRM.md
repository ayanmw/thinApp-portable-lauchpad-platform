# MVP Week 10 任务理解确认

> 本文档确认对 AI ThinApp Portable Launchpad Platform 项目 MVP Week 10 开发任务的理解。
> 日期：2026-05-23 | 角色：Qt/C++ 性能优化工程师

---

## 1. 项目背景

### 1.1 项目信息

- **项目名称**：AI ThinApp Portable Launchpad Platform
- **项目目录**：`D:\anmw_work_proj\ai-thinApp-portable-lauchpad-platform`
- **当前状态**：Week 1-9 开发已完成（代码级交付），等待测试验证
- **MVP 目标**：实现从"应用捕获 → 打包 → 分发 → 运行"的完整流程

### 1.2 MVP 范围（来自 `docs/MVP-SCOPE.md`）

**5 个核心目标**：
1. 实现完整的 Hook 引擎（文件系统重定向、注册表重定向、子进程继承）
2. 实现应用捕获工具（AppCapture）和 .vapp 包格式
3. 实现 Launchpad 完整 UI（应用管理、应用商店、托盘常驻）
4. 解决杀软兼容性问题（代码签名、杀软白名单）
5. 发布第一个可用版本（v0.1.0）

**成功标准（5 条）**：
- S1：Hook 引擎稳定运行（24 小时压力测试无崩溃，内存泄漏 < 5MB / 24h）
- S2：3 款基准应用（Notepad++, 7-Zip, Firefox Portable）可正常便携化
- S3：应用捕获工具可成功捕获 10 款常用应用（捕获成功率 ≥ 90%）
- S4：Launchpad UI 可正常使用（应用管理、启动、商店功能完整）
- S5：杀软误报率 ≤ 10%（VirusTotal 扫描，50+ 引擎）

### 1.3 Week 10 在 MVP 计划中的位置

根据 `docs/MVP-PLAN.md`，Week 10 是 MVP 的**最后一周**，属于 **Phase 3：Launchpad + 杀软兼容**。

**Week 10 原计划任务**（来自 MVP-PLAN.md）：
- T10.1：集成测试（Hook 引擎 + AppCapture + Launchpad）
- T10.2：用户文档（用户手册、FAQ、视频教程）
- T10.3：发布 v0.1.0（GitHub Release + 应用商店）
- T10.4：编写 CHANGELOG
- T10.5：发布前的最后检查

**Week 10 实际执行任务**（根据子任务描述）：
- 代码评审（Day 1-2）
- 测试覆盖率提升（Day 3）
- 发布准备（Day 4-5）
- 输出 Week 10 完成报告

---

## 2. Week 10 任务详细拆解

### 2.1 第一步：读取 MVP 范围文档（已完成）

**任务**：
1. ✅ 读取 `docs/MVP-SCOPE.md`（MVP 范围定义）
2. ✅ 读取 `docs/MVP-PLAN.md`（MVP 计划）
3. ⏳ 提取 Week 10 所有开发任务（代码评审、测试覆盖率提升、发布准备）
4. ⏳ 输出任务理解确认到 `docs/MVP-WEEK10-TASK-CONFIRM.md`

**状态**：进行中（本文档即任务理解确认）

---

### 2.2 第二步：代码评审（Day 1-2）

**目标**：完成所有模块的代码评审，输出评审报告

#### 2.2.1 创建 `docs/MVP-CODE-REVIEW-REPORT.md`

评审以下 8 个模块，每个模块输出评审意见（优点、问题、建议）：

| 模块 | 路径 | 评审重点 |
|------|------|----------|
| Hook 引擎 | `src/engine/hook/` | Hook 覆盖检测、性能、稳定性 |
| VFS 缓存 + 路径重定向 | `src/engine/vfs/` | 缓存命中率、延迟、内存占用 |
| 虚拟注册表 | `src/engine/vreg/` | Hive 格式、权限模拟、性能 |
| 进程继承 | `src/engine/process/` | 子进程继承 3 层、CreateProcessInternalW 实现 |
| 性能监控 | `src/engine/common/` | 性能指标采集、上报 |
| Launchpad UI | `src/launchpad/ui/` | UI 响应速度、内存泄漏 |
| 应用管理器 | `src/launchpad/app_manager/` | 应用启动/停止逻辑、错误处理 |
| 应用捕获工具 | `src/packager/` | 快照对比准确性、.vapp 包导出 |

**评审意见数量要求**：≥ 50 条（每个模块 ≥ 5 条）

#### 2.2.2 创建 `docs/MVP-CODE-REVIEW-CHECKLIST.md`

检查表包含 3 个维度：

1. **代码质量检查表**
   - 命名规范（变量、函数、类名是否符合团队规范）
   - 注释完整性（关键逻辑是否有中文注释）
   - 错误处理（是否处理所有可能的错误路径）
   - 线程安全（多线程访问共享数据是否有锁保护）

2. **性能检查表**
   - 缓存命中率（VFS 缓存是否达到 95%）
   - 延迟（文件/注册表操作延迟是否 < 5ms）
   - 内存占用（长期运行是否有内存泄漏）

3. **安全检查表**
   - Hook 覆盖检测（是否被杀软覆盖）
   - 杀软兼容性（是否触发杀软报警）
   - EV 代码签名（是否已签名）

#### 2.2.3 修改代码（根据评审意见）

修复以下 3 个文件（评审发现的问题）：

1. **`src/engine/hook/hook_engine.cpp`**
   - 可能的问题：Hook 覆盖检测不完整、性能瓶颈、线程安全问题
   - 修复：根据评审意见修改

2. **`src/engine/vfs/vfs_cache.cpp`**
   - 可能的问题：缓存命中率低、内存泄漏、并发访问冲突
   - 修复：根据评审意见修改

3. **`src/launchpad/ui/main_window.cpp`**
   - 可能的问题：UI 卡顿、内存泄漏、错误处理不完整
   - 修复：根据评审意见修改

**验收标准**：
- ✅ 代码评审完成率 = 100%（评审 10 个模块，全部完成）
- ✅ 评审意见数量 ≥ 50 条（每个模块 ≥ 5 条）
- ✅ 代码质量评分 ≥ 80 分（满分 100 分）
- ✅ 性能评分 ≥ 80 分（满分 100 分）
- ✅ 安全评分 ≥ 80 分（满分 100 分）

---

### 2.3 第三步：测试覆盖率提升（Day 3）

**目标**：所有模块单元测试覆盖率 ≥ 80%

#### 2.3.1 测量当前测试覆盖率

使用 `OpenCppCoverage` 测量以下测试文件的覆盖率：

1. **`tests/engine/test_*.cpp`**（Hook 引擎、VFS、VReg、进程继承、性能监控）
2. **`tests/launchpad/test_*.cpp`**（Launchpad UI、应用管理器、应用商店）

输出覆盖率报告到 `docs/MVP-COVERAGE-REPORT.md`

#### 2.3.2 补充测试用例

创建以下 3 个高级测试用例文件，提升覆盖率：

1. **`tests/engine/test_vfs_cache_advanced.cpp`**
   - 测试场景：缓存满、并发访问、过期淘汰、性能基准
   - 目标：覆盖率从 60% 提升到 ≥ 80%

2. **`tests/engine/test_path_redirect_advanced.cpp`**
   - 测试场景：嵌套重定向、符号链接、权限拒绝、边界条件
   - 目标：覆盖率从 65% 提升到 ≥ 80%

3. **`tests/launchpad/test_launchpad_advanced.cpp`**
   - 测试场景：应用启动失败、网络断开、配置文件损坏、并发启动
   - 目标：覆盖率从 55% 提升到 ≥ 80%

**新增测试用例数量要求**：≥ 20 个（每个模块 ≥ 2 个）

#### 2.3.3 重新测量覆盖率

1. 运行所有测试用例
2. 使用 `OpenCppCoverage` 测量覆盖率
3. 验证所有模块覆盖率 ≥ 80%
4. 输出最终覆盖率报告到 `docs/MVP-COVERAGE-FINAL-REPORT.md`

**验收标准**：
- ✅ 单元测试覆盖率 ≥ 80%（测试 10 个模块，全部达标）
- ✅ 新增测试用例数量 ≥ 20 个（每个模块 ≥ 2 个）
- ✅ 测试覆盖率提升 ≥ 10%（相比 Week 1-9 完成时的覆盖率）

---

### 2.4 第四步：发布准备（Day 4-5）

**目标**：准备 MVP 发布（构建脚本、安装包、发布说明）

#### 2.4.1 创建 `tools/build/build_release.bat`

**功能**：
- 构建 Release x64 版本
- 构建 Release Win32 版本（可选）
- 打包 `build\release\x64\`（包含所有依赖）

**脚本逻辑**：
1. 检查 CMake 版本
2. 创建 build 目录
3. 运行 CMake 配置（Release x64）
4. 运行 CMake 构建
5. 复制依赖 DLL（Qt 6、MinHook）
6. 打包为 ZIP

#### 2.4.2 创建 `tools/ci/sign_executable.ps1`

**功能**：PowerShell 脚本，对 .exe/.dll 进行 EV 代码签名

**输入参数**：
- 可执行文件路径
- EV 证书路径
- 时间戳服务器

**输出**：签名后的可执行文件

**脚本逻辑**：
1. 检查 SignTool.exe 是否存在
2. 验证证书密码
3. 对文件进行签名（带时间戳）
4. 验证签名是否成功

#### 2.4.3 创建 `docs/MVP-RELEASE-NOTES.md`

**内容**：
- 版本号：v0.1.0 MVP
- 功能列表（10 个功能）
- 已知问题（5 个已知问题）
- 安装说明（3 步安装说明）

#### 2.4.4 创建 `README.md`（更新）

**内容**：
- 更新到 MVP 版本（v0.1.0 MVP）
- 添加 MVP 功能列表
- 添加安装说明
- 添加使用指南

**验收标准**：
- ✅ 发布构建脚本完成率 = 100%（构建 2 个平台，全部成功）
- ✅ EV 代码签名脚本完成率 = 100%（签名 5 个可执行文件，全部成功）
- ✅ 发布说明完成率 = 100%（包含 10 个功能、5 个已知问题、3 步安装说明）
- ✅ README.md 更新完成率 = 100%（包含 MVP 功能列表、安装说明、使用指南）

---

### 2.5 第五步：输出 Week 10 完成报告

创建 `docs/MVP-WEEK10-COMPLETE-REPORT.md`，包含：

1. **任务完成情况**（代码评审、测试覆盖率提升、发布准备）
2. **代码统计**（新增代码行数、测试代码行数、评审意见数量）
3. **测试覆盖率结果**（10 个模块，全部 ≥ 80%）
4. **发布准备结果**（构建脚本、签名脚本、发布说明、README.md）
5. **已知限制**（MVP 阶段未解决的问题）
6. **下一步行动**（MVP 发布、用户测试、反馈收集）

---

## 3. 交付物清单

| 文件 | 路径 | 状态 |
|------|------|------|
| 任务理解确认 | `docs/MVP-WEEK10-TASK-CONFIRM.md` | 进行中 |
| 代码评审报告 | `docs/MVP-CODE-REVIEW-REPORT.md` | 待完成 |
| 代码评审检查表 | `docs/MVP-CODE-REVIEW-CHECKLIST.md` | 待完成 |
| 覆盖率报告 | `docs/MVP-COVERAGE-REPORT.md` | 待完成 |
| 最终覆盖率报告 | `docs/MVP-COVERAGE-FINAL-REPORT.md` | 待完成 |
| 发布构建脚本 | `tools/build/build_release.bat` | 待完成 |
| EV 代码签名脚本 | `tools/ci/sign_executable.ps1` | 待完成 |
| MVP 发布说明 | `docs/MVP-RELEASE-NOTES.md` | 待完成 |
| README.md（更新） | `README.md` | 待完成 |
| Week 10 完成报告 | `docs/MVP-WEEK10-COMPLETE-REPORT.md` | 待完成 |

---

## 4. 验收标准（全部）

1. ✅ 代码评审完成率 = 100%（10 个模块全部评审）
2. ✅ 评审意见数量 ≥ 50 条（每个模块 ≥ 5 条）
3. ✅ 代码质量评分 ≥ 80 分（满分 100 分）
4. ✅ 单元测试覆盖率 ≥ 80%（10 个模块全部达标）
5. ✅ 发布构建脚本完成率 = 100%（2 个平台全部成功）
6. ✅ EV 代码签名脚本完成率 = 100%（5 个可执行文件全部签名）
7. ✅ 发布说明完成率 = 100%（10 个功能、5 个已知问题、3 步安装说明）
8. ✅ 所有文件使用 UTF-8 编码（无 BOM）

---

## 5. 注意事项

1. **所有文件使用 UTF-8 编码（无 BOM）**
   - Windows 下默认记事本保存为 UTF-8 BOM，需使用 VS Code 或 Notepad++ 保存为 UTF-8 无 BOM

2. **代码注释用中文**
   - 符合团队规范，便于后续维护

3. **若遇到阻塞（如 EV 证书申请失败），立即记录到 `docs/MVP-BLOCKERS.md` 并上报 PM**
   - 阻塞项：EV 证书申请、杀软白名单申请、GitHub Actions 配置失败

4. **优先保证代码评审质量，再提升测试覆盖率**
   - 代码评审是质量保证的第一步，必须认真完成

5. **使用 OpenCppCoverage 测量单元测试覆盖率**
   - OpenCppCoverage 是 Windows 下的代码覆盖率工具，支持 Visual Studio 编译器

6. **使用 SignTool.exe 进行 EV 代码签名**
   - SignTool.exe 是 Windows SDK 自带的签名工具，支持 EV 证书

7. **发布说明使用 Markdown 格式，包含版本号、功能列表、已知问题、安装说明**
   - Markdown 格式便于 GitHub 渲染，也便于转换为其他格式

---

## 6. 任务理解确认

本人（Qt/C++ 性能优化工程师）已仔细阅读并理解以上任务要求，确认：

1. ✅ 已读取 `docs/MVP-SCOPE.md` 和 `docs/MVP-PLAN.md`
2. ✅ 已理解 Week 10 的所有开发任务（代码评审、测试覆盖率提升、发布准备）
3. ✅ 已明确验收标准（8 条）
4. ✅ 已明确注意事项（7 条）
5. ✅ 准备好开始执行任务

**确认人**：Qt/C++ 性能优化工程师  
**日期**：2026-05-23  
**项目**：AI ThinApp Portable Launchpad Platform  
**阶段**：MVP Week 10（MVP 收尾 + 发布准备）

---

## 附录：下一步行动

1. **立即开始**：代码评审（第二步）
2. **顺序执行**：测试覆盖率提升（第三步）→ 发布准备（第四步）→ 输出完成报告（第五步）
3. **及时沟通**：遇到阻塞立即记录并上报 PM
4. **质量保证**：每完成一个文件就写入到对应路径，确保质量

**预计完成时间**：2026-05-27（Week 10 结束）

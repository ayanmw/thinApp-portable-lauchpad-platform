# 文档一致性校验报告

生成时间：2026-05-23

## 校验概述

本次校验对以下8个核心文档进行了逐一比对：
- README.md（根目录）
- docs/SPEC.md
- docs/ARCHITECTURE.md
- docs/POC-PLAN.md
- docs/RISK-REGISTER.md
- docs/UX-RESEARCH.md
- docs/UX-DESIGN.md
- docs/SANDBOX-VISUAL-IDENTITY.md

---

## 冲突列表

| 文档 A | 文档 B | 冲突描述 | 严重程度 | 建议修复 |
|---------|---------|----------|----------|----------|
| ARCHITECTURE.md | RISK-REGISTER.md | ARCHITECTURE.md 将 MinHook 列为第三方依赖（MIT 协议），但 RISK-REGISTER.md R-TECH-01 指出 MinHook 与 GPL v3 存在协议冲突风险 | 高 | 等待 PM 确认协议兼容性后，统一修改 ARCHITECTURE.md 的依赖列表和 RISK-REGISTER.md 的缓解措施 |

---

## 遗漏列表

| 文档 | 遗漏内容 | 建议补充到 | 优先级 |
|------|----------|------------|--------|
| README.md | 快速开始（Quick Start）部分 | README.md | 高 |
| README.md | 详细项目结构说明（每个子目录的用途） | README.md | 高 |
| README.md | 贡献指南（Contributing） | README.md | 中 |
| README.md | 社区链接（GitHub Discussions 等） | README.md | 中 |
| README.md | FAQ 部分 | README.md | 中 |
| ARCHITECTURE.md | 虚拟注册表 hive 格式的最终决策（MVP 用 JSON 还是二进制） | ARCHITECTURE.md 2.3.1 | 中 |
| ARCHITECTURE.md | Launchpad GUI 框架最终决策（Electron 还是 Win32） | ARCHITECTURE.md 3.1 | 中 |
| POC-PLAN.md | 与 RISK-REGISTER.md 相比，缺少技术风险 R-TECH-05 到 R-TECH-10 | POC-PLAN.md | 低 |
| POC-PLAN.md | 缺少产品风险、法律风险、项目风险、外部风险 | POC-PLAN.md | 低 |

---

## 版本不一致列表

| 文档 | 版本号 | 日期 | 与实际代码不一致？ | 说明 |
|------|--------|------|-------------------|------|
| README.md | 无版本号 | 无日期 | 否 | README.md 未标注版本号和日期，建议与 SPEC.md 保持一致 |
| SPEC.md | 0.1 | 2026-05-23 | 否 | 与项目阶段一致（POC 阶段） |
| ARCHITECTURE.md | 0.1 | 2026-05-23 | 否 | 与项目阶段一致 |
| POC-PLAN.md | 0.1 | 2026-05-23 | 否 | 与项目阶段一致 |
| RISK-REGISTER.md | 0.1 | 2026-05-23 | 否 | 与项目阶段一致 |
| UX-RESEARCH.md | 无版本号 | 无日期 | 否 | 建议添加版本号和日期 |
| UX-DESIGN.md | 无版本号 | 无日期 | 否 | 建议添加版本号和日期 |
| SANDBOX-VISUAL-IDENTITY.md | 无版本号 | 无日期 | 否 | 建议添加版本号和日期 |

---

## 一致性校验详情

### 1. README.md vs SPEC.md

| 检查项 | README.md | SPEC.md | 一致？ | 说明 |
|--------|-----------|---------|--------|------|
| 项目定位 | ✅ 一致 | ✅ 一致 | 是 | 都描述了便携化平台的核心目标 |
| 技术架构 | ✅ 一致 | ✅ 一致 | 是 | 架构图与 SPEC.md 3.1 一致 |
| 目录结构 | ✅ 一致 | ✅ 一致 | 是 | 都包含 src/engine, src/packager, src/launchpad, src/sync |
| 开发阶段 | ✅ 一致 | ✅ 一致 | 是 | POC/MVP/体验完善/生态与企业 |
| 构建要求 | ✅ 一致 | N/A | 是 | README.md 提到 VS2022 + CMake 3.20+，与 ARCHITECTURE.md 7.1 一致 |

### 2. SPEC.md vs ARCHITECTURE.md

| 检查项 | SPEC.md | ARCHITECTURE.md | 一致？ | 说明 |
|--------|---------|------------------|--------|------|
| 文件系统虚拟化 API | NtCreateFile, NtWriteFile, NtOpenFile | 2.1.2 列表包含这些 API | 是 | 完全一致 |
| 注册表虚拟化 API | NtCreateKey, NtSetValueKey, NtOpenKey | 2.1.2 列表包含这些 API | 是 | 完全一致 |
| 进程隔离 API | CreateProcessInternalW, ShellExecuteExW | 2.1.2 列表包含这些 API | 是 | 完全一致 |
| Hook 引擎 | 基于 MinHook | 2.1.1 提到 MinHook | 是 | 一致，但存在协议冲突风险（见冲突列表） |
| POC 验证标准 | 7.1 POC 验收标准 V1-V8 | 4.1 验证标准 V1-V8 | 是 | 完全一致 |

### 3. ARCHITECTURE.md vs POC-PLAN.md

| 检查项 | ARCHITECTURE.md | POC-PLAN.md | 一致？ | 说明 |
|--------|------------------|--------------|--------|------|
| Hook 引擎框架 | 2.1 Hook 引擎模块 | 2.1 In Scope #1 | 是 | 都提到基于 MinHook |
| 文件系统重定向 | 2.2 虚拟文件系统模块 | 2.1 In Scope #2 | 是 | 一致 |
| 注册表重定向 | 2.3 虚拟注册表模块 | 2.1 In Scope #3 | 是 | 一致 |
| 子进程继承 | 2.4 进程隔离模块 | 2.1 In Scope #4 | 是 | 一致 |
| 验证标准 | N/A | 4.1 V1-V8 | 是 | ARCHITECTURE.md 未重复验证标准，指向 POC-PLAN.md 即可 |

### 4. POC-PLAN.md vs RISK-REGISTER.md

| 检查项 | POC-PLAN.md | RISK-REGISTER.md | 一致？ | 说明 |
|--------|--------------|-------------------|--------|------|
| 风险数量 | 6 个（R-POC-1 到 R-POC-6） | 17 个（R-TECH-01 到 R-EXT-02） | 部分 | POC-PLAN.md 的风险是 RISK-REGISTER.md 的子集，RISK-REGISTER.md 更全面 |
| 风险 ID 命名 | R-POC-X | R-TECH-XX, R-PROD-XX 等 | 否 | 命名规范不一致，建议统一为 RISK-REGISTER.md 的格式 |
| 风险缓解措施 | 有 | 有 | 是 | 主要内容一致，RISK-REGISTER.md 更详细 |

### 5. UX-RESEARCH.md vs UX-DESIGN.md

| 检查项 | UX-RESEARCH.md | UX-DESIGN.md | 一致？ | 说明 |
|--------|-----------------|---------------|--------|------|
| 简化概念传达 | 建议避免技术术语 | 1.1 首次使用引导流程使用简单语言 | 是 | 一致 |
| 符合原生交互习惯 | 建议遵循 Windows 规范 | 界面线框图遵循 Windows 风格 | 是 | 一致 |
| 强化应用库管理 | 建议提供分类清晰的应用库 | 1.3 应用库管理流程 | 是 | 一致 |
| 可视化沙箱内容 | 建议提供沙箱内容浏览器 | 1.4 沙箱内容浏览流程 | 是 | 一致 |
| 简化配置流程 | 建议提供默认配置 | 1.5 设置流程提供默认配置 | 是 | 一致 |
| 明确视觉区分 | 建议任务栏标识+窗口边框着色 | SANDBOX-VISUAL-IDENTITY.md 推荐相同方案 | 是 | 一致 |

### 6. UX-DESIGN.md vs ARCHITECTURE.md

| 检查项 | UX-DESIGN.md | ARCHITECTURE.md | 一致？ | 说明 |
|--------|---------------|------------------|--------|------|
| Launchpad 主界面 | 2.1 线框图显示应用库视图 | 3.2.1 主界面（应用库） | 是 | 完全一致 |
| 应用详情页 | 2.2 线框图 | 3.2.2 应用详情页 | 是 | 一致 |
| 软件商店 | 2.3 线框图 | 3.2.3 软件商店 | 是 | 一致 |
| 技术选型 | N/A | 3.1 提到 Electron 或 Win32 | 是 | UX-DESIGN.md 的线框图是 Web 风格，与 Electron 决策一致 |

### 7. SANDBOX-VISUAL-IDENTITY.md vs UX-RESEARCH.md/UX-DESIGN.md

| 检查项 | SANDBOX-VISUAL-IDENTITY.md | UX-RESEARCH.md/UX-DESIGN.md | 一致？ | 说明 |
|--------|-----------------------------|--------------------------------|--------|------|
| 视觉区分方案 | 推荐任务栏标识+窗口边框着色 | UX-RESEARCH.md 4.2 建议相同方案 | 是 | 完全一致 |
| 技术实现难度 | 评估了 L1-L6 层级 | N/A | 是 | 补充了技术可行性分析，与 ARCHITECTURE.md 一致 |

---

## 建议改进项

1. **统一版本号管理**：所有文档应标注版本号和日期，建议与 SPEC.md 保持一致（版本号 + 日期 + 作者 + 变更说明）
2. **协议冲突风险**：等待 PM 确认 MinHook 与 GPL v3 的协议兼容性，然后统一修改相关文档
3. **风险 ID 命名规范**：建议统一使用 RISK-REGISTER.md 的命名规范（R-TECH-XX, R-PROD-XX 等）
4. **POC-PLAN.md 风险部分**：建议引用 RISK-REGISTER.md，避免重复维护
5. **ARCHITECTURE.md 未决决策**：建议添加"待决策项"章节，列出 MinHook 协议、hive 格式、GUI 框架等待确认项

---

## 校验结论

✅ **总体一致性：良好**

- 核心功能需求、技术架构、POC 验证标准等关键内容一致
- 存在 1 个高风险冲突（MinHook 协议冲突），需要 PM 确认
- 存在若干遗漏项（主要是 README.md 缺少章节，以及部分文档缺少版本号）
- 建议按照"建议改进项"逐一修复，然后重新校验

---

**校验人**：技术文档工程师（子代理）  
**校验日期**：2026-05-23  
**下一步**：等待 PM 确认冲突项，然后更新不一致的文档

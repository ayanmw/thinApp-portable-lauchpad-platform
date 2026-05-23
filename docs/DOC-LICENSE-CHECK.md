# 协议一致性校验报告 (License Consistency Check)

> 本文档校验项目所有文档中的协议声明是否一致，特别是 GPL v3 许可证与第三方依赖协议（如 MIT）的兼容性。

---

## 1. 协议声明检查表

| 文档 | 协议声明 | 一致？ | 说明 |
|------|----------|--------|------|
| LICENSE | GPL v3 | - | 项目主协议，完整文本 |
| README.md | GPL v3 | ✅ 一致 | 底部徽章 + "许可证"章节明确声明 GPL v3 |
| SPEC.md | GPL v3 | ✅ 一致 | 4.4 节"代码开源（GPL v3）" |
| ARCHITECTURE.md | MinHook（MIT）为依赖 | ❌ 不一致（潜在冲突） | 2.1.1 节提到 MinHook 使用 MIT 协议，与 GPL v3 静态链接可能存在协议冲突 |
| POC-PLAN.md | 未明确声明 | ⚠️ 部分一致 | 未提及协议，但引用了 ARCHITECTURE.md 和 SPEC.md |
| RISK-REGISTER.md | R-TECH-01：MinHook 协议冲突风险 | ✅ 一致（已记录风险） | 技术风险表格中明确记录了协议冲突风险，需 PM 决策 |
| FAQ.md | GPL v3 | ✅ 一致 | Q2 "代码是开源的吗？" 明确回答 GPL v3 |
| DEV-GUIDE.md | 未明确声明 | ⚠️ 部分一致 | 1.3 节"注意：协议兼容性风险"，但未明确项目协议 |
| THIRD-PARTY.md | （缺失） | ❌ 不一致 | 需创建，列出所有依赖及协议 |
| 所有代码文件头部 | 未检查 | ⚠️ 待检查 | 需确认代码文件头部是否有 GPL v3 声明 |

---

## 2. 协议冲突清单

| ID | 文档 | 问题 | 冲突级别 | 解决方案 | 负责人 |
|----|------|------|----------|----------|--------|
| L-01 | ARCHITECTURE.md | MinHook（MIT）与 GPL v3 静态链接冲突 | 高 | 1. 替换为自研 Hook 引擎；2. 获得 MinHook 作者双协议授权（MIT + GPL v3 兼容）；3. 动态链接 MinHook（MIT 允许动态链接） | PM + Legal |
| L-02 | THIRD-PARTY.md | 缺失第三方依赖协议声明 | 中 | 创建 THIRD-PARTY.md，列出所有依赖及协议（MinHook-MIT, zlib-zlib, RapidJSON-MIT, Electron-MIT） | Dev Lead |
| L-03 | DEV-GUIDE.md | 未明确声明项目协议 | 低 | 在文档开头添加协议声明："本项目采用 GPL v3 协议，详见 LICENSE 文件" | Dev Lead |
| L-04 | 所有代码文件 | 未确认是否有 GPL v3 头部声明 | 中 | 检查并添加 GPL v3 头部声明（如需要） | Dev Lead |

---

## 3. 第三方依赖协议检查

### 3.1 已识别的依赖及协议

| 依赖 | 用途 | 协议 | 与 GPL v3 兼容性 | 备注 |
|------|------|------|---------------------|------|
| MinHook | API Hook 框架 | MIT | ⚠️ 需确认 | 静态链接可能导致 GPL v3 传染性问题 |
| zlib | .vapp 包压缩 | zlib | ✅ 兼容 | zlib 协议与 GPL v3 兼容 |
| RapidJSON | JSON 解析（元数据） | MIT | ✅ 兼容 | MIT 与 GPL v3 兼容（动态链接） |
| Electron | Launchpad GUI | MIT | ✅ 兼容 | MIT 与 GPL v3 兼容（运行时依赖） |
| CMake | 构建系统 | BSD | ✅ 兼容 | BSD 与 GPL v3 兼容（构建时依赖） |

### 3.2 协议冲突分析

#### MinHook（MIT）与 GPL v3 的冲突

**问题**：
- GPL v3 要求衍生项目必须开源（传染性）
- 如果静态链接 MinHook（MIT），某些法律观点认为 MinHook 代码也必须开源（但 MinHook 已开源，所以可能无问题）
- 更严格的观点：静态链接后，整个项目被视为"衍生作品"，必须采用 GPL v3（MinHook 的 MIT 协议允许闭源使用，但 GPL v3 要求开源）→ **冲突**

**解决方案**：
1. **替换为自研 Hook 引擎**（RISK-REGISTER.md R-TECH-01 缓解措施 2）
   - 优点：完全规避协议冲突
   - 缺点：开发工作量大，延迟项目进度

2. **获得 MinHook 作者双协议授权**
   - 联系 MinHook 作者，请求授权本项目可同时使用 MIT 和 GPL v3
   - 优点：继续使用 MinHook，无需自研
   - 缺点：作者可能拒绝，或需要付费

3. **动态链接 MinHook**（推荐）
   - 将 MinHook 编译为独立的 DLL，本项目动态链接（运行时加载）
   - MIT 协议允许动态链接，且动态链接不触发 GPL v3 的"衍生作品"定义
   - 优点：继续使用 MinHook，协议合规
   - 缺点：需要修改构建系统，动态链接可能有性能损耗（微小）

4. **使用其他 Hook 框架**（如 Microsoft Detours）
   - Detours 使用 Microsoft EULA，需确认是否允许 GPL v3 项目使用
   - 优点：如果 Detours 协议兼容，可替换 MinHook
   - 缺点：Detours 协议可能不兼容 GPL v3

**建议**：采用方案 3（动态链接 MinHook），并在 THIRD-PARTY.md 中明确说明。

---

## 4. GPL v3 协议要求检查

### 4.1 必须包含的内容

| 要求 | 是否满足 | 说明 |
|------|----------|------|
| 协议文本 | ✅ 满足 | LICENSE 文件包含完整 GPL v3 文本 |
| 协议声明 | ✅ 满足 | README.md、SPEC.md、FAQ.md 明确声明 GPL v3 |
| 版权声明 | ⚠️ 部分满足 | LICENSE 文件有版权声明（Copyright (C) 2024 anmingwei），但代码文件头部未检查 |
| 源代码提供 | ⚠️ 待确认 | 需确认 GitHub 仓库是否包含完整源代码 |

### 4.2 建议添加的内容

| 内容 | 位置 | 说明 |
|------|------|------|
| 协议摘要 | README.md 顶部 | 简要说明 GPL v3 的核心要求（开源、衍生项目必须开源等） |
| 第三方依赖协议 | THIRD-PARTY.md | 列出所有依赖及协议，说明兼容性 |
| 贡献者协议 | CONTRIBUTING.md | 说明贡献者签署 CLA（Contributor License Agreement）的要求（如果需要） |

---

## 5. 校验结论

### 总体评价
- **检查项**：10
- **通过**：6
- **失败**：2
- **待确认**：2
- **通过率**：60%（不含待确认项）

### 主要问题
1. **MinHook 协议冲突**（高优先级）- L-01
   - 问题描述：MinHook（MIT）与 GPL v3 静态链接可能存在协议冲突
   - 修复建议：采用动态链接方案，并在 THIRD-PARTY.md 中说明

2. **THIRD-PARTY.md 缺失**（中优先级）- L-02
   - 问题描述：未列出第三方依赖及协议，协议合规性不明确
   - 修复建议：创建 THIRD-PARTY.md，列出所有依赖及协议

3. **代码文件协议声明缺失**（中优先级）- L-04
   - 问题描述：未确认代码文件头部是否有 GPL v3 声明
   - 修复建议：检查并添加 GPL v3 头部声明

### 建议
- **立即行动**：创建 THIRD-PARTY.md，明确第三方依赖协议
- **法律咨询**：咨询开源法律专家，确认 MinHook（MIT）与 GPL v3 静态链接的合规性
- **长期方案**：考虑将 MinHook 改为动态链接，彻底规避协议冲突

---

## 6. 修复建议

### 创建 THIRD-PARTY.md

```markdown
# 第三方依赖协议声明 (Third-Party Licenses)

> 本文档列出 AI ThinApp Portable Launchpad Platform 项目的所有第三方依赖及其协议。

---

## 运行时依赖

| 依赖 | 版本 | 协议 | 用途 | 与 GPL v3 兼容性 |
|------|------|------|------|---------------------|
| MinHook | 1.3.3 | MIT | API Hook 框架 | ✅ 兼容（动态链接） |
| zlib | 1.3.1 | zlib | .vapp 包压缩 | ✅ 兼容 |
| RapidJSON | 1.1.0 | MIT | JSON 解析 | ✅ 兼容 |
| Electron | 28.x | MIT | Launchpad GUI | ✅ 兼容 |

---

## 构建时依赖

| 依赖 | 版本 | 协议 | 用途 | 与 GPL v3 兼容性 |
|------|------|------|------|---------------------|
| CMake | 3.20+ | BSD | 构建系统 | ✅ 兼容 |
| Visual Studio 2022 | 17.0+ | Proprietary | 编译器 | ✅ 兼容（构建工具） |

---

## 协议冲突说明

### MinHook（MIT）与 GPL v3

本项目采用 **动态链接** 方式使用 MinHook，不触发 GPL v3 的"衍生作品"定义，因此协议兼容。

如果采用静态链接，建议咨询法律专家，确认合规性。

---

## 协议文本

各依赖的完整协议文本可在以下位置找到：
- MinHook：https://github.com/TsudaKageyu/minhook/blob/master/LICENSE.md
- zlib：https://github.com/madler/zlib/blob/master/LICENSE
- RapidJSON：https://github.com/Tencent/rapidjson/blob/master/LICENSE
- Electron：https://github.com/electron/electron/blob/main/LICENSE

---

**维护者**：AI ThinApp Portable Launchpad Platform 开源社区  
**最后更新**：2026-05-23
```

### 修改代码文件头部（示例）

```cpp
/**
 * @file hook_engine.cpp
 * @brief Hook 引擎核心实现
 * 
 * @copyright Copyright (C) 2024 anmingwei
 * @license This file is part of AI ThinApp Portable Launchpad Platform.
 *          This program is free software: you can redistribute it and/or modify
 *          it under the terms of the GNU General Public License as published by
 *          the Free Software Foundation, either version 3 of the License, or
 *          (at your option) any later version.
 * 
 *          This program is distributed in the hope that it will be useful,
 *          but WITHOUT ANY WARRANTY; without even the implied warranty of
 *          MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *          GNU General Public License for more details.
 * 
 *          You should have received a copy of the GNU General Public License
 *          along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */
```

---

**校验人**：文档工程师  
**校验日期**：2026-05-23  
**版本**：v0.1.0

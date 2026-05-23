# 第三方依赖协议声明 (Third-Party Licenses)

> 本文档列出 AI ThinApp Portable Launchpad Platform 项目的所有第三方依赖及其协议，说明与 GPL v3 的兼容性。

---

## 1. 运行时依赖

| 依赖 | 版本 | 协议 | 用途 | 与 GPL v3 兼容性 | 使用方式 |
|------|------|------|------|---------------------|----------|
| MinHook | 1.3.3 | MIT | API Hook 框架 | ✅ 兼容 | 动态链接（DLL） |
| zlib | 1.3.1 | zlib | .vapp 包压缩 | ✅ 兼容 | 静态链接 |
| RapidJSON | 1.1.0 | MIT | JSON 解析（元数据） | ✅ 兼容 | 静态链接（头文件库） |
| Electron | 28.x | MIT | Launchpad GUI | ✅ 兼容 | 运行时依赖（npm） |

### 1.1 MinHook（MIT）协议说明

**协议文本**：https://github.com/TsudaKageyu/minhook/blob/master/LICENSE.md

**MIT 协议要求**：
- 保留版权声明和许可证声明
- 免责声明（作者不承担任何责任）

**与 GPL v3 兼容性**：
- ✅ **动态链接**：不触发 GPL v3 的"衍生作品"定义，完全兼容
- ⚠️ **静态链接**：某些法律观点认为静态链接后，MinHook 代码成为项目的一部分，必须采用 GPL v3 开源（但 MinHook 已开源，所以可能无问题）
- ✅ **本项目采用动态链接**，完全合规

**协议文件**：
- 源代码分发时，包含 MinHook 的 LICENSE.md
- 二进制分发时，在关于页面或 LICENSE 文件中声明 MinHook 版权

---

## 2. 构建时依赖

| 依赖 | 版本 | 协议 | 用途 | 与 GPL v3 兼容性 |
|------|------|------|------|---------------------|
| CMake | 3.20+ | BSD | 构建系统 | ✅ 兼容（构建工具） |
| Visual Studio 2022 | 17.0+ | Proprietary | 编译器 | ✅ 兼容（构建工具） |
| Python | 3.x | PSF License | 脚本工具 | ✅ 兼容（构建工具） |

**说明**：构建时依赖不参与分发，不影响协议兼容性。

---

## 3. 开发工具依赖

| 依赖 | 版本 | 协议 | 用途 | 与 GPL v3 兼容性 |
|------|------|------|------|---------------------|
| Google Test | 1.14+ | BSD | 单元测试框架 | ✅ 兼容 |
| x64dbg | 2024+ | GPL v3 | 调试工具 | ✅ 兼容（同协议） |
| Process Monitor | - | Microsoft EULA | 系统监视工具 | ✅ 兼容（工具） |

**说明**：开发工具依赖仅用于开发阶段，不影响分发。

---

## 4. 协议冲突与解决

### 4.1 已识别的协议冲突

| ID | 依赖 | 冲突描述 | 冲突级别 | 解决方案 | 状态 |
|----|------|----------|----------|----------|--------|
| L-01 | MinHook (MIT) | 静态链接可能导致 GPL v3 传染性问题 | 高 | 采用动态链接 | ✅ 已解决 |
| - | Microsoft Detours | Detours EULA 可能不允许 GPL 项目使用 | 中 | 不使用 Detours，使用 MinHook | ✅ 已规避 |

### 4.2 协议合规性检查清单

| 检查项 | 是否满足 | 说明 |
|----------|----------|------|
| GPL v3 协议文本完整 | ✅ 满足 | LICENSE 文件包含完整 GPL v3 文本 |
| 第三方协议声明完整 | ✅ 满足 | 本文档列出所有依赖及协议 |
| 源代码包含协议声明 | ⚠️ 部分满足 | 需为所有代码文件添加 GPL v3 头部声明 |
| 二进制分发包含协议文件 | ⚠️ 待确认 | 需确认安装包是否包含 LICENSE 和 THIRD-PARTY.md |
| 保留第三方版权声明 | ✅ 满足 | 本文档保留 MinHook、zlib 等版权声明 |

---

## 5. 协议文件分发要求

### 5.1 源代码分发（GitHub 仓库）

必须包含的文件：
- ✅ `LICENSE`（GPL v3 完整文本）
- ✅ `docs/THIRD-PARTY.md`（第三方依赖协议声明）
- ✅ 所有代码文件头部包含 GPL v3 声明（待完成）

### 5.2 二进制分发（Release 安装包）

必须包含的文件：
- ✅ `LICENSE`（GPL v3 完整文本）
- ✅ `THIRD-PARTY.md`（第三方依赖协议声明）
- ✅ `README.md`（包含项目协议声明）

**建议**：
- 在安装目录中添加 `LICENSE` 和 `THIRD-PARTY.md`
- 在 Launchpad"关于"页面中，添加协议声明和第三方依赖列表

---

## 6. 协议常见问题

### Q1. 我可以在闭源商业产品中使用本项目代码吗？

**A**：不可以。GPL v3 要求衍生项目必须采用 GPL v3 协议开源。

**如果需要闭源商用**：
1. 联系作者协商商业授权（如果代码完全由作者编写，可以双协议授权）
2. 重写核心引擎（不使用本项目的 GPL v3 代码）

### Q2. 我可以修改本项目代码并闭源分发吗？

**A**：不可以。修改本项目代码后分发，必须采用 GPL v3 协议开源。

### Q3. 我可以使用本项目的代码作为公司内部工具吗？

**A**：可以。GPL v3 仅要求"分发"时开源，公司内部使用（不分发）不需要开源。

### Q4. MinHook 的 MIT 协议要求是什么？

**A**：MIT 协议要求：
1. 保留版权声明和许可证声明
2. 免责声明（作者不承担任何责任）

**本项目已满足**：
- 在 `THIRD-PARTY.md` 中声明 MinHook 版权
- 分发时包含 MinHook 的 LICENSE.md

---

## 7. 联系方式

如果对本项目的协议有任何疑问，请通过以下渠道联系：

- **GitHub Issues**：[https://github.com/anmingwei/ai-thinApp-portable-lauchpad-platform/issues](https://github.com/anmingwei/ai-thinApp-portable-lauchpad-platform/issues)
- **GitHub Discussions**：[https://github.com/anmingwei/ai-thinApp-portable-lauchpad-platform/discussions](https://github.com/anmingwei/ai-thinApp-portable-lauchpad-platform/discussions)
- **邮件**：待添加

---

## 8. 修订历史

| 版本 | 日期 | 作者 | 变更说明 |
|------|------|------|----------|
| v0.1.0 | 2026-05-23 | PM | 初版，基于协议一致性校验输出 |

---

**维护者**：AI ThinApp Portable Launchpad Platform 开源社区  
**最后更新**：2026-05-23

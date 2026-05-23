# 贡献指南 (Contributing Guide)

感谢您对 AI ThinApp Portable Launchpad Platform 项目的关注！

## 如何报告 Bug

如果您发现了 Bug，请通过以下步骤报告：

1. 检查 [GitHub Issues](https://github.com/用户名/ai-thinapp-portable-launchpad/issues) 中是否已存在相同问题
2. 如果不存在，点击 "New Issue" 创建新问题
3. 使用 Bug 报告模板（如果已配置）
4. 提供详细信息：
   - 问题描述
   - 复现步骤
   - 预期行为
   - 实际行为
   - 环境信息（操作系统版本、编译器版本等）
   - 相关日志或截图

## 如何提交 Pull Request

1. **Fork 本仓库**
   - 点击 GitHub 页面右上角的 "Fork" 按钮

2. **创建功能分支**
   ```bash
   git clone https://github.com/你的用户名/ai-thinapp-portable-launchpad.git
   cd ai-thinapp-portable-launchpad
   git checkout -b feature/你的功能名称
   ```

3. **进行更改**
   - 遵循代码规范（见下文）
   - 添加或更新测试
   - 更新文档（如果需要）

4. **提交更改**
   - 遵循 Git 提交规范（见下文）
   - 保持提交历史清晰

5. **推送到你的 Fork**
   ```bash
   git push origin feature/你的功能名称
   ```

6. **创建 Pull Request**
   - 访问你 Fork 的仓库页面
   - 点击 "Compare & pull request"
   - 填写 PR 描述模板
   - 关联相关的 Issue（如果有）

## 代码规范

### C++ 标准
- 使用 C++17 标准
- 保持代码风格一致

### 注释规范
- 使用中文注释
- 函数、类、重要逻辑都需要注释
- 注释应说明"为什么"而不是"做什么"

### 编码规范
- 文件编码：UTF-8（无 BOM）
- 缩进：4 个空格
- 命名规范：
  - 类名：PascalCase（如 `HookEngine`）
  - 函数名：camelCase（如 `injectDll`）
  - 成员变量：m_ 前缀（如 `m_registryRoot`）
  - 常量：k 前缀 + PascalCase（如 `kMaxPathLength`）

### 文件组织
- 头文件：`.h`
- 实现文件：`.cpp`
- 每个类一个文件（或逻辑相关的类组）

## Git 提交规范

遵循 [Conventional Commits](https://www.conventionalcommits.org/) 规范：

```
<类型>[可选的作用域]: <描述>

[可选的正文]

[可选的脚注]
```

### 类型
- **feat**: 新功能
- **fix**: Bug 修复
- **docs**: 文档更新
- **style**: 代码格式（不影响代码运行的变动）
- **refactor**: 重构（既不是新功能也不是 Bug 修复）
- **perf**: 性能优化
- **test**: 添加或修正测试
- **chore**: 构建过程或辅助工具的变动

### 示例

```bash
feat(hook): 添加进程继承 Hook 支持

实现了基于 PROC_THREAD_ATTRIBUTE_LIST 的进程继承拦截，
支持 CreateProcess 的句柄继承控制。

Closes #123
```

```bash
fix(vfs): 修复文件重定向在 WoW64 下的兼容性问题

Release 模式下正确地进行文件系统重定向，避免 DLL 加载失败。

Fixes #456
```

## 开发环境搭建

请参考 [开发指南](docs/DEV-GUIDE.md) 设置开发环境。

### 快速开始

1. **安装依赖**
   - Visual Studio 2022 (C++ 桌面开发工作负载)
   - CMake 3.20+
   - Git

2. **克隆仓库**
   ```bash
   git clone https://github.com/用户名/ai-thinapp-portable-launchpad.git
   cd ai-thinapp-portable-launchpad
   ```

3. **构建项目**
   ```bash
   cmake -B build -DCMAKE_BUILD_TYPE=Debug
   cmake --build build --config Debug
   ```

4. **运行测试**
   ```bash
   cd build
   ctest --output-on-failure
   ```

## 代码审查

所有 Pull Request 都需要经过代码审查：

- 至少需要 1 个维护者的 Approve
- CI 检查必须通过
- 所有讨论都必须解决

审查重点：
- 代码质量和可读性
- 测试覆盖率
- 性能影响
- 安全考虑
- 文档完整性

## 社区

- [GitHub Discussions](https://github.com/用户名/ai-thinapp-portable-launchpad/discussions) - 问答、讨论
- [GitHub Issues](https://github.com/用户名/ai-thinapp-portable-launchpad/issues) - Bug 报告、功能请求

## 许可证

通过贡献代码，您同意您的贡献将在 GPL v3 许可证下发布。

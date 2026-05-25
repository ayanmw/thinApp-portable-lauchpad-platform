# MVP 发布流程

**项目**: AI-ThinApp Portable Launchpad Platform  
**版本**: MVP (v1.0.0)  
**日期**: 2026-05-24  
**负责人**: 产品经理  

---

## 📋 流程概述

MVP 发布流程分为 **6 个阶段**，总估算时间 **240 分钟（4 小时）**。

| 阶段 | 名称 | 时间 | 负责角色 | 阻塞项 |
|------|------|------|----------|--------|
| 1 | 编译验证 | 30 分钟 | 开发工程师 | 否 |
| 2 | 测试验证 | 60 分钟 | 测试工程师 | 是 |
| 3 | 代码签名 | 30 分钟 | 代码签名工程师 | 是 |
| 4 | 打包发布 | 30 分钟 | 部署工程师 | 是 |
| 5 | 应用商店提交 | 60 分钟 | 部署工程师 | 否 |
| 6 | 发布后监控 | 持续 | 产品经理 | 否 |

---

## 🚀 阶段 1：编译验证（30 分钟）

**目标**: 验证所有模块编译通过，0 错误 0 警告。

**负责人**: 开发工程师

### 步骤

1. **手动运行编译脚本**

   ```bash
   # 进入项目目录
   cd D:\anmw_work_proj\ai-thinApp-portable-launchpad-platform
   
   # 运行编译脚本
   .\tools\build\build_hook_simple.bat
   ```

2. **验证编译结果**

   - 检查输出日志，确认：
     - ✅ 0 错误
     - ✅ 0 警告
     - ✅ 所有模块编译成功
   
   - 检查生成的二进制文件：
     - `build\hook_core.dll`
     - `build\launcher.exe`
     - `build\vreg.exe`

3. **运行测试程序**

   ```bash
   # 运行所有测试程序
   .\build\test_hook_core.exe
   .\build\test_launcher.exe
   .\build\test_vreg.exe
   ```

   - 验证功能正常
   - 确认无崩溃、无异常

### 验证方法

- [ ] 编译日志显示 0 错误 0 警告
- [ ] 所有二进制文件已生成
- [ ] 所有测试程序运行通过

### 失败处理

- **编译失败** → 修复编译错误，重新编译
- **测试程序崩溃** → 调试并修复 Bug，重新编译

---

## ✅ 阶段 2：测试验证（60 分钟）

**目标**: 运行所有测试，验证功能、性能、集成测试通过。

**负责人**: 测试工程师

**阻塞项**: 是（必须通过才能继续）

### 步骤

1. **运行全模块测试**

   ```bash
   # 运行全模块测试
   tools\ci\run_all_tests.bat
   
   # 查看测试报告
   type build\test_report.txt
   ```

   - 验证所有测试用例通过
   - 记录失败的测试用例

2. **运行覆盖率测试**

   ```bash
   # 运行覆盖率测试
   tools\ci\run_coverage.bat
   
   # 查看覆盖率报告
   start build\coverage\index.html
   ```

   - 验证覆盖率 ≥ 80%
   - 关键模块覆盖率 ≥ 90%

3. **运行性能基准测试**

   ```bash
   # 运行性能测试
   tools\ci\run_perf_tests.bat
   
   # 查看性能报告
   type build\perf_report.txt
   ```

   - 验证启动时间 < 3s
   - 验证内存占用 < 200MB
   - 验证 Hook 开销 < 10%

4. **运行集成测试**

   ```bash
   # 运行集成测试
   tools\ci\run_integration_tests.bat
   
   # 查看集成测试报告
   type build\integration_test_report.txt
   ```

   - 验证所有集成测试用例通过
   - 确认无阻塞性问题

### 验证方法

- [ ] 全模块测试通过率 100%
- [ ] 代码覆盖率 ≥ 80%
- [ ] 性能基准测试通过（启动 < 3s、内存 < 200MB）
- [ ] 集成测试通过率 100%

### 失败处理

- **测试失败** → 记录失败用例，提交 Bug，修复后重新测试
- **覆盖率不达标** → 补充单元测试，重新测试
- **性能不达标** → 性能优化，重新测试

---

## 🔐 阶段 3：代码签名（30 分钟）

**目标**: 使用 EV 代码签名证书签署可执行文件。

**负责人**: 代码签名工程师

**阻塞项**: 是（必须签名才能发布）

### 步骤

1. **插入 EV 代码签名 USB Token**

   - 将 USB Token 插入计算机
   - 等待驱动程序加载完成
   - 确认设备已识别

2. **签署可执行文件**

   ```bash
   # 签署可执行文件
   tools\ci\sign_ev.ps1 -FilePath "build\launcher.exe"
   tools\ci\sign_ev.ps1 -FilePath "build\hook_core.dll"
   tools\ci\sign_ev.ps1 -FilePath "build\vreg.exe"
   ```

   - 输入 USB Token PIN 码
   - 确认签名成功

3. **添加时间戳**

   ```bash
   # 添加时间戳（确保签名在长期内有效）
   tools\ci\sign_timestamp.ps1 -FilePath "build\launcher.exe"
   tools\ci\sign_timestamp.ps1 -FilePath "build\hook_core.dll"
   tools\ci\sign_timestamp.ps1 -FilePath "build\vreg.exe"
   ```

   - 确认时间戳服务器可用
   - 确认时间戳添加成功

4. **验证签名**

   ```bash
   # 验证签名是否有效
   tools\ci\verify_signature.ps1 -FilePath "build\launcher.exe"
   tools\ci\verify_signature.ps1 -FilePath "build\hook_core.dll"
   tools\ci\verify_signature.ps1 -FilePath "build\vreg.exe"
   ```

   - 确认签名有效
   - 确认证书链完整
   - 确认时间戳有效

### 验证方法

- [ ] USB Token 已插入并识别
- [ ] 所有可执行文件已签署
- [ ] 时间戳已添加
- [ ] 签名验证通过

### 失败处理

- **USB Token 未识别** → 重新插入，安装驱动程序
- **签名失败** → 检查证书是否有效，重新签署
- **时间戳失败** → 检查网络连接，更换时间戳服务器

---

## 📦 阶段 4：打包发布（30 分钟）

**目标**: 构建 Release 版本，创建便携版 ZIP 和安装包。

**负责人**: 部署工程师

**阻塞项**: 是（必须测试通过、签名通过）

### 步骤

1. **构建 Release 版本**

   ```bash
   # 构建 Release 版本
   tools\build\build_release.bat
   
   # 验证 Release 构建
   # - 所有文件已生成
   # - 版本号正确
   # - 签名有效
   ```

   - 确认 `build\release\` 目录已生成
   - 确认所有文件已签署

2. **创建便携版 ZIP**

   ```bash
   # 创建便携版 ZIP
   tools\deploy\create_portable_zip.ps1 -OutputPath "build\release\AI-ThinApp-Portable-v1.0.0.zip"
   
   # 验证 ZIP 文件
   # - 包含所有必要文件
   # - 文件结构正确
   # - 可以解压并运行
   ```

   - 确认 ZIP 文件已生成
   - 测试解压后可以正常运行

3. **创建安装包**

   ```bash
   # 创建 NSIS 安装包
   tools\deploy\create_installer.nsi
   
   # 验证安装包
   # - 安装包已生成
   # - 可以正常安装
   # - 卸载功能正常
   ```

   - 确认 `build\release\AI-ThinApp-Setup-v1.0.0.exe` 已生成
   - 测试安装和卸载流程

### 验证方法

- [ ] Release 版本已构建
- [ ] 便携版 ZIP 已创建
- [ ] 安装包已创建
- [ ] 安装包可以正常安装和卸载

### 失败处理

- **构建失败** → 检查配置，修复错误，重新构建
- **ZIP 创建失败** → 检查文件权限，重新创建
- **安装包创建失败** → 检查 NSIS 脚本，修复错误，重新创建

---

## 🌐 阶段 5：应用商店提交（60 分钟）

**目标**: 提交应用到 Microsoft Store、Itch.io 和独立站。

**负责人**: 部署工程师

### 步骤

1. **提交 Microsoft Store**

   ```bash
   # 提交 Microsoft Store
   tools\deploy\submit_ms_store.ps1 -PackagePath "build\release\AI-ThinApp-Setup-v1.0.0.exe" -Version "1.0.0"
   ```
   
   - 登录 Microsoft Partner Center
   - 上传安装包
   - 填写应用信息（名称、描述、截图等）
   - 提交审核

2. **提交 Itch.io**

   ```bash
   # 提交 Itch.io
   tools\deploy\submit_itch.ps1 -PackagePath "build\release\AI-ThinApp-Portable-v1.0.0.zip" -Version "1.0.0"
   ```
   
   - 登录 Itch.io
   - 上传便携版 ZIP
   - 填写游戏/应用信息
   - 发布

3. **部署独立站**

   ```bash
   # 部署独立站
   tools\deploy\deploy_indie_site.ps1 -PackagePath "build\release\" -Version "1.0.0"
   ```
   
   - 构建静态网站
   - 上传到 GitHub Pages 或 Vercel
   - 验证网站可访问
   - 验证下载链接有效

### 验证方法

- [ ] Microsoft Store 提交成功，进入审核流程
- [ ] Itch.io 发布成功，可下载
- [ ] 独立站部署成功，可访问

### 失败处理

- **Microsoft Store 提交失败** → 检查应用信息是否完整，重新提交
- **Itch.io 上传失败** → 检查网络连接，重新上传
- **独立站部署失败** → 检查构建日志，修复错误，重新部署

---

## 📊 阶段 6：发布后监控（持续）

**目标**: 监控应用商店审核状态，收集用户反馈，修复关键 Bug。

**负责人**: 产品经理

### 任务

1. **监控应用商店审核状态**

   - 每天检查 Microsoft Store 审核状态
   - 如果被拒绝，查看拒绝原因，修复问题，重新提交

2. **收集用户反馈**

   - 监控 GitHub Issues
   - 监控邮件反馈
   - 监控应用商店评论

3. **修复关键 Bug**

   - 对关键 Bug 创建 hotfix
   - 测试 hotfix
   - 发布紧急更新

### 监控方法

- [ ] Microsoft Store 审核状态：每天检查
- [ ] GitHub Issues：每天检查
- [ ] 邮件反馈：每天检查
- [ ] 应用商店评论：每周检查

---

## 📝 发布检查清单

在启动发布流程前，确认以下检查项已通过：

- [ ] 编译验证通过（0 错误 0 警告）
- [ ] 单元测试通过率 ≥ 80%
- [ ] 集成测试通过
- [ ] 性能基准测试通过（启动 < 3s、内存 < 200MB）
- [ ] 代码签名验证通过（EV 证书）
- [ ] 应用商店材料完整（图标、截图、描述）
- [ ] 用户文档完整（手册、FAQ、教程）
- [ ] 部署文档完整（安装包、签名、商店上架）
- [ ] 已知问题清单已整理
- [ ] 后续路线图已规划

---

## 🎯 时间估算

| 阶段 | 时间 | 依赖 |
|------|------|------|
| 阶段 1：编译验证 | 30 分钟 | 无 |
| 阶段 2：测试验证 | 60 分钟 | 阶段 1 |
| 阶段 3：代码签名 | 30 分钟 | 阶段 2 |
| 阶段 4：打包发布 | 30 分钟 | 阶段 3 |
| 阶段 5：应用商店提交 | 60 分钟 | 阶段 4 |
| **总计** | **240 分钟（4 小时）** | - |

---

## 🚨 回滚计划

如果发布后发现严重问题，执行以下回滚计划：

1. **Microsoft Store** → 下架应用，修复问题后重新提交
2. **Itch.io** → 删除版本，上传修复后的版本
3. **独立站** → 回滚到上一个版本，修复问题后重新部署

---

## 📄 相关文档

- [MVP 发布前检查清单](./MVP-LAUNCH-CHECKLIST.md)
- [已知问题清单](./MVP-KNOWN-ISSUES.md)
- [后续路线图](./MVP-ROADMAP.md)
- [发布准备报告](./MVP-LAUNCH-PREP-REPORT.md)

---

**最后更新**: 2026-05-24  
**更新人**: 产品经理

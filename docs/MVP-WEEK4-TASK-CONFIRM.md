# MVP Week 4 任务理解确认

> 本文档确认对 AI ThinApp Portable Launchpad Platform 项目 MVP Week 4 开发任务的理解。
> 日期：2026-05-23 | 负责人：Windows C++ 底层开发工程师

---

## 任务概述

负责 MVP Week 4 的应用捕获工具实现，包括四个核心模块：
1. .vapp 包格式实现（ZIP 压缩 + EV 代码签名）
2. 兼容性规则库实现（YAML 格式，三种匹配模式）
3. 应用商店集成实现（RESTful API + SQLite 数据库）
4. 捕获失败诊断实现（模式匹配，准确率 ≥ 80%）

---

## 模块一：.vapp 包格式（Day 1-2）

### 设计理解

根据设计文档第 2 章，.vapp 包采用 ZIP 压缩格式（MVP 阶段），包含：
- `manifest.json`：应用元数据（名称、版本、发布者等）
- `registry.hive`：虚拟注册表 hive
- `VFS\`：虚拟文件系统目录
- `signature.json`：EV 代码签名信息
- `rules.yaml`：兼容性规则（可选）

### 开发任务

1. **创建 `src/packager/vapp_package.h`**
   - 定义 `VAppPackage` 类
   - 成员变量：`package_path_`、`app_name_`、`version_`、`signature_`
   - 方法声明：`Compress()`、`Decompress()`、`VerifySignature()`、`SignPackage()`

2. **创建 `src/packager/vapp_package.cpp`**
   - 实现 `Compress()`：使用 zlib 压缩目录为 .vapp 包
   - 实现 `Decompress()`：解压 .vapp 包到目标目录
   - 实现 `VerifySignature()`：验证 EV 代码签名（调用 WinTrust.dll API）
   - 实现 `SignPackage()`：对 .vapp 包进行 EV 代码签名（调用 SignTool.exe）

3. **修改 `src/packager/app_capture.h/cpp`**
   - 集成 `VAppPackage` 类
   - 在 `CaptureApp()` 方法结束时调用 `VAppPackage::Compress()`
   - 添加 `ExportVApp()` 方法

### 验收标准

- ✅ .vapp 包压缩正确率 = 100%（压缩 100 个应用，全部成功）
- ✅ .vapp 包解压正确率 = 100%（解压 100 个 .vapp 包，全部成功）
- ✅ EV 代码签名验证正确率 = 100%（验证 100 个已签名包，全部通过）
- ✅ .vapp 包大小 ≤ 100MB（压缩率 ≥ 50%）
- ✅ 单元测试覆盖率 ≥ 80%

---

## 模块二：兼容性规则库（Day 3）

### 设计理解

根据设计文档第 3 章，兼容性规则库使用 YAML 格式，支持三种匹配模式：
- 前缀匹配（如 `C:\Windows\System32\*`）
- 通配符匹配（如 `C:\Program Files\*\app.exe`）
- 正则匹配（如 `^C:\\Users\\[^\\]+\\AppData\\.*$`）

### 开发任务

1. **创建 `src/packager/compat_rules.h`**
   - 定义 `CompatRule` 结构体（`rule_type_`、`pattern_`、`description_`）
   - 定义 `CompatRules` 类
   - 方法声明：`LoadRules()`、`MatchRule()`、`GetMatchedRules()`

2. **创建 `src/packager/compat_rules.cpp`**
   - 实现 `LoadRules()`：从 YAML 文件加载兼容性规则（使用 yaml-cpp 库）
   - 实现 `MatchRule()`：匹配文件路径到兼容性规则（三种模式）
   - 实现 `GetMatchedRules()`：返回所有匹配的规则

3. **创建 `tools/ci/check_compat_rules.ps1`**
   - PowerShell 脚本：检查应用是否匹配兼容性规则
   - 输入：应用安装目录、兼容性规则文件
   - 输出：匹配的规则列表、建议的 Hook 配置

4. **编写测试程序 `tests/packager/test_compat_rules.cpp`**
   - 测试用例 1：加载兼容性规则
   - 测试用例 2：前缀匹配
   - 测试用例 3：通配符匹配
   - 测试用例 4：正则匹配
   - 测试用例 5：无匹配规则

### 验收标准

- ✅ 兼容性规则加载正确率 = 100%（加载 100 条规则，全部成功）
- ✅ 前缀匹配正确率 = 100%（测试 100 个路径，全部匹配正确）
- ✅ 通配符匹配正确率 = 100%（测试 100 个路径，全部匹配正确）
- ✅ 正则匹配正确率 = 100%（测试 100 个路径，全部匹配正确）
- ✅ 无匹配规则处理正确率 = 100%（测试 100 个路径，全部返回空列表）
- ✅ 单元测试覆盖率 ≥ 80%

---

## 模块三：应用商店集成（Day 4）

### 设计理解

根据设计文档第 4 章，应用商店集成包括：
- RESTful API 设计（`/apps`、`/apps/{app_id}`、`/apps/search` 等端点）
- SQLite 数据库（本地缓存应用元数据）
- WinHTTP API 调用 RESTful API

### 开发任务

1. **创建 `src/packager/app_store.h`**
   - 定义 `AppStore` 类
   - 成员变量：`api_base_url_`、`local_db_path_`
   - 方法声明：`UploadApp()`、`DownloadApp()`、`SearchApps()`、`UpdateLocalDB()`

2. **创建 `src/packager/app_store.cpp`**
   - 实现 `UploadApp()`：上传 .vapp 包到应用商店（POST /api/v1/apps/upload）
   - 实现 `DownloadApp()`：从应用商店下载 .vapp 包（GET /api/v1/apps/download）
   - 实现 `SearchApps()`：搜索应用商店中的应用（GET /api/v1/apps/search）
   - 实现 `UpdateLocalDB()`：更新本地 SQLite 数据库

3. **创建 `tools/ci/sync_app_store.ps1`**
   - PowerShell 脚本：同步本地应用库与应用商店
   - 输入：本地应用库目录、应用商店 API 地址
   - 输出：同步日志（新增应用、更新应用、删除应用）

4. **编写测试程序 `tests/packager/test_app_store.cpp`**
   - 测试用例 1：上传应用
   - 测试用例 2：下载应用
   - 测试用例 3：搜索应用
   - 测试用例 4：更新本地数据库

### 验收标准

- ✅ 应用上传成功率 = 100%（上传 100 个 .vapp 包，全部成功）
- ✅ 应用下载成功率 = 100%（下载 100 个 .vapp 包，全部成功）
- ✅ 应用搜索正确率 = 100%（搜索 100 个关键词，全部返回正确结果）
- ✅ 本地数据库更新正确率 = 100%（更新 100 次，全部成功）
- ✅ 单元测试覆盖率 ≥ 80%

---

## 模块四：捕获失败诊断（Day 5）

### 设计理解

根据设计文档第 5 章，捕获失败自动诊断使用模式匹配算法：
- 预定义失败模式库（正则表达式匹配）
- 自动分析日志、定位问题、给出修复建议
- 准确率 ≥ 80%

### 开发任务

1. **修改 `src/packager/app_capture.h/cpp`**
   - 添加 `DiagnoseCaptureFailure()` 方法：诊断捕获失败原因，返回诊断报告
   - 添加 `SuggestFix()` 方法：根据诊断报告，建议修复方案

2. **创建 `docs/CAPTURE-FAILURE-DIAGNOSIS.md`**
   - 文档：捕获失败诊断规则（10 类常见失败原因、诊断步骤、修复建议）

3. **编写测试程序 `tests/packager/test_capture_diagnosis.cpp`**
   - 测试用例 1：诊断注册表快照失败
   - 测试用例 2：诊断 .vapp 包导出失败
   - 测试用例 3：诊断 Hook DLL 注入失败
   - 测试用例 4：建议修复方案

### 验收标准

- ✅ 捕获失败诊断准确率 ≥ 80%（模拟 100 次捕获失败，诊断正确 ≥ 80 次）
- ✅ 修复建议正确率 ≥ 80%（根据诊断报告，建议修复方案正确 ≥ 80 次）
- ✅ 诊断报告格式正确率 = 100%（输出 JSON 格式，包含失败原因、诊断步骤、修复建议）
- ✅ 单元测试覆盖率 ≥ 80%

---

## 关键技术方案

| 模块 | 技术栈 | 说明 |
|------|--------|------|
| .vapp 包格式 | zlib | ZIP 压缩 |
| .vapp 包格式 | WinTrust.dll / SignTool.exe | EV 代码签名验证和签名 |
| 兼容性规则库 | yaml-cpp | YAML 解析 |
| 兼容性规则库 | 正则表达式 | 三种匹配模式 |
| 应用商店集成 | WinHTTP | RESTful API 调用 |
| 应用商店集成 | SQLite3 | 本地数据库 |
| 捕获失败诊断 | 模式匹配 | 预定义失败模式库 |

---

## 依赖环境

### 必需库

1. **zlib**：ZIP 压缩
2. **yaml-cpp**：YAML 解析
3. **SQLite3**：本地数据库
4. **Windows SDK**：WinHTTP、WinTrust.dll

### 工具

1. **SignTool.exe**：EV 代码签名（需提前申请 EV 证书）
2. **CMake**：构建系统
3. **Visual Studio 2022**：编译器

---

## 风险与阻塞

| 风险 | 影响 | 概率 | 缓解措施 |
|------|------|------|----------|
| EV 代码签名证书申请失败 | 无法测试签名功能 | 中 | 先实现签名逻辑，使用测试证书或模拟签名 |
| yaml-cpp 库编译失败 | 无法解析 YAML 规则 | 低 | 使用 vcpkg 或 Conan 管理依赖 |
| WinHTTP API 调用复杂 | 开发周期延长 | 中 | 使用 libcurl 作为备选方案 |
| 注册表快照实现复杂 | 无法完成捕获诊断 | 高 | 使用 RegEnumKeyEx 备选方案 |

---

## 下一步行动

1. 创建 `docs/MVP-WEEK4-TASK-CONFIRM.md`（本文件）
2. 实现 .vapp 包格式（Day 1-2）
3. 实现兼容性规则库（Day 3）
4. 实现应用商店集成（Day 4）
5. 实现捕获失败诊断（Day 5）
6. 输出 Week 4 完成报告

---

## 确认签名

- **负责人**：Windows C++ 底层开发工程师
- **日期**：2026-05-23
- **状态**：✅ 任务理解确认完成，开始开发

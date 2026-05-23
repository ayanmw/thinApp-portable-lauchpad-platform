# 杀毒软件误报缓解指南 (Antivirus Guide)

## 1. 问题概述

### 1.1 为什么会被误报？

Hook 引擎使用了以下技术，容易被杀毒软件标记为"可疑行为"：

1. **API Hook（函数钩子）**
   - 修改系统 API 的函数指针
   - 杀毒软件可能认为是"注入行为"或"Rootkit"

2. **DLL 注入**
   - 通过 `CreateRemoteThread` + `LoadLibrary` 注入 DLL 到目标进程
   - 杀毒软件可能认为是"进程注入攻击"

3. **注册表和文件系统重定向**
   - 拦截并修改系统 API 的行为
   - 杀毒软件可能认为是"沙箱"或"虚拟化软件"

4. **未签名的可执行文件**
   - 没有代码签名证书的 EXE/DLL
   - 杀毒软件对未签名文件更敏感

### 1.2 常见误报类型

| 杀毒软件 | 常见误报名称 | 严重程度 |
|----------|--------------|----------|
| Windows Defender | `Win32/Verti` / `PUP` (Potentially Unwanted Program) | 中等 |
| 火绒 | `RiskWare` / `HackTool` | 中等 |
| 360 安全卫士 | `注入行为` / `可疑程序` | 高 |
| 腾讯电脑管家 | `风险软件` / `恶意行为` | 高 |

---

## 2. 解决方案

### 2.1 方案 1：代码签名证书（推荐，根本解决）

#### 2.1.1 证书类型

| 证书类型 | 价格（约） | 验证级别 | 白名单效果 | 推荐度 |
|----------|------------|----------|------------|--------|
| OV (Organization Validation) | $200-300/年 | 验证企业身份 | 中等 | ⭐⭐⭐ |
| EV (Extended Validation) | $300-400/年 | 严格验证企业身份 | 高，自动加入微软智能屏幕白名单 | ⭐⭐⭐⭐⭐ |
| 个人证书（无） | 免费 | 无 | 无 | ❌ |

#### 2.1.2 申请流程（以 DigiCert 为例）

1. **选择 CA（证书颁发机构）**
   - DigiCert（推荐，信誉好）
   - Sectigo（性价比高）
   - GlobalSign（老牌 CA）

2. **准备材料**
   - 企业营业执照（OV/EV 需要）
   - 申请人身份证
   - 企业电话号码（需验证）

3. **购买证书**
   - 访问 CA 官网或代理商
   - 选择 OV 或 EV 代码签名证书
   - 完成支付

4. **验证身份**
   - CA 会电话联系企业验证身份
   - EV 证书还需要律师事务所或会计师事务所验证

5. **签发证书**
   - 验证通过后，CA 会邮寄 USB Token（EV 证书）
   - 或提供下载链接（OV 证书）

6. **签名 EXE/DLL**
   ```powershell
   # 使用 signtool.exe 签名
   # Visual Studio 自带 signtool.exe
   
   # OV 证书签名
   signtool.exe sign /f "MyCertificate.pfx" /p "证书密码" /t "http://timestamp.digicert.com" "HookEngine_x64.dll"
   
   # EV 证书签名（USB Token）
   signtool.exe sign /sha1 "证书指纹" /tr "http://timestamp.digicert.com" /td SHA256 "HookEngine_x64.dll"
   ```

7. **验证签名**
   ```powershell
   # 检查文件签名
   signtool.exe verify /pa /v "HookEngine_x64.dll"
   ```

#### 2.1.3 注意事项

- **EV 证书效果最好**：自动加入微软智能屏幕白名单，Windows Defender 不会误报
- **时间戳很重要**：即使证书过期，有时间戳的签名仍然有效
- **所有 EXE/DLL 都要签名**：包括测试程序和依赖的 DLL

---

### 2.2 方案 2：在 GitHub 开源（提高透明度）

#### 2.2.1 为什么开源能减少误报？

1. **透明化**：杀毒软件厂商可以审查代码，确认没有恶意行为
2. **社区信任**：开源项目更容易获得杀毒软件厂商的信任
3. **白名单申请**：提供开源仓库链接，申请加入杀毒软件白名单更容易

#### 2.2.2 开源步骤

1. **选择开源协议**
   - **MIT 协议**（推荐）：宽松，商业友好
   - **GPL v3**：强 Copyleft，要求衍生项目也开源
   - **Apache 2.0**：宽松，包含专利授权条款

2. **创建 GitHub 仓库**
   ```bash
   # 初始化仓库
   git init
   git add .
   git commit -m "Initial commit"
   
   # 创建 GitHub 仓库后
   git remote add origin https://github.com/你的用户名/ai-thinapp-launchpad.git
   git push -u origin main
   ```

3. **编写 README.md**
   - 项目介绍
   - 技术原理（说明为什么需要 Hook/注入）
   - 编译步骤
   - 使用方法
   - **明确声明**：本项目用于合法的应用虚拟化，不涉及恶意行为

4. **提交到 GitHub**
   - 确保代码完整可编译
   - 包含文档（ARCHITECTURE.md, POC-PLAN.md 等）
   - 添加 LICENSE 文件（选择的开源协议）

5. **申请杀毒软件白名单**
   - 向各大杀毒软件厂商提交白名单申请
   - 提供 GitHub 开源仓库链接
   - 说明项目用途和技术原理

#### 2.2.3 常见杀毒软件厂商白名单申请入口

| 厂商 | 白名单申请入口 | 备注 |
|------|----------------|------|
| Microsoft (Windows Defender) | https://www.microsoft.com/en-us/wdsi/filesubmission | 提交文件供分析 |
| 火绒 | https://www.huorong.cn/feedback.html | 联系客服申请 |
| 360 | https://sampleup.b.360.cn/ | 提交误报文件 |
| 腾讯电脑管家 | https://guanjia.qq.com/feedback.html | 联系客服申请 |

---

### 2.3 方案 3：用户侧临时解决方案（无奈之举）

#### 2.3.1 关闭杀毒软件（不推荐）

**风险**：降低系统安全性，仅用于测试环境。

**操作步骤（Windows Defender）**：
1. 打开"Windows 安全中心"
2. 点击"病毒和威胁防护"
3. 点击"管理设置"
4. 关闭"实时保护"

**恢复步骤**（重要！）：
- 测试完成后，务必重新开启"实时保护"

#### 2.3.2 添加信任/排除项（推荐）

**Windows Defender**：
1. 打开"Windows 安全中心"
2. 点击"病毒和威胁防护"
3. 点击"管理设置"
4. 向下滚动，点击"添加或删除排除项"
5. 点击"添加排除项"，选择：
   - **文件**：`HookEngine_x64.dll`
   - **文件夹**：项目输出目录（如 `D:\anmw_work_proj\ai-thinApp-portable-lauchpad-platform\build\output`）

**火绒安全**：
1. 打开火绒主界面
2. 点击"安全设置"
3. 点击"病毒防护"-"信任区"
4. 点击"添加信任文件"或"添加信任目录"
5. 选择项目文件或目录

**360 安全卫士**：
1. 打开 360 主界面
2. 点击"木马查杀"-"信任区"
3. 点击"添加信任文件"或"添加信任目录"

**腾讯电脑管家**：
1. 打开管家主界面
2. 点击"病毒查杀"-"信任区"
3. 点击"添加文件"或"添加目录"

#### 2.3.3 提示用户放行（用户体验优化）

在程序中检测杀毒软件误报，并提示用户添加信任：

```cpp
// 检测 Windows Defender 是否开启
BOOL IsDefenderEnabled() {
    // 通过注册表或 WMI 检测
    // 简化示例：假设检测到 Defender 开启
    return TRUE;
}

// 提示用户添加排除项
void ShowAntivirusGuide() {
    MessageBoxW(NULL, 
        L"检测到 Windows Defender 可能误报本程序。\n\n"
        L"请手动添加排除项：\n"
        L"1. 打开"Windows 安全中心"\n"
        L"2. 点击"病毒和威胁防护"-"管理设置"\n"
        L"3. 点击"添加或删除排除项"\n"
        L"4. 添加本程序所在目录\n\n"
        L"详细说明请查看文档：docs\\ANTIVIRUS-GUIDE.md",
        L"杀毒软件误报提示", 
        MB_OK | MB_ICONINFORMATION);
}
```

---

## 3. 最佳实践建议

### 3.1 开发阶段

1. **使用代码签名证书（EV 优先）**
   - 这是根本解决方法
   - 预算允许的情况下，务必购买 EV 证书

2. **在干净虚拟机中测试**
   - 使用 VMware Workstation 创建快照
   - 在不同杀毒软件环境下测试

3. **逐步发布**
   - 先发布到 GitHub（开源）
   - 收集社区反馈
   - 再申请杀毒软件白名单

### 3.2 用户使用阶段

1. **提供详细文档**
   - 本文档（ANTIVIRUS-GUIDE.md）
   - 说明为什么需要 Hook/注入
   - 说明如何添加排除项

2. **提供联系渠道**
   - GitHub Issues
   - 邮箱支持
   - 用户误报后可以联系开发者

3. **持续维护**
   - 如果收到新的误报报告，及时申请加入白名单
   - 更新文档，添加新的杀毒软件配置方法

---

## 4. 附录

### 4.1 代码签名工具下载

- **signtool.exe**（Visual Studio 自带）
  - 安装 Visual Studio 2022 时勾选"Windows SDK"
  - 路径：`C:\Program Files (x86)\Windows Kits\10\bin\x64\signtool.exe`

- **OpenSSL**（生成证书请求）
  - 下载：https://slproweb.com/products/Win32OpenSSL.html

### 4.2 参考链接

- **Microsoft 代码签名文档**：https://learn.microsoft.com/en-us/windows/win32/seccrypto/cryptography-tools
- **DigiCert 代码签名证书**：https://www.digicert.com/code-signing
- **Windows Defender 文件提交**：https://www.microsoft.com/en-us/wdsi/filesubmission
- **GitHub 开源指南**：https://docs.github.com/en/get-started/exploring-projects-on-github/contributing-to-a-project

### 4.3 更新记录

| 版本 | 日期 | 作者 | 变更说明 |
|------|------|--------|----------|
| 1.0 | 2026-05-23 | AI Agent (Week 2) | 初始版本 |

---

**声明**：本文档旨在帮助开发者和用户理解杀毒软件误报问题，并提供合法合规的解决方案。请勿将本文档用于恶意目的。

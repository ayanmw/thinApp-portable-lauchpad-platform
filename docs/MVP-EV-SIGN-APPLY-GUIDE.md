# MVP EV 代码签名证书申请指南

## 1. 证书选择对比

| 证书提供商 | 证书名称 | 价格（年） | 支持算法 | Token 类型 | 时间戳服务器 |
|-----------|---------|-----------|----------|------------|-------------|
| **Sectigo** | EV Code Signing Certificate | $299-399/年 | SHA-256 | USB Token (FIPS 140-2) | http://timestamp.sectigo.com |
| **DigiCert** | EV Code Signing Certificate | $299-399/年 | SHA-256 | USB Token (FIPS 140-2) | http://timestamp.digicert.com |
| **GlobalSign** | EV Code Signing Certificate | $299-399/年 | SHA-256 | USB Token (FIPS 140-2) | http://timestamp.globalsign.com |

### 对比总结

- **Sectigo**：性价比高，申请流程相对简单，支持中国公司申请
- **DigiCert**：品牌知名度高，技术支持好，但价格稍贵
- **GlobalSign**：亚洲市场支持好，中文技术支持

**推荐**：对于 MVP 项目，建议选择 **Sectigo EV 代码签名证书**，性价比高且支持中国公司申请。

---

## 2. 申请流程详细步骤

### 2.1 准备工作

**所需材料：**
1. **公司营业执照**（扫描件，加盖公章）
2. **申请人身份证**（扫描件）
3. **公司电话账单**（最近 3 个月，证明公司地址）
4. **申请人电话**（公司座机或法人手机）
5. **DUNS 号码**（邓白氏编码，可选但推荐）

### 2.2 申请步骤

| 步骤 | 操作 | 说明 | 预计时间 |
|------|------|------|----------|
| 1 | 选择证书提供商 | 访问官网或联系代理商 | 1 小时 |
| 2 | 填写申请表 | 提供公司信息、联系人信息 | 1 小时 |
| 3 | 上传验证材料 | 营业执照、身份证、电话账单等 | 1 小时 |
| 4 | 支付费用 | 信用卡或银行转账 | 即时 |
| 5 | 公司身份验证 | CA 机构验证公司真实性（电话回访） | 1-3 工作日 |
| 6 | 电话验证 | CA 机构拨打公司电话验证申请人身份 | 1-2 工作日 |
| 7 | 证书审批 | CA 机构最终审批 | 1-2 工作日 |
| 8 | USB Token 制作 | 将证书写入 USB Token | 3-5 工作日 |
| 9 | Token 邮寄 | 快递邮寄 USB Token | 3-7 工作日 |

### 2.3 详细流程说明

#### 步骤 5：公司身份验证
- CA 机构会通过电话、邮件或第三方数据库验证公司真实性
- 确保公司电话账单上的号码与申请表中填写的号码一致
- 确保公司地址与营业执照上的地址一致

#### 步骤 6：电话验证
- CA 机构会拨打申请表中的公司电话
- 接听人需要能够确认申请人的身份
- 建议提前告知接听人可能收到的验证电话

#### 步骤 8：USB Token 制作
- EV 代码签名证书必须存储在 FIPS 140-2 Level 2 或更高标准的 USB Token 中
- Token 通常预装了证书管理软件
- 每个 Token 有 PIN 码，需要妥善保管

---

## 3. 费用预算

### 3.1 证书费用

| 证书类型 | 1 年 | 2 年 | 3 年 |
|---------|------|------|------|
| Sectigo EV Code Signing | $299 | $549 | $799 |
| DigiCert EV Code Signing | $399 | $699 | $999 |
| GlobalSign EV Code Signing | $299 | $549 | $799 |

**推荐**：购买 **1 年证书**，MVP 阶段验证流程即可。

### 3.2 时间戳服务器费用

**免费选项（推荐）：**
- Sectigo：http://timestamp.sectigo.com
- DigiCert：http://timestamp.digicert.com
- GlobalSign：http://timestamp.globalsign.com

**付费选项：**
- 自建时间戳服务器：$500-1000/年（不推荐，MVP 阶段无需）

### 3.3 总费用预算

| 项目 | 费用 | 说明 |
|------|------|------|
| EV 代码签名证书（1 年） | $299-399 | 根据选择的提供商 |
| USB Token 邮寄费 | $20-50 | 国际快递费用 |
| **总计** | **$319-449** | 约 ¥2300-3200（汇率 7.2） |

---

## 4. 时间周期

### 4.1 完整时间线

| 阶段 | 预计时间 | 说明 |
|------|----------|------|
| 准备材料 | 1-2 工作日 | 收集营业执照、电话账单等 |
| 提交申请 | 1 小时 | 在线填写申请表 |
| 公司身份验证 | 1-3 工作日 | CA 机构验证公司真实性 |
| 电话验证 | 1-2 工作日 | CA 机构电话验证申请人 |
| 证书审批 | 1-2 工作日 | CA 机构最终审批 |
| USB Token 制作 | 3-5 工作日 | 证书写入 Token |
| Token 邮寄 | 3-7 工作日 | 国际快递 |
| **总计** | **10-21 工作日** | 约 2-4 周 |

### 4.2 加急选项

**Sectigo 加急服务：**
- 加急验证：额外 $100，可缩短至 5-7 工作日
- 加急邮寄：DHL Express，额外 $50，2-3 工作日到达

**推荐**：如果 MVP 发布时间紧迫，建议选择加急服务。

---

## 5. 替代方案

### 5.1 OV 代码签名证书（组织验证）

**适用场景：**
- 预算有限（价格约为 EV 证书的 50%）
- 不需要立即获得 SmartScreen 声誉
- 可以接受用户看到"未知发布者"警告

**优缺点对比：**

| 项目 | EV 代码签名 | OV 代码签名 |
|------|-------------|-------------|
| 价格 | $299-399/年 | $99-199/年 |
| SmartScreen 声誉 | 立即获得 | 需要累积（可能数月） |
| 用户警告 | 无（绿色签名） | 有（未知发布者） |
| 申请难度 | 高（严格验证） | 低（简单验证） |
| Token 要求 | 必须 USB Token | 可选（软件证书） |

**推荐**：MVP 阶段如果预算有限，可以先使用 OV 代码签名证书，后续升级到 EV。

### 5.2 自签名证书（测试用）

**适用场景：**
- 内部测试
- 开发阶段
- 不需要分发给外部用户

**创建方法：**
```powershell
# 创建自签名证书（仅用于测试）
$cert = New-SelfSignedCertificate -Type CodeSigningCert `
    -Subject "CN=AI ThinApp Portable Launchpad (Test)" `
    -CertStoreLocation "Cert:\CurrentUser\My" `
    -NotAfter (Get-Date).AddYears(1)

# 导出证书（包含私钥）
$password = ConvertTo-SecureString -String "YourPassword" -Force -AsPlainText
Export-PfxCertificate -Cert $cert -FilePath ".\test_cert.pfx" -Password $password
```

**限制：**
- 用户会看到"未知发布者"警告
- 杀毒软件可能标记为可疑文件
- 无法通过 SmartScreen 验证

### 5.3 不签名（仅限测试）

**适用场景：**
- 内部测试
- 开发阶段
- 用户可以手动"解除锁定"

**操作方法（Windows）：**
1. 右键点击可执行文件 → 属性
2. 在"常规"选项卡底部，勾选"解除锁定"
3. 点击"应用" → "确定"

**限制：**
- 用户体验差
- 企业环境可能被组策略阻止
- 无法通过网络分发（浏览器会阻止下载）

---

## 6. 申请建议

### 6.1 推荐申请流程

1. **立即开始准备材料**（今天）
   - 收集营业执照、电话账单
   - 确认公司电话可接听

2. **选择证书提供商**（明天）
   - 推荐：Sectigo（性价比高）
   - 备选：GlobalSign（中文支持）

3. **提交申请**（明天）
   - 在线填写申请表
   - 上传验证材料

4. **跟进验证进度**（每周）
   - 及时接听验证电话
   - 回复 CA 机构的邮件

5. **收到 Token 后测试**（2-4 周后）
   - 安装 Token 驱动
   - 测试签名流程

### 6.2 注意事项

- ⚠️ **提前申请**：EV 证书申请周期长（2-4 周），务必提前申请
- ⚠️ **电话畅通**：确保公司电话在申请期间畅通
- ⚠️ **妥善保管 Token**：USB Token 是签名的关键，丢失后需要重新申请
- ⚠️ **PIN 码安全**：Token 的 PIN 码不要泄露

---

## 7. 参考链接

- **Sectigo EV 代码签名**：https://sectigo.com/ev-code-signing
- **DigiCert EV 代码签名**：https://www.digicert.com/ev-code-signing
- **GlobalSign EV 代码签名**：https://www.globalsign.com/ev-code-signing
- **微软代码签名要求**：https://learn.microsoft.com/en-us/windows/win32/seccrypto/cryptography-tools
- **Signtool.exe 使用指南**：https://learn.microsoft.com/en-us/dotnet/framework/tools/signtool-exe

---

**文档版本**：v1.0  
**创建日期**：2026-05-24  
**更新日期**：2026-05-24  
**作者**：MVP Code Sign Engineer

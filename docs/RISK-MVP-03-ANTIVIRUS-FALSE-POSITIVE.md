# 技术风险详述 - R-MVP-03（杀软误报率高）

## 风险描述

Hook 引擎（DLL 注入 + Inline Hook）是杀毒软件的典型"恶意行为"，会导致：
- 主流杀软（Windows Defender、360、火绒等）报毒
- 用户无法正常运行应用（被拦截或删除）
- 应用商店上架困难（微软 Store 拒绝签名应用）

## 影响范围

- **影响用户**：所有用户（尤其是企业用户）
- **影响功能**：Hook 引擎、Launchpad 主程序
- **影响指标**：杀软误报率、用户留存率

## 概率评估

- **技术难度**：低（代码签名可解决大部分误报）
- **类似项目参考**：Sandboxie Plus 初期误报率 > 50%，签名后降至 < 5%
- **概率**：**高**（70-90% 概率在 MVP 阶段误报率 > 10%）

## 缓释措施（按优先级排序）

### P0（必须实现，MVP Phase 1 完成）

**措施 1：申请 OV 代码签名证书**
- 证书类型：OV（Organization Validation，约 $200-400/年）
- 证书用途：签名 Hook DLL、Launchpad 可执行文件
- 预期误报率降低：50% → 20%

**措施 2：提交白名单申请**
- 向主流杀软厂商提交白名单申请（Windows Defender、360、火绒等）
- 提供源码审计报告和公司资质
- 预期误报率降低：20% → 10%

### P1（建议实现，MVP Phase 2 完成）

**措施 3：申请 EV 代码签名证书**
- 证书类型：EV（Extended Validation，约 $300-600/年）
- 优势：微软 SmartScreen 立即信任（无需累积声誉）
- 预期误报率降低：10% → 5%

**措施 4：开源透明（GitHub 完整源码）**
- 公开 Hook 引擎源码（让用户和社区审计）
- 提供构建脚本（用户可自己编译）
- 预期误报率降低：5% → 2%

### P2（可选实现，MVP Phase 3 完成）

**措施 5：使用虚拟机隔离（可选功能）**
- 提供"高级模式"（在轻量级虚拟机中运行应用）
- 避免 Hook 注入（彻底消除误报）
- 预期误报率降低：2% → 0%（高级模式）

## 应急预案（若缓释措施失效）

1. **降级方案**：提供"兼容模式"（用户自己添加杀软排除项）
2. **回滚方案**：若误报率无法降低，开源整个项目（用户自编译）
3. **用户沟通**：在文档中明确说明如何添加杀软排除项

## 负责人与截止日期

- **负责人**：Sec Lead + PM
- **P0 措施截止日期**：2026-06-15（MVP Phase 1 中期）
- **P1 措施截止日期**：2026-07-22（MVP Phase 2 结束前）
- **P2 措施截止日期**：2026-08-12（MVP Phase 3 结束前）

## 监控指标

- VirusTotal 扫描误报率（目标：≤ 10%，即 50+ 引擎中 ≤ 5 个报毒）
- 主流杀软（Windows Defender、360、火绒）误报（目标：0）
- 用户反馈的误报问题数量（目标：< 5 个/月）

若任意指标连续 3 次测试不达标，触发应急预案。

## 测试计划

| 测试阶段 | 测试方法 | 验收标准 |
|----------|----------|----------|
| 代码签名前 | 上传到 VirusTotal | 误报率 < 50% |
| OV 签名后 | 上传到 VirusTotal | 误报率 < 20% |
| 白名单申请后 | 上传到 VirusTotal | 误报率 < 10% |
| EV 签名后 | 上传到 VirusTotal | 误报率 < 5% |
| 开源后 | 上传到 VirusTotal | 误报率 < 2% |

## 代码签名证书申请流程

1. **选择 CA（Certificate Authority）**
   - 推荐：Sectigo（原 Comodo）、DigiCert、GlobalSign
   - 价格：OV 约 $200-400/年，EV 约 $300-600/年

2. **准备材料**
   - 公司营业执照（或个人身份证）
   - 地址证明（水电费账单或银行对账单）
   - 电话验证（CA 会拨打公司电话确认身份）

3. **申请证书**
   - 在线填写申请表
   - 上传材料
   - 等待审核（OV 约1-3 天，EV 约 3-7 天）

4. **安装证书**
   - 收到 CA 的邮件，下载证书
   - 安装到 Windows 证书存储（或 USB Token）
   - 使用 `signtool.exe` 签名文件

5. **签名文件**
   ```powershell
   # 签名 Hook DLL
   signtool sign /fd SHA256 /tr http://timestamp.digicert.com /td SHA256 /s my /n "公司名称" HookEngine.dll
   
   # 签名 Launchpad 可执行文件
   signtool sign /fd SHA256 /tr http://timestamp.digicert.com /td SHA256 /s my /n "公司名称" Launchpad.exe
   ```

## 白名单申请流程

| 杀软厂商 | 申请网址 | 预计周期 |
|----------|----------|----------|
| Windows Defender | https://www.microsoft.com/en-us/wdsi/filesubmission | 3-7 天 |
| 360 | https://cloud.360.cn/submit | 7-14 天 |
| 火绒 | https://www.huorong.cn/submit.html | 7-14 天 |
| 腾讯电脑管家 | https://guanjia.qq.com/submit.html | 7-14 天 |
| 金山毒霸 | https://www.ijinshan.com/submit.html | 14-30 天 |

## 参考文献

- Microsoft SmartScreen 和 EV 代码签名：https://docs.microsoft.com/en-us/windows/security/threat-protection/microsoft-defender-smartscreen/
- VirusTotal 上传地址：https://www.virustotal.com/gui/home/upload
- 代码签名最佳实践：https://www.digicert.com/kb/code-signing/

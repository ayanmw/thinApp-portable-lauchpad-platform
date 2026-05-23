# 常见问题解答 (FAQ)

> 本文档收集 AI ThinApp Portable Launchpad Platform 项目的最常见问题，并给出详细解答。
> 如果你的问题未在此列出，请到 [GitHub Discussions](https://github.com/anmingwei/ai-thinApp-portable-lauchpad-platform/discussions) 提问。

---

## 目录

- [产品类](#产品类)
- [技术类](#技术类)
- [安全类](#安全类)
- [贡献类](#贡献类)

---

## 产品类

### Q1. 这个项目和 Sandboxie 有什么区别？

**A**：核心区别在于**沙箱粒度**和**便携化理念**：

| 对比项 | AI ThinApp Portable Launchpad | Sandboxie Plus |
|--------|-------------------------------|----------------|
| 沙箱粒度 | **每个软件独立沙箱** | 多软件共享一个沙箱 |
| 便携化 | ✅ 核心目标：应用目录可移动 | ❌ 不支持（沙箱位置固定） |
| 重装系统恢复 | ✅ 复制目录即可 | ❌ 需要导出/导入沙箱 |
| 开始菜单替换 | ✅ 计划支持（V3） | ❌ 不支持 |
| 应用商店 | ✅ 计划支持（MVP） | ❌ 不支持 |
| 协议 | GPL v3（开源） | GPL v3（开源） |

**总结**：如果你需要"每个软件完全独立、可移动、重装系统不丢配置"，选本项目；如果你只需要"隔离软件运行，防止污染系统"，选 Sandboxie。

### Q2. 支持哪些 Windows 版本？

**A**：计划支持以下版本：

| Windows 版本 | 支持状态 | 说明 |
|--------------|----------|------|
| Windows 10 22H2+ | ✅ 计划支持（POC 验证中） | POC 阶段主要测试版本 |
| Windows 11 23H2+ | ✅ 计划支持（POC 验证中） | POC 阶段主要测试版本 |
| Windows 10 21H2 及更早 | ⚠️ 未测试，可能兼容 | 不保证兼容 |
| Windows 8.1/8 | ❌ 不支持 | 市场份额太小，放弃支持 |
| Windows 7 | ❌ 不支持 | 已停止支持，安全风险高 |
| Windows XP 及更早 | ❌ 不支持 | 架构差异大，无法支持 |

**注意**：本项目是开源项目，社区可以提交 PR 添加对其他版本的支持。

### Q3. 是否需要管理员权限运行？

**A**：**部分功能需要**，具体如下：

| 功能 | 需要管理员权限？ | 原因 |
|------|-------------------|------|
| Hook 引擎（注入 DLL） | ✅ 是 | 注入 DLL 到其他进程需要 `SeDebugPrivilege` 权限 |
| 文件系统重定向 | ❌ 否（大部分情况） | 重定向在用户态完成，不需要管理员权限 |
| 注册表重定向 | ❌ 否（大部分情况） | 同文件系统重定向 |
| 应用捕获（快照） | ✅ 是 | 需要读取系统目录（如 `C:\Windows`）和注册表 `HKLM` |
| 软件商店（下载/安装） | ❌ 否 | 仅写入用户目录，不需要管理员权限 |

**建议**：日常使用以普通用户权限运行即可；需要应用捕获时，再以管理员权限运行。

### Q4. 会影响系统性能吗？

**A**：**会有一定影响，但目标是控制在可接受范围内**。

| 操作 | 性能损耗（目标） | 说明 |
|------|-------------------|------|
| 文件读取（CoW 检查） | < 5% | POC 目标 < 15%，MVP 目标 < 5% |
| 文件写入（重定向） | < 10% | POC 目标 < 15%，MVP 目标 < 10% |
| 注册表读取/写入 | < 5% | 同文件操作 |
| 进程创建（注入 Hook DLL） | < 15% | 仅创建进程时有损耗，运行后无影响 |
| Launchpad 启动 | < 2 秒 | 目标启动时间 |
| 应用启动（沙箱模式 vs 真实模式） | < 10% | 目标启动时间增幅 |

**注意**：
- POC 阶段不优化性能，损耗可能较高（> 50%），**请勿用于生产环境**
- MVP 阶段会进行热路径优化（缓存、异步 IO、批量操作）
- 如果性能损耗无法接受，可以在应用设置中关闭部分 Hook（如仅重定向注册表，不 Hook 文件操作）

### Q5. 重装系统后如何恢复所有应用？

**A**：这是本项目的核心优势之一！步骤如下：

1. **重装系统前**：将应用目录（如 `D:\ThinAppApps\`）备份到外部硬盘或 U 盘
2. **重装系统后**：
   - 安装 AI ThinApp Portable Launchpad Platform
   - 将备份的应用目录复制回原位置（如 `D:\ThinAppApps\`）
   - 打开 Launchpad，所有应用会自动识别，**无需重新安装**
3. **验证**：启动应用，确认配置、数据都在（因为它们都保存在应用目录内）

**原理**：本项目将所有应用的状态（配置、缓存、数据文件）都保存在应用目录内，不写入系统盘（C 盘），因此重装系统不影响应用状态。

---

## 技术类

### Q1. Hook 引擎是如何工作的？

**A**：本项目使用 **MinHook** 框架实现用户态 Inline Hook，原理如下：

#### Hook 安装流程
```
1. 保存原始函数地址（如 NtCreateFile 的地址）
2. 修改原始函数的前几个字节，写入跳转指令（JMP）跳转到我们的钩子函数
3. 当应用调用 NtCreateFile 时，实际执行的是我们的钩子函数
```

#### 钩子函数逻辑
```cpp
// 伪代码
NTSTATUS HOOK_NtCreateFile(...) {
    // 1. 检查目标路径是否需要重定向
    if (NeedRedirect(ObjectAttributes)) {
        // 2. 修改路径参数（重定向到沙箱路径）
        RedirectObjectAttributes(ObjectAttributes);
    }

    // 3. 调用原始函数（使用原始路径或重定向后的路径）
    return OriginalNtCreateFile(...);
}
```

#### Hook 卸载流程
```
1. 恢复原始函数的前几个字节（还原跳转指令）
2. 应用后续调用 NtCreateFile 时，将直接执行原始函数
```

**注意**：
- Hook 在用户态完成，不需要内核驱动（POC/MVP 阶段）
- Hook 可能被其他软件覆盖（如杀软、其他 Hook 工具），本项目会定期检查并重新安装 Hook

### Q2. 文件系统重定向的原理是什么？

**A**：通过 Hook 文件 API（`NtCreateFile`, `NtWriteFile`, `NtReadFile`, `NtOpenFile`），修改文件路径参数，将应用对系统目录的读写重定向到沙箱目录。

#### 重定向规则示例
| 真实路径（应用视角） | 虚拟路径（实际存储） |
|---------------------|---------------------|
| `C:\Windows\temp\app.log` | `{AppDir}\VFS\C\Windows\temp\app.log` |
| `C:\Program Files\App\config.ini` | `{AppDir}\VFS\C\Program Files\App\config.ini` |
| `%APPDATA%\App\settings.json` | `{AppDir}\VFS\AppData\Roaming\App\settings.json` |

#### Copy-on-Write (CoW) 逻辑
```
读取文件 X:
  1. 检查 {AppDir}\VFS\...X... 是否存在
  2. 若存在 → 读取虚拟文件（应用已修改过的版本）
  3. 若不存在 → 读取真实文件 X（透传）

写入文件 X:
  1. 若 X 在拦截路径列表内 → 重定向到 {AppDir}\VFS\...X...
  2. 若 X 不在拦截路径列表内 → 透传（允许写入真实系统，可配置）
```

**优势**：
- 应用对系统的修改完全隔离在沙箱目录内，不污染系统
- 读取时优先读沙箱文件，沙箱不存在则透传真实文件（兼容性好）

### Q3. 注册表重定向的原理是什么？

**A**：与文件系统重定向类似，通过 Hook 注册表 API（`NtCreateKey`, `NtSetValueKey`, `NtOpenKey`），将应用对注册表的写入重定向到虚拟 hive 文件。

#### 重定向规则示例
| 真实路径（应用视角） | 虚拟路径（实际存储） |
|---------------------|---------------------|
| `HKLM\Software\App` | `registry.hive\HKLM\Software\App` |
| `HKCU\Software\App` | `registry.hive\HKCU\Software\App` |

#### CoW 逻辑（同 VFS）
- 读取时先查虚拟 hive，不存在则透传真实注册表
- 写入时重定向到虚拟 hive

**注意**：
- 虚拟 hive 格式：MVP 阶段使用 JSON 格式（简单，人类可读），V2 阶段迁移到二进制 hive 格式（性能优化）
- 虚拟 hive 存储位置：`{AppDir}\VFS\registry.hive`

### Q4. 如何保证子进程也受 Hook 约束？

**A**：通过 Hook 进程创建 API（`CreateProcessInternalW`, `ShellExecuteExW`），在创建子进程时注入 Hook DLL，使子进程也加载我们的 Hook 逻辑。

#### 子进程 Hook 继承流程
```
父进程启动子进程:
  1. 拦截 CreateProcessInternalW
  2. 创建挂起子进程 (CREATE_SUSPENDED)
  3. 在子进程中注入 Hook DLL (远程线程)
  4. 恢复子进程执行
  5. 子进程 DllMain 中自动安装 Hook → 继承完成
```

#### 支持的进程创建方式
| 创建方式 | 是否支持 | 说明 |
|----------|----------|------|
| `CreateProcess` / `CreateProcessAsUser` | ✅ 支持 | 直接 Hook `CreateProcessInternalW` |
| `ShellExecute` / `ShellExecuteEx` | ✅ 支持 | Hook `ShellExecuteExW` |
| `WinExec`（已废弃，但仍有应用使用） | ✅ 支持 | 内部调用 `CreateProcess` |
| COM 对象创建（`CoCreateInstance`） | ⚠️ V2 支持 | 需要 Hook COM 相关 API |
| .NET `Process.Start` | ✅ 支持 | 底层调用 `CreateProcess` |

**注意**：
- 支持 2+ 层子进程继承（覆盖 Chrome 多进程等场景）
- 注入方式有多种（远程线程、AppInit_DLLs、`SetWindowsHookEx`），本项目使用远程线程注入（兼容性最好）

### Q5. 直接使用 Syscall 的应用如何处理？

**A**：**POC/MVP 阶段无法处理，V2 阶段通过内核驱动解决**。

#### 问题原因
- 部分应用（如反作弊系统、杀毒软件、高级调试工具）会直接调用 `syscall` 指令，绕过 `ntdll.dll` 的导出函数
- 用户态 Hook（如 MinHook）只能 Hook `ntdll.dll` 的导出函数，无法拦截直接 Syscall

#### 解决方案
| 阶段 | 方案 | 说明 |
|------|------|------|
| POC/MVP | 接受限制（不处理直接 Syscall） | 在文档中说明限制；对受影响的应用，建议以真实模式运行 |
| V2 | 增加内核 Minifilter 驱动 | 在内核层拦截文件/注册表操作，不受直接 Syscall 影响 |
| V2+ | 结合用户态 Hook + 内核驱动 | 用户态 Hook 处理大部分应用，内核驱动兜底处理直接 Syscall |

**注意**：
- 直接使用 Syscall 的应用占少数（主要是安全软件、游戏反作弊），大部分普通应用不会这样做
- 如果应用因直接 Syscall 导致兼容性问题，可以在应用设置中关闭 Hook（以真实模式运行）

---

## 安全类

### Q1. 会不会被杀软误报？

**A**：**可能会**，因为本项目使用 API Hook 技术（类似病毒行为）。但我们采取了以下措施减少误报：

| 措施 | 说明 |
|------|------|
| 代码签名 | 申请 EV 代码签名证书（~$200-400/年），对可执行文件签名，建立信誉 |
| 开源透明 | 所有代码在 GitHub 公开，任何人都可以审计，证明无恶意行为 |
| 申请白名单 | 与杀软厂商合作，将本项目加入白名单（周期 3-6 个月） |
| 提示用户 | 首次运行时提示用户"请允许 ThinApp.exe 运行，否则沙箱功能无法使用" |

#### 遇到误报怎么办？
1. **添加信任**：将 `ai-thinapp.exe` 和 `hook_engine.dll` 添加到杀软白名单
2. **报告误报**：到 [GitHub Issues](https://github.com/anmingwei/ai-thinApp-portable-lauchpad-platform/issues) 报告，我们会联系杀软厂商解决
3. **临时关闭杀软**：如果杀软不允许添加白名单，可以临时关闭杀软（不推荐，仅测试用）

### Q2. 代码是开源的吗？

**A**：**是的，完全开源**。

- **协议**：GNU General Public License v3.0 (GPL v3)
- **仓库**：[https://github.com/anmingwei/ai-thinApp-portable-lauchpad-platform](https://github.com/anmingwei/ai-thinApp-portable-lauchpad-platform)
- **含义**：
  - 任何人都可以自由使用、修改、分发本项目的代码
  - 衍生项目也必须采用 GPL v3 协议开源
  - 你可以用于商业用途，但衍生项目必须开源

**为什么选择 GPL v3？**
- 确保代码永远开源，任何人都可以自由使用、修改、分发
- 防止有人将代码闭源商用，损害社区利益
- 与 MinHook（MIT 协议）兼容（MIT 协议允许与 GPL 协议代码链接）

### Q3. 我的数据会被上传吗？

**A**：**不会**。本项目不收集任何用户数据。

| 功能 | 是否上传数据 | 说明 |
|------|--------------|------|
| 应用虚拟化引擎 | ❌ 否 | 完全在本地运行，不联网 |
| Launchpad 客户端 | ❌ 否（默认） | 仅在使用"软件商店"功能时联网（下载 .vapp 包） |
| 软件商店 | ✅ 是（仅下载） | 从社区服务器下载 .vapp 包，不上传数据 |
| 云端同步（V2） | ✅ 是（可选） | 用户数据加密后同步到用户指定的云盘/NAS，我们无权访问 |

**隐私政策**：
- 我们不会收集你的个人数据（如姓名、邮箱、位置）
- 我们不会上传你的应用数据（如配置文件、缓存）
- 云端同步功能（V2）是**可选的**，且数据加密存储，我们无权访问

### Q4. 如何验证发布文件的完整性？

**A**：我们使用 **SHA-256 校验和** 和 **GPG 签名** 确保发布文件未被篡改。

#### 验证步骤
1. **下载发布文件**：从 [GitHub Releases](https://github.com/anmingwei/ai-thinApp-portable-lauchpad-platform/releases) 下载可执行文件（如 `ai-thinapp-v0.1.0-setup.exe`）
2. **下载校验和文件**：同一页面下载 `SHA256SUMS` 文件
3. **计算本地文件的 SHA-256 校验和**：
   ```bat
   certutil -hashfile ai-thinapp-v0.1.0-setup.exe SHA256
   ```
4. **比对校验和**：打开 `SHA256SUMS` 文件，查找 `ai-thinapp-v0.1.0-setup.exe` 对应的校验和，与本地计算的校验和比对
   - ✅ 一致：文件完整，未被篡改
   - ❌ 不一致：**不要运行**，立即删除，并到 [GitHub Issues](https://github.com/anmingwei/ai-thinApp-portable-lauchpad-platform/issues) 报告
5. **（可选）验证 GPG 签名**：
   ```bat
   gpg --verify SHA256SUMS.sig SHA256SUMS
   ```
   - 需要导入我们的 GPG 公钥（在仓库 `docs/GPG-PUBLIC-KEY.asc`）

**注意**：
- 如果校验和不一致，说明文件可能被篡改（如中间人攻击、镜像站被入侵）
- 请仅从官方 GitHub Releases 下载发布文件，不要从第三方网站下载

---

## 贡献类

### Q1. 如何报告 Bug？

**A**：请按以下步骤报告 Bug：

1. **检查现有 Issue**：先在 [GitHub Issues](https://github.com/anmingwei/ai-thinApp-portable-lauchpad-platform/issues) 中搜索，确认 Bug 未被报告
2. **创建新 Issue**：
   - 点击"New Issue"
   - 选择"Bug Report"模板
   - 填写以下信息：
     - **复现步骤**（尽量详细）
     - **预期行为 vs 实际行为**
     - **环境信息**（Windows 版本、编译器版本、Commit Hash）
     - **相关日志或截图**（如果有）
3. **等待回复**：维护者会在 3-5 个工作日内回复

**示例 Bug Report**：
```
标题：Hook 安装失败（Windows 11 23H2）

复现步骤：
1. 以管理员权限运行 ai-thinapp.exe
2. 启动 Notepad++
3. 查看日志（hook_engine.log）

预期行为：Hook 安装成功，文件写入被重定向
实际行为：Hook 安装失败，日志显示"MinHook 初始化失败"

环境信息：
- Windows 11 23H2 (22631.3527)
- Visual Studio 2022 17.8.3
- Commit: abc123

日志片段：
[2026-05-23 16:30:15] [HookEngine] MinHook 初始化失败：错误代码 0x80070005
```

### Q2. 如何贡献代码？

**A**：请按以下步骤贡献代码：

1. **Fork 仓库**：点击 GitHub 页面右上角"Fork"按钮
2. **创建分支**：
   ```bash
   git checkout -b feat/your-feature-name  # 新功能
   git checkout -b fix/your-bug-fix       # Bug 修复
   ```
3. **提交更改**：
   - 遵循 [Conventional Commits](https://www.conventionalcommits.org/) 规范
   - 示例：`feat(hook): 添加对 NtDeleteFile 的 Hook 支持`
4. **推送分支**：`git push origin feat/your-feature-name`
5. **创建 Pull Request**：
   - 填写 PR 模板（描述、关联 Issue、测试计划）
   - 等待 Code Review
   - 根据 Review 意见修改

**代码规范**：
- 遵循 [C++ 编码规范](DEV-GUIDE.md#41-c-编码规范)
- 添加单元测试
- 更新相关文档

### Q3. 如何添加新的应用打包配置？

**A**：应用打包配置（`.vapp` 包）可以通过两种方式添加：

#### 方法一：使用应用捕获工具（MVP 阶段提供）
1. **启动应用捕获工具**：`tools\app_capture.exe`
2. **拍摄前快照**：工具自动扫描文件系统（C:\Windows, C:\Program Files, %APPDATA% 等）和注册表
3. **安装应用**：手动安装目标应用（或静默安装）
4. **拍摄后快照**：工具自动扫描差异
5. **生成 .vapp 包**：工具将差异打包为 `.vapp` 文件
6. **测试 .vapp 包**：在 Launchpad 中安装并运行，确认功能正常

#### 方法二：手动创建配置（适合高级用户）
1. **创建配置目录**：在 `community/` 目录下创建 `{app-name}\` 目录
2. **创建配置文件**：在 `{app-name}\` 目录中创建 `manifest.json`：
   ```json
   {
     "name": "Notepad++",
     "version": "8.6.4",
     "author": "Your Name",
     "description": "Notepad++ 便携化配置",
     "homepage": "https://notepad-plus-plus.org/",
     "download_url": "https://github.com/notepad-plus-plus/notepad-plus-plus/releases/download/v8.6.4/npp.8.6.4.portable.x64.zip",
     "dependencies": [],
     "files": [
       {
         "src": "C:\\Program Files\\Notepad++\\*",
         "dest": "{AppDir}\\VFS\\C\\Program Files\\Notepad++\\*"
       }
     ],
     "registry": [
       {
         "src": "HKCU\\Software\\Notepad++",
         "dest": "{AppDir}\\VFS\\registry.hive\\HKCU\\Software\\Notepad++"
       }
     ]
   }
   ```
3. **提交 Pull Request**：将配置目录提交到本仓库，维护者审核后合并

### Q4. 协议允许商业使用吗？

**A**：**允许，但有条件**。

- **GPL v3 协议**：允许商业使用，但衍生项目必须采用 GPL v3 协议开源
- **示例**：
  - ✅ **允许**：公司 A 使用本项目代码，修改后用于内部部署（不需要开源，因为不分发）
  - ✅ **允许**：公司 B 使用本项目代码，修改后作为产品销售（**需要开源修改后的代码**）
  - ❌ **不允许**：公司 C 使用本项目代码，修改后作为闭源商业产品销售（违反 GPL v3）

**企业版计划**：
- 我们计划提供企业版（闭源，商业协议），但需要重新实现核心引擎（不使用 GPL v3 代码）
- 如果你对企业版感兴趣，请到 [GitHub Discussions](https://github.com/anmingwei/ai-thinApp-portable-lauchpad-platform/discussions) 联系我们

---

## 未涵盖的问题？

如果你的问题未在此 FAQ 中列出，请通过以下渠道提问：

- **GitHub Discussions**：[https://github.com/anmingwei/ai-thinApp-portable-lauchpad-platform/discussions](https://github.com/anmingwei/ai-thinApp-portable-lauchpad-platform/discussions)（推荐）
- **GitHub Issues**：[https://github.com/anmingwei/ai-thinApp-portable-lauchpad-platform/issues](https://github.com/anmingwei/ai-thinApp-portable-lauchpad-platform/issues)（Bug 报告/功能请求）

---

**最后更新**：2026-05-23  
**维护者**：AI ThinApp Portable Launchpad Platform 开源社区

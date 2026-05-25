# AI ThinApp Portable Launchpad — 快速开始指南

> 版本：v0.1.0 MVP
> 更新日期：2026-05-24
> 5 分钟上手，6 步完成首个应用安装和启动

---

## 第 1 步：下载安装包 📥

1. 打开浏览器，访问 [GitHub Releases](https://github.com/anmingwei/ai-thinApp-portable-launchpad-platform/releases)
2. 找到最新版本（如 v0.1.0）
3. 下载 `ai-thinapp-v0.1.0-setup.exe`（安装程序）
4. 保存到桌面或下载文件夹

> [截图占位：GitHub Releases 页面]
> 应截取：GitHub Releases 页面，展示最新版本的下载链接

## 第 2 步：安装 🛠️

1. **双击安装包** `ai-thinapp-v0.1.0-setup.exe`
2. 如果弹出"用户账户控制"提示，点击"**是**"
3. 同意许可协议，点击"下一步"
4. 选择安装路径（推荐默认 `C:\Program Files\AI ThinApp\`），点击"安装"
5. 等待安装完成，勾选"启动 AI ThinApp Launchpad"，点击"完成"

> [截图占位：安装向导界面]
> 应截取：安装向导的欢迎页面，显示版本号和许可协议

## 第 3 步：添加杀软白名单 🛡️

> **重要**：由于 AI ThinApp 使用沙箱引擎技术，可能被杀毒软件误报。请先添加白名单。

**Windows Defender**：
1. 打开"Windows 安全中心" → "病毒和威胁防护"
2. 点击"管理设置" → "排除项" → "添加排除项" → "文件夹"
3. 选择 `C:\Program Files\AI ThinApp\`

**其他杀软**：在设置中找到"白名单"或"信任列表"，将 `ai-thinapp.exe` 和 `hook_engine.dll` 添加进去。

> [截图占位：Windows Defender 排除项设置]
> 应截取：Windows 安全中心中添加排除项的界面

## 第 4 步：启动 Launchpad 🚀

1. 双击桌面的"AI ThinApp Launchpad"快捷方式
2. **首次启动建议以管理员身份运行**（右键 → 以管理员身份运行）
3. Launchpad 主界面打开，看到"我的应用"列表（初始为空）

> [截图占位：Launchpad 主界面]
> 应截取：Launchpad 主界面，展示左侧导航栏、应用列表（空状态）、搜索框

## 第 5 步：添加首个应用 📦

**方法一：从软件商店安装**（推荐）
1. 点击左侧"软件商店"
2. 浏览热门应用或搜索应用名称
3. 点击"安装"，等待下载完成

**方法二：添加本地应用**
1. 点击右上角"+ 添加应用"
2. 选择本地应用的可执行文件（如 `notepad++.exe`）
3. 点击"确认"

> [截图占位：软件商店界面]
> 应截取：软件商店页面，展示应用列表和安装按钮

## 第 6 步：启动应用并验证沙箱效果 ✅

1. 在"我的应用"列表中，找到刚才添加的应用
2. 点击"**启动**"按钮
3. 应用以**沙箱模式**启动（默认）
4. 在应用中做一些操作（如创建文件、修改设置）
5. 关闭应用，回到 Launchpad
6. 点击应用卡片，查看"沙箱大小"——你应该能看到沙箱有数据写入

**验证成功** 🎉 你的首个便携化应用已经就绪！

> [截图占位：应用详情页 - 沙箱大小]
> 应截取：应用详情页，展示沙箱大小和数据写入情况

---

## 常用操作速查

| 操作 | 方法 |
|------|------|
| 启动应用 | 点击"启动"按钮 |
| 停止应用 | 点击"停止"按钮 |
| 切换沙箱/真实模式 | 右键应用卡片 → "启动设置" |
| 删除应用 | 右键应用卡片 → "删除应用" |
| 查看沙箱文件 | 应用详情页 → "沙箱内容浏览器" |
| 导出应用包 | 应用详情页 → "导出为 .vapp" |
| 清除沙箱数据 | 应用详情页 → "清除沙箱" |
| 打开设置 | 右上角"⚙️ 设置"图标 |

## 遇到问题？

| 问题 | 解决 |
|------|------|
| 杀软报毒 | 添加白名单（见第 3 步） |
| Hook 注入失败 | 以管理员身份运行 Launchpad |
| 应用无法启动 | 切换到"真实模式"尝试 |
| 更多帮助 | 查看 [用户手册](USER-MANUAL.md) 或 [FAQ](USER-MANUAL-FAQ.md) |

## 下一步

- **了解沙箱机制**：阅读 [用户手册第 3 章](USER-MANUAL.md#第-3-章沙箱管理)
- **学习高级功能**：阅读 [用户手册第 5 章](USER-MANUAL.md#第-5-章高级功能)
- **查看视频教程**：[视频教程脚本](VIDEO-TUTORIAL-SCRIPT.md)
- **加入社区**：[GitHub Discussions](https://github.com/anmingwei/ai-thinApp-portable-launchpad-platform/discussions)

## 更多资源

- **完整用户手册**：[USER-MANUAL.md](USER-MANUAL.md)
- **常见问题解答**：[USER-MANUAL-FAQ.md](USER-MANUAL-FAQ.md)
- **部署文档**：[DEPLOYMENT-GUIDE.md](DEPLOYMENT-GUIDE.md)
- **发布说明**：[MVP-RELEASE-NOTES.md](MVP-RELEASE-NOTES.md)
- **GitHub 仓库**：[GitHub](https://github.com/anmingwei/ai-thinApp-portable-launchpad-platform)
- **Bug 报告**：[GitHub Issues](https://github.com/anmingwei/ai-thinApp-portable-launchpad-platform/issues)
- **交流讨论**：[GitHub Discussions](https://github.com/anmingwei/ai-thinApp-portable-launchpad-platform/discussions)

---

**最后更新**：2026-05-24
**维护者**：AI ThinApp Portable Launchpad Platform 开源社区

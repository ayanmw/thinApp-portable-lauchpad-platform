# 演示视频录制说明

## 演示 1：Hook 引擎工作原理

**时长**：3-5 分钟  
**内容**：
1. 启动测试程序 `test_hook.exe`，展示 Hook 安装/卸载
2. 启动 Notepad++，展示文件操作重定向（使用 Process Monitor 监控）
3. 启动 7-Zip，展示注册表操作重定向（使用 RegEdit 监控）
4. 启动 Firefox Portable，展示子进程继承（使用 Process Monitor 监控）

**录制工具**：OBS Studio（免费）  
**输出格式**：MP4（H.264，1080p）  
**输出文件**：`hook-engine-demo.mp4`

---

## 演示 2：Launchpad 原型交互

**时长**：3-5 分钟  
**内容**：
1. 打开 Launchpad 主界面，展示应用列表
2. 点击应用图标，启动应用（展示 Hook 注入）
3. 打开应用详情页，展示沙箱状态
4. 打开应用商店，展示浏览/搜索/安装流程
5. 打开新手引导，展示 3 步引导流程

**录制工具**：OBS Studio（免费）  
**输出格式**：MP4（H.264，1080p）  
**输出文件**：`launchpad-prototype-demo.mp4`

---

## 录制检查清单

- [ ] 音频清晰（麦克风无杂音）
- [ ] 屏幕分辨率 1080p
- [ ] 操作步骤流畅（无卡顿）
- [ ] 关键界面有文字标注（使用 OBS 文本插件）
- [ ] 文件保存至 `review-package/demos/` 目录
- [ ] 视频文件命名正确（`hook-engine-demo.mp4`、`launchpad-prototype-demo.mp4`）

---

## 录制脚本示例（演示 1）

### 场景 1：Hook 安装/卸载（0:00 - 1:00）
1. 打开命令提示符（CMD）
2. 导航到 `build\Release\` 目录
3. 运行 `test_hook.exe install`（展示 Hook 安装成功消息）
4. 运行 `test_hook.exe uninstall`（展示 Hook 卸载成功消息）
5. 查看日志文件 `tests\engine\test_hook.log`（展示 Hook 安装/卸载记录）

### 场景 2：文件操作重定向（1:00 - 2:30）
1. 启动 Process Monitor（过滤 `notepad++.exe`）
2. 启动 Notepad++
3. 打开文件 `C:\Windows\Temp\test.txt`（实际重定向到 `{AppDir}\VFS\C\Windows\Temp\test.txt`）
4. 编辑文件，保存
5. 在 Process Monitor 中查看文件操作（展示路径重定向）
6. 打开 `{AppDir}\VFS\C\Windows\Temp\test.txt`（展示文件确实写入沙箱目录）

### 场景 3：注册表操作重定向（2:30 - 4:00）
1. 启动 RegEdit
2. 导航到 `HKCU\Software\Notepad++`
3. 启动 Notepad++，修改设置（如主题、字体）
4. 在 RegEdit 中查看 `HKCU\Software\Notepad++`（展示注册表操作被重定向到虚拟 hive）
5. 打开虚拟 hive 文件 `{AppDir}\VFS\Registry\user.dat`（展示注册表写入沙箱）

### 场景 4：子进程继承（4:00 - 5:00）
1. 启动 Process Monitor（过滤 `firefox.exe`）
2. 启动 Firefox Portable
3. 在 Process Monitor 中查看子进程创建（展示 Firefox 主进程启动子进程）
4. 验证子进程也加载了 Hook DLL（展示 Hook 继承）
5. 在 Firefox 中下载文件，查看下载路径（展示文件操作重定向到沙箱）

---

## 录制脚本示例（演示 2）

### 场景 1：Launchpad 主界面（0:00 - 1:00）
1. 打开 Launchpad 原型（`assets/prototype/index.html`）
2. 展示应用列表（卡片视图 + 列表视图切换）
3. 展示应用搜索（搜索框输入 "Notepad"，展示搜索结果）
4. 展示应用分类（办公、开发、游戏、工具等）

### 场景 2：应用启动（1:00 - 2:00）
1. 点击 Notepad++ 图标
2. 展示 Hook 注入过程（日志输出或进度条）
3. Notepad++ 启动完成，展示应用窗口
4. 打开应用详情页，展示沙箱状态（文件重定向、注册表重定向、子进程继承）

### 场景 3：应用商店（2:00 - 3:30）
1. 打开应用商店
2. 展示浏览/搜索/安装流程
3. 搜索应用（如 "7-Zip"）
4. 点击安装按钮，展示安装进度
5. 安装完成，返回应用列表（展示新安装的应用）

### 场景 4：新手引导（3:30 - 5:00）
1. 打开新手引导
2. 展示 3 步引导流程：
   - 第 1 步：欢迎使用 Launchpad（介绍产品价值）
   - 第 2 步：如何启动应用（演示点击图标启动应用）
   - 第 3 步：如何管理沙箱（演示查看沙箱状态、清理沙箱）
3. 引导完成，进入主界面

---

## 注意事项

1. **音频**：使用外接麦克风，避免内置麦克风杂音
2. **分辨率**：OBS 设置 → 输出 → 录制 → 分辨率：1920x1080
3. **帧率**：OBS 设置 → 输出 → 录制 → 帧率：30 FPS
4. **编码**：OBS 设置 → 输出 → 录制 → 编码：H.264（硬件编码）
5. **文件大小**：5 分钟视频，H.264 编码，约 100-200 MB
6. **备份**：录制完成后，立即备份至项目共享目录 + 本地磁盘

---

**说明结束**

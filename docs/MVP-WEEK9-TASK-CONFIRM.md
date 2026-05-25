# MVP Week 9 任务理解确认

> 本文档确认对 AI ThinApp Portable Launchpad Platform 项目 MVP Week 9 开发任务的理解。
> 日期：2026-05-23 | 角色：Qt/C++ 软件开发工程师

---

## 1. 任务概述

Week 9 的主要开发任务是实现**设置界面**和**任务栏集成**两个核心功能模块。

**时间分配**：
- Day 1-3：设置界面实现（3天）
- Day 4-5：任务栏集成实现（2天）

**关键交付物**：
1. 设置对话框（`SettingsDialog` 类）
2. 任务栏管理器（`TaskbarManager` 类）
3. 托盘菜单修改（添加"设置"动作）
4. 主窗口修改（集成任务栏管理器）
5. 单元测试（设置对话框 + 任务栏管理器）

---

## 2. 设置界面设计理解

### 2.1 界面布局

设置界面采用**标签页**设计，包含三个主要标签页：
1. **全局设置**（对应设计文档 7.2 节）
2. **单应用设置**（对应设计文档 7.4 节启动项管理）
3. **关于**（对应设计文档 7.5 节）

### 2.2 常规设置项（7.2 节）

| 设置项 | 控件类型 | 默认值 | 持久化方式 | 说明 |
|--------|----------|--------|------------|------|
| 开机启动 | `QCheckBox` | 未勾选 | 注册表 `HKCU\Software\Microsoft\Windows\CurrentVersion\Run` | 通过修改注册表实现 |
| 最小化到托盘 | `QCheckBox` | 勾选 | `QSettings` (注册表) | 关闭窗口时最小化到托盘 |
| 退出确认 | `QCheckBox` | 勾选 | `QSettings` (注册表) | 点击退出时显示确认对话框 |
| 默认沙箱路径 | `QLineEdit` + `QPushButton` (浏览) | `D:\ThinAppApps` | `QSettings` (配置文件) | 新应用的沙箱数据保存路径 |
| 清理全局缓存 | `QPushButton` | - | - | 清理 Hook 缓存数据 |

### 2.3 Hook 设置项（7.3 节）

| 设置项 | 控件类型 | 默认值 | 持久化方式 | 说明 |
|--------|----------|--------|------------|------|
| Hook 开关 | `QCheckBox` | 勾选 | `QSettings` (配置文件) | 全局 Hook 开关（文件/注册表拦截） |
| 日志级别 | `QCheckBox` (多选) | 错误 + 警告 | `QSettings` (配置文件) | 高级选项，控制 Hook 日志输出详细程度 |
| 覆盖检测频率 | `QSpinBox` (秒) | 5 秒 | `QSettings` (配置文件) | Hook 覆盖检测频率 |

### 2.4 启动项管理（7.4 节）

**功能说明**：
- 管理 Windows 启动时会自动启动的应用（类似 `msconfig` 的启动项管理）
- 通过 Windows 注册表 `HKCU\Software\Microsoft\Windows\CurrentVersion\Run` 管理

**界面设计**：
```
+---------------------------------------------------------------+
|  启动项管理                                                    |
|  +----------------+  [添加] [删除] [上移] [下移]             |
|  | QQ             |                                            |
|  | 微信           |                                            |
|  | Chrome         |                                            |
|  +----------------+                                            |
+---------------------------------------------------------------+
```

**操作**：
- **添加**：打开文件选择器，选择 exe 文件，添加到启动项列表
- **删除**：从启动项列表中移除（从注册表中删除）
- **上移/下移**：调整启动顺序（写入注册表顺序）

### 2.5 关于界面（7.5 节）

| 信息 | 说明 |
|------|------|
| 版本号 | `v0.1.0 (MVP)` |
| GitHub 链接 | `https://github.com/ai-thinapp/...`（点击用浏览器打开） |
| 许可证 | `GPL v3`（点击显示完整许可证文本） |
| 作者 | `AI ThinApp Team` |
| 感谢 | `MinHook`、`Qt 6`、`zlib` 等开源项目 |

---

## 3. 任务栏集成设计理解

### 3.1 跳转列表设计（8.1 节）

**功能说明**：
- 右键任务栏图标时显示的跳转列表
- 包含：常用应用、最近使用的应用、任务

**跳转列表内容**：
```
+---------------------------------------------------------------+
|  AI ThinApp Launchpad                                          |
|  ------------------------------------------------------------- |
|  常用应用                                                      |
|  - QQ                                                          |
|  - 微信                                                        |
|  - Chrome                                                      |
|  ------------------------------------------------------------- |
|  最近使用的应用                                                |
|  - VSCode (2026-05-23 18:30)                                |
|  - Firefox (2026-05-23 18:25)                               |
|  ------------------------------------------------------------- |
|  任务                                                          |
|  - 添加应用                                                    |
|  - 打开设置                                                    |
+---------------------------------------------------------------+
```

**实现方式**：
- **Windows API**：`ICustomDestinationList` (Windows 7+)
- **Qt 支持**：Qt 6 未直接封装跳转列表 API，需调用 Windows Shell API

### 3.2 缩略图工具栏设计（8.2 节）

**功能说明**：
- 在任务栏缩略图预览中显示工具栏（如播放/暂停、上一曲/下一曲等）
- MVP 阶段：仅实现基础框架，具体应用根据实际需求定制

**实现方式**：
- **Windows API**：`ITaskbarList3::ThumbBarAddButtons()`
- **Qt 支持**：需调用 Windows Shell API

### 3.3 进度条设计（8.3 节）

**功能说明**：
- 在任务栏图标上显示进度条（如文件复制、下载等进度）
- MVP 阶段：实现基础 API，具体应用根据实际需求调用

**实现方式**：
- **Windows API**：`ITaskbarList3::SetProgressValue()`
- **Qt 支持**：需调用 Windows Shell API

**代码实现**：
```cpp
// 设置任务栏进度条（Windows Shell API）
void TaskbarIntegrator::SetProgress(int current, int total) {
    // 获取 ITaskbarList3 接口
    ITaskbarList3* pTaskbarList = nullptr;
    CoCreateInstance(CLSID_TaskbarList, nullptr, CLSCTX_INPROC_SERVER,
                    IID_ITaskbarList3, (void**)&pTaskbarList);
    
    // 设置进度值
    pTaskbarList->SetProgressValue(hwnd_, current, total);
    
    pTaskbarList->Release();
}
```

---

## 4. 技术方案理解

### 4.1 设置持久化方案

**技术方案**：使用 `QSettings` 实现设置持久化

**持久化位置**：
1. **注册表**（Windows 平台）：`HKEY_CURRENT_USER\Software\AI ThinApp\Launchpad`
2. **配置文件**（跨平台兼容）：`settings.json` (位于应用目录)

**QSettings 初始化**：
```cpp
// 在 main.cpp 或应用初始化时设置
QCoreApplication::setOrganizationName("AI ThinApp");
QCoreApplication::setApplicationName("Launchpad");
```

**读写设置示例**：
```cpp
// 读取设置
QSettings settings;
bool autoStart = settings.value("auto_start", false).toBool();
QString sandboxPath = settings.value("sandbox_path", "D:/ThinAppApps").toString();

// 写入设置
settings.setValue("auto_start", true);
settings.setValue("sandbox_path", "D:/NewPath");
```

### 4.2 任务栏集成技术方案

**技术方案**：使用 Windows Shell API (`ITaskbarList3`、`ICustomDestinationList`)

**关键接口**：
1. **`ITaskbarList3`**：
   - `SetProgressValue()` - 设置任务栏进度条
   - `ThumbBarAddButtons()` - 添加缩略图工具栏按钮
   - `SetTabProperties()` - 设置标签页属性

2. **`ICustomDestinationList`**：
   - `SetAppID()` - 设置应用 ID
   - `BeginList()` - 开始构建跳转列表
   - `AppendCategory()` - 添加跳转列表类别
   - `CommitList()` - 提交跳转列表

**实现步骤**：
1. 在 `TaskbarManager` 构造函数中初始化 `ITaskbarList3` 接口
2. 在应用启动/停止时调用 `SetProgressValue()` 更新进度条
3. 在应用添加/删除时调用 `ICustomDestinationList` 更新跳转列表
4. 在 `ThumbnailToolbar` 按钮点击时处理事件

**注意事项**：
- Windows Shell API 需要链接 `shell32.lib` 和 `ole32.lib`
- 需要在项目文件 (`CMakeLists.txt`) 中添加库依赖
- 需要使用 `CoInitialize()` 初始化 COM 库

---

## 5. 开发任务拆解

### 5.1 设置界面实现（Day 1-3）

#### 任务 1：创建 `SettingsDialog` 类（Day 1）

**文件**：`src/launchpad/ui/settings_dialog.h`

**类定义**：
```cpp
class SettingsDialog : public QDialog {
    Q_OBJECT

public:
    explicit SettingsDialog(QWidget* parent = nullptr);
    ~SettingsDialog();

private slots:
    void onAutoStartChanged(bool checked);      // 开机自启动复选框状态变化
    void onSandboxBorderChanged(bool checked);  // 沙箱边框颜色复选框状态变化
    void onCacheSizeChanged(int value);         // 缓存大小滑块值变化
    void onOkClicked();                         // 确定按钮点击
    void onCancelClicked();                     // 取消按钮点击

private:
    void initUI();       // 初始化 UI
    void loadSettings(); // 加载设置
    void saveSettings(); // 保存设置

    // UI 控件
    QTabWidget* tabWidget_;
    QCheckBox* autoStartCheckBox_;
    QCheckBox* sandboxBorderCheckBox_;
    QSpinBox* cacheSizeSpinBox_;
    QPushButton* okButton_;
    QPushButton* cancelButton_;
};
```

#### 任务 2：实现 `SettingsDialog` 功能（Day 2）

**文件**：`src/launchpad/ui/settings_dialog.cpp`

**关键功能实现**：
1. **`initUI()`**：创建三个标签页（全局设置、单应用设置、关于），添加控件
2. **`loadSettings()`**：从 `QSettings` 加载设置，更新 UI 控件状态
3. **`saveSettings()`**：保存 UI 控件状态到 `QSettings`，修改注册表（开机自启动）
4. **`onAutoStartChanged()`**：修改注册表 `HKCU\Software\Microsoft\Windows\CurrentVersion\Run`
5. **`contextMenuEvent()`**：右键菜单（导入/导出设置）

#### 任务 3：修改托盘菜单（Day 3）

**文件**：`src/launchpad/ui/tray_menu.cpp`

**修改内容**：
- 在右键菜单中添加"设置"动作
- 连接 `SettingsDialog::exec()` 槽

### 5.2 任务栏集成实现（Day 4-5）

#### 任务 4：创建 `TaskbarManager` 类（Day 4）

**文件**：`src/launchpad/ui/taskbar_manager.h`

**类定义**：
```cpp
class TaskbarManager : public QObject {
    Q_OBJECT

public:
    static TaskbarManager& instance();  // 单例模式
    ~TaskbarManager();

    void setTaskbarProgress(int progress);  // 设置任务栏进度条
    void setThumbnailToolbar();              // 设置缩略图工具栏
    void updateJumpList();                  // 更新跳转列表

signals:
    void thumbnailButtonClicked(int buttonId);  // 缩略图工具栏按钮点击信号

private:
    explicit TaskbarManager(QObject* parent = nullptr);
    bool initTaskbarList();  // 初始化 ITaskbarList3 接口

    ITaskbarList3* taskbarList_;
    HWND hwnd_;  // 主窗口句柄
};
```

#### 任务 5：实现 `TaskbarManager` 功能（Day 5）

**文件**：`src/launchpad/ui/taskbar_manager.cpp`

**关键功能实现**：
1. **`initTaskbarList()`**：调用 `CoCreateInstance()` 初始化 `ITaskbarList3` 接口
2. **`setTaskbarProgress()`**：调用 `ITaskbarList3::SetProgressValue()` 设置进度条
3. **`setThumbnailToolbar()`**：调用 `ITaskbarList3::ThumbBarAddButtons()` 添加按钮
4. **`updateJumpList()`**：调用 `ICustomDestinationList` API 更新跳转列表
5. **`OnThumbnailButtonClicked()`**：处理缩略图工具栏按钮点击事件

#### 任务 6：修改主窗口（Day 5）

**文件**：`src/launchpad/ui/main_window.cpp`

**修改内容**：
- 在构造函数中创建 `TaskbarManager` 实例
- 在 `onAppLaunched()` 中调用 `TaskbarManager::setTaskbarProgress()`
- 在 `onAppStopped()` 中调用 `TaskbarManager::setTaskbarProgress()`
- 在 `onAppSelected()` 中调用 `TaskbarManager::updateJumpList()`

---

## 6. 验收标准理解

### 6.1 设置界面验收标准

| ID | 验收项 | 成功标准 |
|----|--------|----------|
| S1 | 设置对话框正常显示 | 三个标签页（全局设置、单应用设置、关于）正常显示，布局美观 |
| S2 | 常规设置功能正常 | 开机自启动、最小化到托盘、退出确认功能正常 |
| S3 | Hook 设置功能正常 | Hook 开关、日志级别、覆盖检测频率功能正常 |
| S4 | 启动项管理功能正常 | 添加/删除启动项功能正常，注册表已更新 |
| S5 | 关于页面功能正常 | 版本号、GitHub 链接、许可证显示正确 |
| S6 | 设置持久化正常 | 关闭对话框后重新打开，设置已保存 |

### 6.2 任务栏集成验收标准

| ID | 验收项 | 成功标准 |
|----|--------|----------|
| T1 | 任务栏进度条正常显示 | 应用启动时显示 100% 进度，停止后清除进度条 |
| T2 | 缩略图工具栏正常显示 | 右键任务栏图标，显示 3 个按钮（启动、停止、设置） |
| T3 | 跳转列表正常显示 | 右键任务栏图标，显示"最近使用的应用"和"固定应用" |
| T4 | 按钮点击事件正常处理 | 点击缩略图工具栏按钮，执行对应操作 |
| T5 | 任务栏集成开关正常 | 设置中可关闭任务栏集成，关闭后任务栏无变化 |

### 6.3 单元测试验收标准

| ID | 验收项 | 成功标准 |
|----|--------|----------|
| U1 | 设置对话框测试用例通过 | 5 个测试用例全部通过 |
| U2 | 任务栏管理器测试用例通过 | 5 个测试用例全部通过 |
| U3 | 单元测试覆盖率 ≥ 80% | 使用 `gcov` 或 `OpenCppCoverage` 测量覆盖率 |

---

## 7. 风险与依赖理解

### 7.1 风险

| 风险 ID | 风险描述 | 影响 | 缓释措施 |
|---------|----------|------|----------|
| R1 | `ITaskbarList3` 接口调用失败 | 高 | 提前研究 Windows Shell API；若实现困难，MVP 阶段仅实现基础功能 |
| R2 | 注册表访问权限不足 | 中 | 以管理员权限运行；若失败，提示用户手动添加注册表项 |
| R3 | 设置持久化失败 | 中 | 同时保存到注册表和配置文件，双备份机制 |

### 7.2 依赖

| 依赖项 | 来源 | 说明 |
|--------|------|------|
| `QSettings` | Qt 6 框架 | 设置持久化 |
| `ITaskbarList3` | Windows Shell API | 任务栏集成 |
| `ICustomDestinationList` | Windows Shell API | 跳转列表 |
| `QSystemTrayIcon` | Qt 6 框架 | 托盘菜单修改 |

---

## 8. 总结

我已经完整理解了 MVP Week 9 的开发任务，包括：

1. **设置界面设计**：三个标签页（全局设置、单应用设置、关于），使用 `QSettings` 持久化
2. **任务栏集成设计**：跳转列表、缩略图工具栏、进度条，使用 Windows Shell API
3. **技术方案**：`QSettings` (设置持久化)、`ITaskbarList3` (任务栏集成)
4. **开发任务**：6 个主要任务（创建 SettingsDialog、实现功能、修改托盘菜单、创建 TaskbarManager、实现功能、修改主窗口）
5. **验收标准**：设置界面 6 项、任务栏集成 5 项、单元测试覆盖率 ≥ 80%
6. **风险与依赖**：`ITaskbarList3` 接口调用失败、注册表访问权限不足、设置持久化失败

**下一步行动**：
1. 开始实现设置界面（Day 1-3）
2. 实现任务栏集成（Day 4-5）
3. 编写单元测试
4. 输出 Week 9 完成报告

---

## 9. 修订历史

| 版本 | 日期 | 作者 | 变更说明 |
|------|------|------|----------|
| 1.0 | 2026-05-23 | Qt/C++ 软件开发工程师 | 初版，确认对 Week 9 任务的理解 |

---

## 附录 A：参考资料

1. **`docs/MVP-LAUNCHPAD-DESIGN.md`**：MVP Launchpad UI 设计文档（第 7、8 节）
2. **Qt 6 官方文档**：https://doc.qt.io/qt-6/qsettings.html
3. **Windows Shell API 文档**：https://docs.microsoft.com/en-us/windows/win32/shell/
4. **`docs/ARCHITECTURE.md`**：技术架构文档
5. **`docs/MVP-PLAN.md`**：MVP 开发计划

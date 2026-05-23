# MVP Launchpad UI 设计文档

> 本文档定义 AI ThinApp Portable Launchpad Platform 项目 MVP 阶段 Launchpad UI 的详细设计。
> 版本：0.1 | 日期：2026-05-23 | 作者：Qt/C++ 软件架构师

---

## 1. 现有技术栈分析

### 1.1 技术选型

| 技术 | 选型 | 理由 | 来源 |
|------|------|------|------|
| GUI 框架 | **Qt 6** | POC 阶段验证通过，原生性能，LGPL 协议兼容 | `src/launchpad/ui/main_window.h/cpp` |
| Hook 框架 | MinHook | 轻量级，支持 x86/x64，MIT 协议 | `docs/ARCHITECTURE.md` |
| 构建系统 | CMake | 跨平台，与 Qt 6 集成良好 | `src/launchpad/CMakeLists.txt` |
| 编译器 | Visual Studio 2022 (MSVC 17.0+) | Windows 原生开发工具链 | `docs/ARCHITECTURE.md` |

**重要变更说明**：
- 原 `docs/ARCHITECTURE.md` 中 Launchpad 技术选型为 Electron（MVP 用 Electron，V2 迁移到 Win32/Qt）
- POC 阶段实际采用 **Qt 6** 实现原型（`src/launchpad/`），验证通过
- **MVP 阶段继续使用 Qt 6**，放弃 Electron 方案（性能更优，部署更简单）

### 1.2 POC 阶段原型代码分析

#### 1.2.1 `src/launchpad/app_manager.h/cpp`

**功能**：
- 应用管理器，负责启动/停止应用，管理沙箱进程
- 使用 `QProcess` 启动外部进程
- 提供信号/槽机制通知应用状态变化

**限制与不足**：
1. **Hook DLL 未实际注入**：`InjectHookDll()` 函数仅设置环境变量（`AI_THINAPP_APP_DIR`），未实际注入 Hook DLL
2. **应用列表为模拟数据**：`LoadApps()` 函数使用硬编码的模拟数据，未从配置文件或注册表加载
3. **进程管理简化**：仅使用 `QProcess` 基本功能，未实现进程树遍历（停止应用时无法终止子进程）
4. **缺少应用元数据**：`AppInfo` 结构体仅包含名称、路径、图标，缺少版本、描述、沙箱状态等字段

#### 1.2.2 `src/launchpad/ui/main_window.h/cpp`

**功能**：
- Launchpad 主窗口，采用 `QListWidget` 图标模式显示应用列表
- 系统托盘图标支持（显示/隐藏窗口）
- 应用列表右键菜单（启动、打开目录、删除）

**限制与不足**：
1. **UI 布局简化**：仅实现基础应用列表，缺少搜索栏、分类过滤、排序等功能
2. **应用卡片设计简单**：仅显示图标和名称，缺少版本、状态标识等信息
3. **缺少应用详情页**：双击应用直接启动，无详情页展示沙箱状态、配置选项等
4. **托盘功能不完整**：仅支持显示/隐藏窗口，缺少 balloon 通知、快捷操作等
5. **未集成沙箱文件浏览器**：无法查看应用沙箱内的文件结构

### 1.3 技术栈总结

| 组件 | POC 状态 | MVP 目标 | 风险 |
|------|-----------|----------|------|
| Qt 6 框架 | ✅ 验证通过 | 继续使用 | 低 |
| Hook DLL 注入 | ❌ 未实现 | 必须实现 | 高（需 Windows API Hook 知识） |
| 应用列表持久化 | ❌ 模拟数据 | 从配置文件加载 | 低 |
| 进程树管理 | ❌ 简化实现 | 完整实现（遍历子进程） | 中 |
| 沙箱文件浏览器 | ❌ 未实现 | 必须实现 | 中（需集成 `docs/SANDBOX-DIR-VISUALIZATION.md`） |
| 任务栏集成 | ❌ 未实现 | MVP 实现跳转列表 | 高（需 Windows Shell API） |

---

## 2. UI 架构设计

### 2.1 模块划分

```
Launchpad UI 架构
│
├── 主窗口模块 (MainWindow)
│   ├── 应用列表视图 (AppListWidget)
│   ├── 应用详情视图 (AppDetailWidget)
│   ├── 搜索栏 (SearchBar)
│   └── 分类过滤器 (CategoryFilter)
│
├── 托盘模块 (TrayManager)
│   ├── 托盘图标 (QSystemTrayIcon)
│   ├── 右键菜单 (QMenu)
│   └── Balloon 通知 (showMessage)
│
├── 应用管理模块 (AppManager)
│   ├── 应用列表模型 (AppListModel)
│   ├── 应用启动器 (AppLauncher)
│   └── 应用数据持久化 (AppConfigStore)
│
├── 沙箱浏览器模块 (SandboxBrowser)
│   ├── 目录树视图 (DirectoryTreeView)
│   ├── 文件列表视图 (FileListView)
│   └── 存储空间可视化 (StorageVisualization)
│
├── 设置模块 (SettingsManager)
│   ├── 常规设置 (GeneralSettings)
│   ├── Hook 设置 (HookSettings)
│   └── 启动项管理 (StartupManager)
│
└── 任务栏集成模块 (TaskbarIntegrator)
    ├── 跳转列表 (JumpList)
    ├── 缩略图工具栏 (ThumbnailToolbar)
    └── 进度条 (ProgressIndicator)
```

### 2.2 信号/槽设计（跨模块通信）

#### 2.2.1 核心信号/槽

| 信号发送者 | 信号 | 接收者 | 槽 | 用途 |
|------------|------|--------|-----|------|
| `AppManager` | `AppLaunched(appDir)` | `MainWindow` | `OnAppLaunched(appDir)` | 更新应用状态（运行中） |
| `AppManager` | `AppStopped(appDir)` | `MainWindow` | `OnAppStopped(appDir)` | 更新应用状态（已停止） |
| `AppManager` | `AppCrashed(appDir, exitCode)` | `TrayManager` | `ShowCrashNotification(appDir, exitCode)` | 显示崩溃通知 |
| `AppLauncher` | `HookInjectionFailed(appDir, error)` | `TrayManager` | `ShowErrorNotification(error)` | 显示 Hook 注入失败通知 |
| `MainWindow` | `AppSelected(appDir)` | `AppDetailWidget` | `ShowAppDetail(appDir)` | 显示应用详情 |
| `MainWindow` | `SearchQueryChanged(query)` | `AppListModel` | `FilterApps(query)` | 过滤应用列表 |
| `SandboxBrowser` | `FileExportRequested(filePath, destPath)` | `AppManager` | `ExportFile(filePath, destPath)` | 导出沙箱文件 |
| `TaskbarIntegrator` | `JumpListActionTriggered(actionId)` | `AppManager` | `HandleJumpListAction(actionId)` | 处理跳转列表操作 |

#### 2.2.2 信号/槽连接示例

```cpp
// AppManager 信号连接到 MainWindow 槽
connect(appManager_, &AppManager::AppLaunched,
        mainWindow_, &MainWindow::OnAppLaunched);

// AppManager 信号连接到 TrayManager 槽
connect(appManager_, &AppManager::AppCrashed,
        trayManager_, &TrayManager::ShowCrashNotification);

// MainWindow 搜索信号连接到 AppListModel 槽
connect(mainWindow_, &MainWindow::SearchQueryChanged,
        appListModel_, &AppListModel::FilterApps);
```

### 2.3 线程模型

#### 2.3.1 线程划分

| 线程 | 职责 | 说明 |
|------|------|------|
| **UI 线程** (主线程) | 界面渲染、用户交互 | Qt 主事件循环 (`QApplication::exec()`) |
| **Hook 引擎线程** | Hook DLL 注入、API Hook 安装 | 使用 `QtConcurrent` 或 `QThread` 异步执行，避免阻塞 UI |
| **应用启动线程** | 创建进程、等待启动完成 | 使用 `QProcess::start()` 异步启动，通过 `QProcess::finished` 信号通知完成 |
| **文件浏览器后台线程** | 遍历沙箱目录、计算文件大小 | 使用 `QtConcurrent::run()` 后台遍历，通过信号通知 UI 更新 |
| **应用数据加载线程** | 从配置文件加载应用列表 | 启动时后台加载，避免界面卡顿 |

#### 2.3.2 线程安全策略

- **UI 线程与其他线程通信**：使用 Qt 信号/槽（自动队列化，跨线程安全）
- **共享数据保护**：使用 `QMutex` 或 `QReadWriteLock` 保护共享数据（如应用列表）
- **后台任务取消**：使用 `QFutureWatcher` + `QFuture::cancel()` 支持任务取消

#### 2.3.3 线程模型示例代码

```cpp
// Hook 引擎线程（异步注入 Hook DLL）
void AppLauncher::LaunchAppAsync(const QString& appDir) {
    // 使用 QtConcurrent 异步执行
    auto future = QtConcurrent::run([this, appDir]() {
        return LaunchApp(appDir);  // 同步启动应用（注入 Hook DLL）
    });
    
    // 使用 QFutureWatcher 监听完成
    auto watcher = new QFutureWatcher<bool>(this);
    connect(watcher, &QFutureWatcher<bool>::finished,
            this, [this, watcher, appDir](bool success) {
                if (success) {
                    emit AppLaunched(appDir);
                } else {
                    emit AppLaunchFailed(appDir, "Hook 注入失败");
                }
                watcher->deleteLater();
            });
    watcher->setFuture(future);
}
```

### 2.4 内存管理

#### 2.4.1 智能指针使用策略

| 场景 | 智能指针类型 | 说明 |
|------|--------------|------|
| **QObject 父子关系** | 不使用智能指针 | Qt 对象树自动管理（父对象销毁时自动销毁子对象） |
| **非 QObject 对象** | `std::unique_ptr` | 独占所有权，自动释放 |
| **共享数据** | `std::shared_ptr` | 共享所有权，引用计数为 0 时释放 |
| **交叉引用** | `std::weak_ptr` | 避免循环引用（如 `AppInfo` 引用 `AppManager`） |

#### 2.4.2 对象树管理

- **原则**：所有 `QObject` 派生类对象必须设置 parent（父对象）
- **示例**：
  ```cpp
  // 正确：设置 parent，由 Qt 对象树管理生命周期
  QPushButton* button = new QPushButton("启动", this);
  
  // 错误：未设置 parent，可能导致内存泄漏
  QPushButton* button = new QPushButton("启动");
  ```

#### 2.4.3 缓存管理

- **应用图标缓存**：使用 `QCache<QString, QIcon>` 缓存应用图标（key 为应用路径）
- **文件信息缓存**：使用 `QCache<QString, QFileInfo>` 缓存文件信息（避免重复读取磁盘）

---

## 3. 主窗口设计

### 3.1 布局设计

```
+---------------------------------------------------------------+
|  AI ThinApp Launchpad                [搜索框] [商店] [设置]   |
+---------------------------------------------------------------+
|  [添加应用]  [全部 ▼] [运行中] [已安装] [未安装] [有更新]  |
+---------------------------------------------------------------+
|  +----------------+  +----------------+  +----------------+  |
|  | [图标]         |  | [图标]         |  | [图标]         |  |
|  | 应用名称1      |  | 应用名称2      |  | 应用名称3      |  |
|  | 版本: 1.0.0   |  | 版本: 2.3.1   |  | 版本: 3.2.0   |  |
|  | 状态: 已便携化 |  | 状态: 运行中   |  | 状态: 未安装   |  |
|  | [启动] [设置]  |  | [启动] [设置]  |  | [安装] [详情]  |  |
|  +----------------+  +----------------+  +----------------+  |
|  +----------------+  +----------------+  +----------------+  |
|  | [图标]         |  | [图标]         |  | [图标]         |  |
|  | 应用名称4      |  | 应用名称5      |  | 应用名称6      |  |
|  | 版本: 4.1.2   |  | 版本: 5.0.0   |  | 版本: 6.2.1   |  |
|  | 状态: 有更新   |  | 状态: 已便携化 |  | 状态: 运行中   |  |
|  | [启动] [设置]  |  | [启动] [设置]  |  | [启动] [设置]  |  |
|  +----------------+  +----------------+  +----------------+  |
+---------------------------------------------------------------+
|  状态栏：3个应用运行中 | 沙箱模式已启用                       |
+---------------------------------------------------------------+
```

#### 3.1.1 布局组件

| 组件 | Qt 控件 | 说明 |
|------|---------|------|
| 顶部工具栏 | `QHBoxLayout` | 包含搜索框、商店按钮、设置按钮 |
| 分类过滤器 | `QHBoxLayout` + `QPushButton` | 全部、运行中、已安装、未安装、有更新 |
| 应用列表区域 | `QListWidget` (图标模式) 或 `QListView` + 自定义委托 | 显示应用卡片 |
| 状态栏 | `QStatusBar` | 显示运行中的应用数量、沙箱模式状态 |

### 3.2 应用卡片设计

#### 3.2.1 卡片布局

```
+----------------------+
|  [应用图标]           |
|  应用名称             |
|  版本: 1.0.0         |
|  状态: [图标] 运行中  |
|  [启动] [设置] [删除] |
+----------------------+
```

#### 3.2.2 卡片组件

| 组件 | Qt 控件 | 说明 |
|------|---------|------|
| 应用图标 | `QIcon` (64x64) | 从应用 exe 提取图标（`QFileIconProvider`） |
| 应用名称 | `QLabel` | 主标题，显示应用名称 |
| 版本 | `QLabel` | 副标题，显示当前版本 |
| 状态标识 | `QLabel` + `QIcon` | 标签："已便携化"、"运行中"、"未安装"、"有更新" |
| 操作按钮 | `QPushButton` | [启动]、[设置]、[删除]（更多操作通过右键菜单） |

#### 3.2.3 沙箱状态标识

- **沙箱模式**：应用图标右上角显示绿色角标（✅）
- **真实模式**：应用图标右上角显示红色角标（⚠️）
- **运行中**：应用卡片显示"运行中"标签，按钮变为[停止]

### 3.3 交互设计

#### 3.3.1 单击/双击行为

| 操作 | 行为 | 说明 |
|------|------|------|
| **单击应用卡片** | 选中应用 | 高亮显示选中状态，右侧显示应用详情（若空间允许） |
| **双击应用卡片** | 启动应用 | 调用 `AppManager::LaunchApp()` 启动应用 |
| **右键点击** | 显示上下文菜单 | 包含：启动、停止、打开目录、删除、属性 |

#### 3.3.2 拖拽排序

- **功能**：支持拖拽应用卡片调整顺序
- **实现**：使用 `QListWidget::setDragEnabled(true)` + 自定义拖拽逻辑
- **持久化**：排序结果保存到配置文件（`QSettings`）

#### 3.3.3 键盘导航

- **方向键**：上下左右导航应用卡片
- **Enter 键**：启动选中的应用
- **Delete 键**：删除选中的应用（需确认）
- **ContextMenu 键**：打开右键菜单

### 3.4 视觉设计

#### 3.4.1 Fluent UI 风格

- **设计语言**：参考 Microsoft Fluent UI（Windows 11 风格）
- **圆角卡片**：`QListWidgetItem` 使用圆角矩形（通过自定义委托绘制）
- **阴影效果**：使用 `QGraphicsDropShadowEffect` 添加卡片阴影
- **悬停效果**：鼠标悬停时卡片背景色变化（浅蓝色高亮）

#### 3.4.2 沙箱标识集成

- **窗口边框着色**：沙箱内运行的应用窗口边框显示为绿色（需 Hook Windows API）
- **任务栏角标**：沙箱内应用任务栏图标添加绿色角标（✅）（需 Windows Shell API）
- **应用卡片角标**：沙箱模式应用卡片右上角显示绿色角标（通过自定义委托绘制）

---

## 4. 托盘模块设计

### 4.1 托盘图标设计

#### 4.1.1 图标状态

| 状态 | 图标 | 说明 |
|------|------|------|
| **默认状态** | 应用图标（灰色） | Launchpad 未启动任何应用 |
| **有应用运行中** | 应用图标（绿色） | 至少一个应用在沙箱中运行 |
| **Hook 注入失败** | 应用图标（红色） | Hook DLL 注入失败，显示警告 |
| **应用崩溃** | 应用图标（橙色） | 应用异常退出 |

#### 4.1.2 动画效果

- **启动中动画**：托盘图标旋转进度条（表示应用正在启动）
- **Hook 注入中动画**：托盘图标脉动效果（表示 Hook 正在注入）

### 4.2 右键菜单设计

```
+-------------------------------------+
| 显示/隐藏 Launchpad                 |  → 显示或隐藏主窗口
| ----------------------------------- |
| 启动应用 >                         |  → 子菜单：显示最近使用的应用
|   - QQ                             |
|   - 微信                           |
|   - Chrome                         |
| ----------------------------------- |
| 打开应用目录                       |  → 在文件资源管理器中显示
| ----------------------------------- |
| 设置                               |  → 打开设置对话框
| ----------------------------------- |
| 关于                               |  → 显示关于对话框
| 退出                               |  → 完全退出 Launchpad
+-------------------------------------+
```

#### 4.2.1 菜单项说明

| 菜单项 | 功能 | 快捷键 |
|--------|------|--------|
| 显示/隐藏 Launchpad | 切换主窗口显示/隐藏 | - |
| 启动应用 > | 子菜单：显示最近使用的应用，点击启动 | - |
| 打开应用目录 | 在文件资源管理器中打开应用目录 | - |
| 设置 | 打开设置对话框 | - |
| 关于 | 显示关于对话框（版本号、GitHub 链接、许可证） | - |
| 退出 | 完全退出 Launchpad（隐藏托盘图标） | - |

### 4.3 Balloon 通知设计

#### 4.3.1 通知场景

| 场景 | 标题 | 内容 | 图标 |
|------|------|------|------|
| 应用启动成功 | "应用启动成功" | "{应用名} 已在沙箱模式中启动" | Information |
| 应用停止 | "应用已停止" | "{应用名} 已正常退出" | Information |
| 应用崩溃 | "应用崩溃" | "{应用名} 异常退出（退出码：{code}）" | Warning |
| Hook 注入失败 | "Hook 注入失败" | "无法注入 Hook DLL，应用将在真实环境中运行" | Critical |
| 杀软拦截 | "杀软拦截警告" | "杀软可能阻止 Hook DLL，请添加白名单" | Warning |

#### 4.3.2 通知实现

```cpp
// 显示 balloon 通知
void TrayManager::ShowNotification(const QString& title,
                                  const QString& message,
                                  QSystemTrayIcon::MessageIcon icon) {
    trayIcon_->showMessage(title, message, icon, 5000);  // 显示 5 秒
}
```

---

## 5. 应用管理模块设计

### 5.1 应用列表模型

#### 5.1.1 数据模型设计

```cpp
// 应用信息结构体
struct AppInfo {
    QString appDir;          // 应用目录路径（沙箱根目录）
    QString name;            // 应用名称
    QString exePath;         // exe 文件路径
    QString iconPath;        // 图标文件路径
    QString version;         // 版本号
    QString description;     // 应用描述
    bool isPortable;         // 是否已便携化（沙箱化）
    bool isRunning;          // 是否正在运行
    QProcess* process;       // 进程指针（运行中非空）
    QDateTime lastLaunchTime; // 上次启动时间
};
```

#### 5.1.2 模型类设计

```cpp
// 应用列表模型（继承 QAbstractListModel）
class AppListModel : public QAbstractListModel {
    Q_OBJECT

public:
    // 角色定义（用于 QML 或自定义委托）
    enum AppRole {
        NameRole = Qt::UserRole + 1,
        VersionRole,
        IsRunningRole,
        IsPortableRole,
        IconRole,
        LastLaunchTimeRole
    };

    // 构造函数
    explicit AppListModel(QObject* parent = nullptr);

    // QAbstractListModel 接口
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

    // 应用管理接口
    void AddApp(const AppInfo& appInfo);
    void RemoveApp(const QString& appDir);
    void UpdateApp(const QString& appDir, const AppInfo& appInfo);
    AppInfo GetAppInfo(const QString& appDir) const;
    QVector<AppInfo> GetAllApps() const;

    // 过滤/排序接口
    void FilterApps(const QString& query);
    void SortApps(Qt::SortOrder order);

signals:
    void AppAdded(const QString& appDir);
    void AppRemoved(const QString& appDir);
    void AppUpdated(const QString& appDir);
    void ModelChanged();  // 模型数据变化信号

private:
    QVector<AppInfo> apps_;           // 所有应用列表
    QVector<AppInfo> filteredApps_;   // 过滤后的应用列表
};
```

### 5.2 应用启动流程

#### 5.2.1 启动流程详细步骤

```
用户点击"启动"按钮
        │
        ▼
检查应用状态（是否已运行？）
        │
        ├── 已运行 → 提示用户"应用正在运行中"
        │
        └── 未运行 → 准备启动环境
                │
                ▼
        检查应用 exe 是否存在
                │
                ├── 不存在 → 提示用户"应用文件缺失"
                │
                └── 存在 → 设置环境变量
                        │
                        ▼
                设置 AI_THINAPP_APP_DIR = appDir
                        │
                        ▼
                注入 Hook DLL
                        │
                        ├── 注入失败 → 提示用户"Hook 注入失败，是否继续？"
                        │
                        └── 注入成功 → 创建 QProcess 启动应用
                                │
                                ▼
                        监听进程状态（finished 信号）
                                │
                                ├── 正常退出 → 发射 AppStopped 信号
                                │
                                └── 异常退出 → 发射 AppCrashed 信号
```

#### 5.2.2 Hook DLL 注入实现

```cpp
// 注入 Hook DLL（设置环境变量 + 远程线程注入）
bool AppLauncher::InjectHookDll(const QString& appDir, QProcess* process) {
    if (!process) return false;
    
    // 1. 设置环境变量
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    env.insert("AI_THINAPP_APP_DIR", appDir);
    process->setProcessEnvironment(env);
    
    // 2. 远程线程注入 Hook DLL（Windows API）
    HANDLE hProcess = process->processHandle();  // 获取进程句柄
    if (!hProcess) {
        qWarning() << "无法获取进程句柄:" << process->errorString();
        return false;
    }
    
    // 3. 在目标进程中分配内存（写入 DLL 路径）
    QString dllPath = QCoreApplication::applicationDirPath() + "/hook.dll";
    LPVOID pRemoteMem = VirtualAllocEx(hProcess, nullptr, dllPath.length() * 2,
                                       MEM_COMMIT, PAGE_READWRITE);
    if (!pRemoteMem) {
        qWarning() << "VirtualAllocEx 失败:" << GetLastError();
        return false;
    }
    
    // 4. 写入 DLL 路径到目标进程内存
    WCHAR widePath[MAX_PATH];
    dllPath.toWCharArray(widePath);
    SIZE_T bytesWritten;
    WriteProcessMemory(hProcess, pRemoteMem, widePath,
                       dllPath.length() * 2, &bytesWritten);
    
    // 5. 创建远程线程（调用 LoadLibraryW 加载 DLL）
    HANDLE hRemoteThread = CreateRemoteThread(hProcess, nullptr, 0,
                                             (LPTHREAD_START_ROUTINE)LoadLibraryW,
                                             pRemoteMem, 0, nullptr);
    if (!hRemoteThread) {
        qWarning() << "CreateRemoteThread 失败:" << GetLastError();
        VirtualFreeEx(hProcess, pRemoteMem, 0, MEM_RELEASE);
        return false;
    }
    
    // 6. 等待远程线程结束
    WaitForSingleObject(hRemoteThread, INFINITE);
    
    // 7. 清理
    VirtualFreeEx(hProcess, pRemoteMem, 0, MEM_RELEASE);
    CloseHandle(hRemoteThread);
    
    qInfo() << "Hook DLL 注入成功:" << appDir;
    return true;
}
```

### 5.3 应用停止流程

#### 5.3.1 停止流程详细步骤

```
用户点击"停止"按钮
        │
        ▼
检查应用状态（是否正在运行？）
        │
        ├── 未运行 → 提示用户"应用未运行"
        │
        └── 正在运行 → 尝试优雅终止
                │
                ▼
        发送 WM_CLOSE 消息到应用主窗口
                │
                ├── 成功关闭 → 等待进程退出（3 秒）
                │
                └── 超时未关闭 → 强制终止进程树
                        │
                        ▼
                遍历子进程（使用 Process Explorer API）
                        │
                        ▼
                强制终止所有子进程（TerminateProcess）
                        │
                        ▼
                发射 AppStopped 信号
```

#### 5.3.2 进程树遍历实现

```cpp
// 终止进程树（父进程 + 所有子进程）
void AppManager::TerminateProcessTree(DWORD parentPid) {
    // 1. 遍历所有进程，查找父进程 ID = parentPid 的进程
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnapshot == INVALID_HANDLE_VALUE) {
        qWarning() << "CreateToolhelp32Snapshot 失败:" << GetLastError();
        return;
    }
    
    PROCESSENTRY32 pe32;
    pe32.dwSize = sizeof(PROCESSENTRY32);
    
    // 2. 递归终止子进程
    if (Process32First(hSnapshot, &pe32)) {
        do {
            if (pe32.th32ParentProcessID == parentPid) {
                // 递归终止子进程
                TerminateProcessTree(pe32.th32ProcessID);
            }
        } while (Process32Next(hSnapshot, &pe32));
    }
    
    CloseHandle(hSnapshot);
    
    // 3. 终止父进程
    HANDLE hProcess = OpenProcess(PROCESS_TERMINATE, FALSE, parentPid);
    if (hProcess) {
        TerminateProcess(hProcess, 0);
        CloseHandle(hProcess);
    }
}
```

### 5.4 应用添加流程

#### 5.4.1 添加流程详细步骤

```
用户点击"添加应用"按钮
        │
        ▼
打开文件选择器（过滤 *.exe）
        │
        ▼
用户选择 exe 文件
        │
        ▼
检查应用是否已被添加
        │
        ├── 已添加 → 提示用户"应用已存在"
        │
        └── 未添加 → 创建应用目录结构
                │
                ▼
                创建 {AppDir}/VFS/ 目录
                        │
                        ▼
                复制 exe 文件到 {AppDir}/（可选）
                        │
                        ▼
                提取应用图标（使用 ExtractIcon）
                        │
                        ▼
                保存到应用列表配置文件（JSON 格式）
                        │
                        ▼
                更新 UI（添加应用卡片）
                        │
                        ▼
                发射 AppAdded 信号
```

#### 5.4.2 应用配置文件格式（JSON）

```json
{
  "apps": [
    {
      "appDir": "D:/ThinAppApps/QQ",
      "name": "QQ",
      "exePath": "D:/ThinAppApps/QQ/QQ.exe",
      "iconPath": "D:/ThinAppApps/QQ/icon.ico",
      "version": "9.9.9",
      "description": "腾讯 QQ",
      "isPortable": true,
      "lastLaunchTime": "2026-05-23T18:30:00"
    }
  ]
}
```

---

## 6. 沙箱文件浏览器设计

### 6.1 集成方案选择

根据 `docs/SANDBOX-DIR-VISUALIZATION.md` 的推荐，**采用方案 A（树状文件浏览器）+ 方案 B（存储空间可视化）组合方案**。

### 6.2 目录树组件设计

#### 6.2.1 组件选型

| 技术方案 | Qt 控件 | 理由 |
|----------|---------|------|
| **目录树** | `QTreeView` + `QFileSystemModel` | `QFileSystemModel` 原生支持文件系统，但需适配虚拟路径 |
| **自定义模型** | `QAbstractItemModel` 子类 | 完全控制数据，适配 `{AppDir}\VFS\` 虚拟路径 |

**决策**：使用 **自定义模型**（`QAbstractItemModel` 子类），因为 `QFileSystemModel` 无法直接映射虚拟路径（`{AppDir}\VFS\C\Windows\...` → 显示为 `C:\Windows\...`）。

#### 6.2.2 自定义模型设计

```cpp
// 沙箱目录树模型（继承 QAbstractItemModel）
class SandboxDirModel : public QAbstractItemModel {
    Q_OBJECT

public:
    // 构造函数
    explicit SandboxDirModel(const QString& appDir, QObject* parent = nullptr);

    // QAbstractItemModel 接口
    QModelIndex index(int row, int column, const QModelIndex& parent) const override;
    QModelIndex parent(const QModelIndex& index) const override;
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;

    // 自定义接口
    QString filePath(const QModelIndex& index) const;  // 获取真实文件路径
    bool isDir(const QModelIndex& index) const;         // 判断是否为目录

private:
    QString appDir_;  // 应用目录（沙箱根目录）
    QString vfsDir_; // VFS 目录（{AppDir}\VFS\）
};
```

### 6.3 文件列表组件设计

#### 6.3.1 组件选型

| 技术方案 | Qt 控件 | 理由 |
|----------|---------|------|
| **文件列表** | `QTableView` + 自定义模型 | 支持排序、列宽调整、多选 |
| **简化方案** | `QListWidget` | 快速实现，但功能有限 |

**决策**：使用 **`QTableView` + 自定义模型**（`QAbstractTableModel` 子类），支持文件属性展示（名称、大小、类型、修改时间）。

#### 6.3.2 自定义模型设计

```cpp
// 沙箱文件列表模型（继承 QAbstractTableModel）
class SandboxFileModel : public QAbstractTableModel {
    Q_OBJECT

public:
    // 列定义
    enum Column {
        NameColumn = 0,
        SizeColumn,
        TypeColumn,
        ModifiedTimeColumn,
        ColumnCount
    };

    // 构造函数
    explicit SandboxFileModel(QObject* parent = nullptr);

    // QAbstractTableModel 接口
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation,
                       int role = Qt::DisplayRole) const override;

    // 自定义接口
    void SetDirectory(const QString& dirPath);  // 设置当前目录
    QString filePath(const QModelIndex& index) const;  // 获取文件路径

private:
    QString currentDir_;               // 当前目录
    QVector<QFileInfo> fileInfos_;   // 文件信息列表
};
```

### 6.4 搜索功能设计

#### 6.4.1 搜索实现

- **实时搜索**：用户输入关键字后实时过滤文件列表（debounce 300ms）
- **模糊匹配**：支持文件名模糊搜索（如输入"txt"会匹配所有 `.txt` 文件）
- **正则表达式**：高级用户支持正则表达式搜索（可选）

#### 6.4.2 搜索实现代码

```cpp
// 搜索框文本变化槽
void SandboxBrowser::OnSearchTextChanged(const QString& text) {
    // 防抖：300ms 后执行搜索
    searchTimer_->stop();
    searchTimer_->start(300);
    searchQuery_ = text;
}

// 执行搜索
void SandboxBrowser::PerformSearch() {
    if (searchQuery_.isEmpty()) {
        // 清空搜索，显示当前目录所有文件
        fileModel_->SetDirectory(currentDir_);
        return;
    }
    
    // 遍历当前目录及子目录，查找匹配文件
    QVector<QFileInfo> matchedFiles;
    SearchFiles(currentDir_, searchQuery_, matchedFiles);
    
    // 更新文件列表模型
    fileModel_->SetFiles(matchedFiles);
}

// 递归搜索文件
void SandboxBrowser::SearchFiles(const QString& dirPath,
                                const QString& query,
                                QVector<QFileInfo>& matchedFiles) {
    QDir dir(dirPath);
    QFileInfoList entries = dir.entryInfoList(QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot);
    
    for (const QFileInfo& entry : entries) {
        if (entry.fileName().contains(query, Qt::CaseInsensitive)) {
            matchedFiles.append(entry);
        }
        
        if (entry.isDir()) {
            // 递归搜索子目录
            SearchFiles(entry.absoluteFilePath(), query, matchedFiles);
        }
    }
}
```

### 6.5 文件操作设计

#### 6.5.1 右键菜单设计

```
+-------------------------------------+
| 打开 (Open)                         |  → 用系统默认程序打开文件
| 打开文件位置 (Reveal in Explorer)  |  → 在资源管理器中显示文件
| ----------------------------------- |
| 导出 (Export)                       |  → 复制文件出沙箱
| 删除 (Delete)                       |  → 从沙箱中删除文件
| ----------------------------------- |
| 属性 (Properties)                   |  → 显示文件详细信息
+-------------------------------------+
```

#### 6.5.2 文件操作实现

| 操作 | 实现方式 | 说明 |
|------|----------|------|
| 打开 | `QDesktopServices::openUrl(QUrl::fromLocalFile(filePath))` | 使用系统默认程序打开 |
| 打开文件位置 | `QDesktopServices::openUrl(QUrl::fromLocalFile(dirPath))` | 在资源管理器中显示 |
| 导出 | `QFileDialog::getSaveFileName()` + `QFile::copy()` | 复制文件到用户指定路径 |
| 删除 | `QFile::remove()` 或 `QDir::removeRecursively()` | 删除文件或目录（需确认） |
| 属性 | 自定义对话框显示 `QFileInfo` 属性 | 显示名称、大小、修改时间、路径等 |

---

## 7. 设置界面设计

### 7.1 设置界面布局

```
+---------------------------------------------------------------+
|  设置                            [全局设置] [单应用设置] [关于]  |
+---------------------------------------------------------------+
|  全局设置                                                      |
|  ☑ 启用 Hook 拦截（文件/注册表）                               |
|  默认沙箱路径：D:\ThinAppSandbox  [浏览]                      |
|  [清理全局缓存]                                                |
|  Hook 日志级别：□ 错误 □ 警告 □ 信息 （高级选项）              |
|                                                                 |
|  启动项管理                                                    |
|  +----------------+  [添加] [删除] [上移] [下移]             |
|  | QQ             |                                            |
|  | 微信           |                                            |
|  | Chrome         |                                            |
|  +----------------+                                            |
+---------------------------------------------------------------+
|  单应用设置                                                    |
|  选择应用：[QQ ▼]                                              |
|  启动模式：○ 沙箱模式 ● 真实模式                               |
|  Hook 规则：□ 允许写入系统目录（高级）                          |
|  启动参数：[                    ]  [保存]                        |
+---------------------------------------------------------------+
|  关于                                                          |
|  版本：v0.1.0 (MVP)                                          |
|  GitHub：https://github.com/ai-thinapp/...                    |
|  许可证：GPL v3                                                |
+---------------------------------------------------------------+
```

### 7.2 常规设置

#### 7.2.1 设置项

| 设置项 | 控件 | 默认值 | 说明 |
|--------|------|--------|------|
| 开机启动 | `QCheckBox` | 未勾选 | 通过 Windows 注册表 `HKCU\Software\Microsoft\Windows\CurrentVersion\Run` 实现 |
| 最小化到托盘 | `QCheckBox` | 勾选 | 关闭窗口时最小化到托盘，而非退出 |
| 退出确认 | `QCheckBox` | 勾选 | 点击退出时显示确认对话框 |
| 默认沙箱路径 | `QLineEdit` + `QPushButton` (浏览) | `D:\ThinAppApps` | 新应用的沙箱数据保存路径 |

### 7.3 Hook 设置

#### 7.3.1 设置项

| 设置项 | 控件 | 默认值 | 说明 |
|--------|------|--------|------|
| Hook 开关 | `QCheckBox` | 勾选 | 全局 Hook 开关（文件/注册表拦截） |
| 日志级别 | `QCheckBox` (多选) | 错误 + 警告 | 高级选项，控制 Hook 日志输出详细程度 |
| 覆盖检测频率 | `QSpinBox` (秒) | 5 秒 | Hook 覆盖检测频率（检测其他软件是否覆盖了我们的 Hook） |

### 7.4 启动项管理

#### 7.4.1 功能说明

- **功能**：管理 Windows 启动时会自动启动的应用（类似 `msconfig` 的启动项管理）
- **实现**：通过 Windows 注册表 `HKCU\Software\Microsoft\Windows\CurrentVersion\Run` 管理

#### 7.4.2 界面设计

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

- **添加**：打开文件选择器，选择 exe 文件，添加到启动项列表
- **删除**：从启动项列表中移除（从注册表中删除）
- **上移/下移**：调整启动顺序（写入注册表顺序）

### 7.5 关于界面

#### 7.5.1 内容

| 信息 | 说明 |
|------|------|
| 版本号 | `v0.1.0 (MVP)` |
| GitHub 链接 | `https://github.com/ai-thinapp/...`（点击用浏览器打开） |
| 许可证 | `GPL v3`（点击显示完整许可证文本） |
| 作者 | `AI ThinApp Team` |
| 感谢 | `MinHook`、`Qt 6`、`zlib` 等开源项目 |

---

## 8. 任务栏集成设计

### 8.1 跳转列表设计

#### 8.1.1 跳转列表内容

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

#### 8.1.2 实现方式

- **Windows API**：`ITaskbarList3::SetWindowProperties()`（需 Windows 7+）
- **Qt 支持**：Qt 6 未直接封装跳转列表 API，需调用 Windows Shell API

#### 8.1.3 代码实现

```cpp
// 设置跳转列表（Windows Shell API）
void TaskbarIntegrator::SetJumpList() {
    // 1. 创建 ICustomDestinationList 实例
    ICustomDestinationList* pDestList = nullptr;
    CoCreateInstance(CLSID_DestinationList, nullptr, CLSCTX_INPROC_SERVER,
                    IID_ICustomDestinationList, (void**)&pDestList);
    
    // 2. 创建跳转列表类别（常用应用、最近使用的应用）
    // ... (省略具体实现，需参考 Windows Shell API 文档)
    
    // 3. 提交跳转列表
    pDestList->CommitList();
    
    pDestList->Release();
}
```

### 8.2 缩略图工具栏设计

#### 8.2.1 功能说明

- **功能**：在任务栏缩略图预览中显示工具栏（如播放/暂停、上一曲/下一曲等）
- **适用场景**：媒体播放器、下载工具等需要快速操作的场景
- **MVP 阶段**：仅实现基础框架，具体应用根据实际需求定制

#### 8.2.2 实现方式

- **Windows API**：`ITaskbarList3::ThumbBarAddButtons()`
- **Qt 支持**：需调用 Windows Shell API

### 8.3 进度条设计

#### 8.3.1 功能说明

- **功能**：在任务栏图标上显示进度条（如文件复制、下载等进度）
- **适用场景**：文件操作、下载、安装等长时间操作
- **MVP 阶段**：实现基础 API，具体应用根据实际需求调用

#### 8.3.2 实现方式

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

## 9. 性能优化设计

### 9.1 启动速度优化

#### 9.1.1 优化策略

| 策略 | 说明 | 预期效果 |
|------|------|----------|
| 延迟加载 | 启动时仅加载必要模块（应用列表），其他模块（沙箱浏览器、设置）按需加载 | 启动时间减少 30% |
| 缓存应用图标 | 启动时后台加载应用图标并缓存（避免重复读取磁盘） | 图标加载时间减少 80% |
| 异步加载应用列表 | 应用列表从配置文件加载时使用后台线程 | UI 响应速度提升 50% |

### 9.2 内存优化

#### 9.2.1 优化策略

| 策略 | 说明 | 预期效果 |
|------|------|----------|
| 懒加载 | 沙箱浏览器仅在用户打开时加载目录树和文件列表 | 内存占用减少 40% |
| 对象池 | 复用 `QListWidgetItem`、`QTableWidgetItem` 等对象 | 内存碎片减少 30% |
| 及时释放 | 关闭沙箱浏览器时释放目录树和文件列表内存 | 内存占用峰值降低 20% |

### 9.3 响应速度优化

#### 9.3.1 优化策略

| 策略 | 说明 | 预期效果 |
|------|------|----------|
| 异步加载 | 文件浏览器遍历目录时使用后台线程 | UI 卡顿时间减少 90% |
| 线程池 | 使用 `QThreadPool` 管理后台任务（限制并发线程数） | CPU 占用降低 20% |
| 防抖/节流 | 搜索框输入、窗口 resize 等操作使用防抖/节流 | 无效操作减少 70% |

---

## 10. 开发任务拆解

### 10.1 任务拆解（按天级别）

#### Week 7：主窗口 + 托盘模块（5 天 × 2 人 = 10 人天）

| 任务 ID | 任务名称 | 负责人 | 工时 | 产出 |
|---------|---------|--------|------|------|
| T7.1 | 实现应用列表模型（`AppListModel`） | Dev A | 1d | `src/launchpad/app_model.cpp` |
| T7.2 | 实现主窗口 UI（应用卡片、搜索栏、过滤器） | UX + Dev B | 2d | `src/launchpad/ui/main_window.cpp` (重构) |
| T7.3 | 实现托盘模块（图标、右键菜单、balloon 通知） | Dev A | 1d | `src/launchpad/ui/tray_manager.cpp` |
| T7.4 | 实现应用启动/停止（Hook DLL 注入、进程管理） | Dev A | 2d | `src/launchpad/app_launcher.cpp` |
| T7.5 | 代码审查 + 修复 Bug | Dev A + QA | 1d | 代码审查报告 |

**验收标准**：
- 应用列表加载成功（从配置文件读取）
- 应用启动/停止功能正常（Hook DLL 注入成功）
- 托盘图标显示正常，右键菜单功能完整
- 单元测试覆盖率 ≥ 80%

---

#### Week 8：应用管理模块 + 沙箱文件浏览器（5 天 × 2 人 = 10 人天）

| 任务 ID | 任务名称 | 负责人 | 工时 | 产出 |
|---------|---------|--------|------|------|
| T8.1 | 实现应用添加/删除/更新（配置文件持久化） | Dev A | 1d | `src/launchpad/app_config_store.cpp` |
| T8.2 | 实现沙箱目录树组件（`SandboxDirModel` + `QTreeView`） | Dev B | 2d | `src/launchpad/ui/sandbox_dir_view.cpp` |
| T8.3 | 实现沙箱文件列表组件（`SandboxFileModel` + `QTableView`） | Dev B | 2d | `src/launchpad/ui/sandbox_file_view.cpp` |
| T8.4 | 实现文件操作（打开、导出、删除、属性） | Dev B | 1d | `src/launchpad/ui/sandbox_file_view.cpp` (更新) |
| T8.5 | 代码审查 + 修复 Bug | Dev B + QA | 1d | 代码审查报告 |

**验收标准**：
- 应用添加/删除/更新功能正常（配置文件正确读写）
- 沙箱目录树和文件列表显示正确（虚拟路径映射正确）
- 文件操作功能完整（打开、导出、删除、属性）
- 单元测试覆盖率 ≥ 80%

---

#### Week 9：设置界面 + 任务栏集成（5 天 × 2 人 = 10 人天）

| 任务 ID | 任务名称 | 负责人 | 工时 | 产出 |
|---------|---------|--------|------|------|
| T9.1 | 实现设置界面（常规设置、Hook 设置、启动项管理） | UX + Dev A | 2d | `src/launchpad/ui/settings_dialog.cpp` |
| T9.2 | 实现任务栏集成（跳转列表、缩略图工具栏、进度条） | Dev B | 2d | `src/launchpad/ui/taskbar_integrator.cpp` |
| T9.3 | 实现关于界面（版本号、GitHub 链接、许可证） | Dev A | 0.5d | `src/launchpad/ui/about_dialog.cpp` |
| T9.4 | 编写设置界面和任务栏集成测试 | QA | 1d | `tests/launchpad/test_settings.cpp` |
| T9.5 | 代码审查 + 修复 Bug | Dev A + Dev B + QA | 1d | 代码审查报告 |

**验收标准**：
- 设置界面功能完整（常规设置、Hook 设置、启动项管理）
- 任务栏集成功能正常（跳转列表、缩略图工具栏、进度条）
- 关于界面显示正确（版本号、链接、许可证）
- 单元测试覆盖率 ≥ 80%

---

#### Week 10：性能优化 + 测试 + Bug 修复（5 天 × 2 人 = 10 人天）

| 任务 ID | 任务名称 | 负责人 | 工时 | 产出 |
|---------|---------|--------|------|------|
| T10.1 | 性能优化（启动速度、内存、响应速度） | Dev A + Dev B | 2d | 性能测试报告 |
| T10.2 | 集成测试（10 款常用应用启动/停止成功） | QA + Dev A + Dev B | 2d | 集成测试报告 |
| T10.3 | Bug 修复（根据测试报告修复 Bug） | Dev A + Dev B | 3d | Bug 修复报告 |
| T10.4 | 用户文档（用户手册、FAQ） | PM + UX | 2d | `docs/USER-MANUAL.md`、`docs/FAQ.md` |
| T10.5 | 发布准备（CHANGELOG、GitHub Release） | PM | 1d | `CHANGELOG.md`、GitHub Release v0.1.0 |

**验收标准**：
- 性能测试通过（启动时间 < 2 秒、内存占用 < 100MB）
- 集成测试通过（10 款常用应用启动/停止成功）
- Bug 修复率 ≥ 90%（严重 Bug 全部修复）
- 用户文档完整（用户手册、FAQ）

### 10.2 任务拆解汇总

| 阶段 | 时间 | 工时 | 关键交付物 |
|------|------|------|------------|
| Week 7 | 5 天 | 10 人天 | 主窗口 + 托盘模块 |
| Week 8 | 5 天 | 10 人天 | 应用管理模块 + 沙箱文件浏览器 |
| Week 9 | 5 天 | 10 人天 | 设置界面 + 任务栏集成 |
| Week 10 | 5 天 | 10 人天 | 性能优化 + 测试 + Bug 修复 |
| **总计** | **20 天** | **40 人天** | Launchpad v1.0 |

### 10.3 依赖关系

| 任务 | 依赖任务 | 说明 |
|------|----------|------|
| T7.4（应用启动/停止） | T7.1（应用列表模型） | 需要先有应用列表模型 |
| T8.2（沙箱目录树） | T7.1（应用列表模型） | 需要获取应用目录路径 |
| T8.3（沙箱文件列表） | T8.2（沙箱目录树） | 需要目录树选择事件触发文件列表更新 |
| T9.2（任务栏集成） | T7.4（应用启动/停止） | 需要应用启动/停止事件更新跳转列表 |
| T10.1（性能优化） | T7.1-T9.3（所有功能） | 需要所有功能完成后才能优化 |

---

## 11. 风险与依赖

### 11.1 依赖

| 依赖项 | 来源 | 说明 |
|--------|------|------|
| `docs/RISK-REGISTER.md` (R-UX-01) | 风险登记册 | 任务栏集成被杀软拦截（需 Windows Shell API 调用，可能被杀软视为可疑行为） |
| `docs/ARCHITECTURE.md` (Hook 引擎) | 技术架构文档 | 应用启动依赖 Hook 引擎完整性（Hook DLL 需正确实现） |
| `docs/SANDBOX-DIR-VISUALIZATION.md` | 沙箱目录可视化方案 | 沙箱文件浏览器需集成方案 A（树状文件浏览器） |

### 11.2 风险

| 风险 ID | 风险描述 | 影响 | 概率 | 等级 | 缓释措施 | 负责人 |
|---------|----------|------|------|----------|----------|--------|
| R-UI-01 | 任务栏集成实现复杂（需要 Windows Shell API） | 高 | 中 | 高 | 提前研究 Windows Shell API；若实现困难，MVP 阶段仅实现基础跳转列表 | Dev B |
| R-UI-02 | 沙箱文件浏览器性能问题（文件数量 > 10000 时） | 中 | 中 | 中 | 使用懒加载、虚拟滚动、后台索引等优化策略 | Dev B |
| R-UI-03 | Hook DLL 注入失败（杀软拦截） | 高 | 高 | 极高 | 代码签名 + 杀软白名单申请；POC 阶段提示用户关闭杀软 | PM |
| R-UI-04 | Qt 6 与 Windows 11 风格不一致（Fluent UI） | 低 | 中 | 低 | 使用自定义委托绘制 Fluent UI 风格卡片 | UX + Dev B |

### 11.3 风险记录

若发现新的技术难点（如任务栏集成复杂度），立即记录到 `docs/MVP-BLOCKERS.md` 并上报 PM。

---

## 12. 验收标准（详细）

### 12.1 功能验收标准

| ID | 验收项 | 成功标准 | 测试方法 | 优先级 |
|----|--------|----------|----------|--------|
| A1 | 应用列表加载成功 | 从配置文件加载应用列表，显示正确 | 功能测试 | P0 |
| A2 | 应用启动/停止成功 | 点击启动/停止按钮，应用正常运行/停止 | 功能测试 | P0 |
| A3 | Hook DLL 注入成功 | Hook DLL 成功注入目标进程，无错误 | 功能测试 + 日志检查 | P0 |
| A4 | 托盘图标显示正确 | 托盘图标根据不同状态显示不同图标 | 功能测试 | P1 |
| A5 | 托盘右键菜单功能完整 | 所有菜单项功能正常 | 功能测试 | P1 |
| A6 | Balloon 通知显示正确 | 应用启动/停止/崩溃时显示通知 | 功能测试 | P1 |
| A7 | 沙箱目录树显示正确 | 目录树正确映射 `{AppDir}\VFS\` 虚拟路径 | 功能测试 | P0 |
| A8 | 沙箱文件列表显示正确 | 文件列表显示当前目录的文件和属性 | 功能测试 | P0 |
| A9 | 文件操作功能完整 | 打开、导出、删除、属性功能正常 | 功能测试 | P1 |
| A10 | 设置界面功能完整 | 常规设置、Hook 设置、启动项管理功能正常 | 功能测试 | P1 |
| A11 | 任务栏集成功能正常 | 跳转列表、缩略图工具栏、进度条功能正常 | 功能测试 | P2 |
| A12 | 关于界面显示正确 | 版本号、GitHub 链接、许可证显示正确 | 功能测试 | P2 |

### 12.2 性能验收标准

| ID | 验收项 | 成功标准 | 测试方法 | 优先级 |
|----|--------|----------|----------|--------|
| P1 | 启动时间 < 2 秒 | 使用 `QElapsedTimer` 测量启动时间，取平均值 | 性能测试 | P1 |
| P2 | 内存占用 < 100MB | 使用 Task Manager 或 `QProcess` 测量内存占用 | 性能测试 | P1 |
| P3 | 应用列表加载时间 < 1 秒（100 个应用以内） | 使用 `QElapsedTimer` 测量加载时间 | 性能测试 | P1 |
| P4 | 沙箱文件列表加载时间 < 2 秒（1000 个文件以内） | 使用 `QElapsedTimer` 测量加载时间 | 性能测试 | P1 |
| P5 | 搜索响应时间 < 500ms（5000 个文件以内） | 使用 `QElapsedTimer` 测量响应时间 | 性能测试 | P1 |

### 12.3 用户体验验收标准

| ID | 验收项 | 成功标准 | 测试方法 | 优先级 |
|----|--------|----------|----------|--------|
| U1 | 界面符合 Fluent UI 风格 | 界面样式与 Windows 11 风格一致 | 设计评审 | P0 |
| U2 | 交互符合 Windows 原生规范 | 交互方式符合 Windows 用户体验规范 | 专家评审 | P1 |
| U3 | 小白用户能理解界面功能 | 可用性测试中，非技术用户能正确解释界面各元素的功能 | 可用性测试（访谈） | P0 |
| U4 | 视觉干扰小 | 用户对界面视觉干扰的评分 ≤ 2 分（1-5 分量表） | 可用性测试（问卷） | P2 |

---

## 13. 修订历史

| 版本 | 日期 | 作者 | 变更说明 |
|------|------|------|----------|
| 0.1 | 2026-05-23 | Qt/C++ 软件架构师 | 初版，基于 POC 代码和项目文档输出 |

---

## 附录 A：参考资料

1. **Qt 6 官方文档**：https://doc.qt.io/qt-6/
2. **Windows Shell API 文档**：https://docs.microsoft.com/en-us/windows/win32/shell/
3. **MinHook 官方文档**：https://github.com/TsudaKageyu/minhook
4. **Fluent UI 设计语言**：https://www.microsoft.com/design/fluent/
5. **docs/ARCHITECTURE.md**：技术架构文档
6. **docs/MVP-SCOPE.md**：MVP 范围定义
7. **docs/MVP-PLAN.md**：MVP 开发计划
8. **docs/RISK-REGISTER.md**：风险登记册
9. **docs/UX-RESEARCH.md**：竞品交互范式研究
10. **docs/UX-DESIGN.md**：交互设计文档
11. **docs/SANDBOX-VISUAL-IDENTITY.md**：沙箱视觉标识方案
12. **docs/SANDBOX-DIR-VISUALIZATION.md**：沙箱目录结构可视化方案

---

## 附录 B：术语表

| 术语 | 定义 |
|------|------|
| **VFS** | Virtual File System，虚拟文件系统，指向 `{AppDir}\VFS\` 目录 |
| **Hook** | 钩子，一种拦截 API 调用的技术 |
| **MinHook** | 一个开源的 API Hook 引擎 |
| **Qt 6** | 一个跨平台的 C++ GUI 框架 |
| **QProcess** | Qt 框架中用于启动外部进程的类 |
| **QSystemTrayIcon** | Qt 框架中用于系统托盘图标的类 |
| **QAbstractListModel** | Qt 框架中用于自定义列表模型的抽象类 |
| **QTreeView** | Qt 框架中用于显示树状数据的控件 |
| **QTableView** | Qt 框架中用于显示表格数据的控件 |
| **Jump List** | Windows 任务栏跳转列表 |
| **Thumbnail Toolbar** | Windows 任务栏缩略图工具栏 |
| **Fluent UI** | Microsoft 的设计语言 |
| **POC** | Proof of Concept，概念验证 |
| **MVP** | Minimum Viable Product，最小可行产品 |

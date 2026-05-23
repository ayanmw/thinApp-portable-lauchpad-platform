# MVP Week 7 任务理解确认

> 本文档确认对 AI ThinApp Portable Launchpad Platform 项目 MVP Week 7 开发任务的理解。
> 日期：2026-05-23 | 作者：Qt/C++ 软件开发工程师

---

## 1. 任务概述

**目标**：实现 Launchpad 主窗口（800×600，应用卡片网格布局）和系统托盘模块（托盘图标、右键菜单、Balloon 通知）

**时间**：5 天（2026-05-23 至 2026-05-29）

**产出**：
- 主窗口 UI（应用卡片、搜索栏、过滤器、右键菜单）
- 托盘模块（托盘图标、右键菜单、Balloon 通知）
- 应用启动/停止功能（Hook DLL 注入、进程管理）
- 应用列表模型（`AppListModel`）

---

## 2. Week 7 任务拆解（详细）

### 2.1 任务 T7.1：实现应用列表模型（`AppListModel`）

**工时**：1 天

**产出**：`src/launchpad/app_model.h/cpp`

**详细任务**：
1. 定义 `AppInfo` 结构体（包含应用目录、名称、exe 路径、图标路径、版本、描述、是否已便携化、是否正在运行、进程指针、上次启动时间）
2. 实现 `AppListModel` 类（继承 `QAbstractListModel`）
   - 实现 `rowCount()`、`data()`、`roleNames()` 等接口
   - 定义角色（NameRole、VersionRole、IsRunningRole、IsPortableRole、IconRole、LastLaunchTimeRole）
   - 实现应用管理接口（`AddApp()`、`RemoveApp()`、`UpdateApp()`、`GetAppInfo()`、`GetAllApps()`）
   - 实现过滤/排序接口（`FilterApps()`、`SortApps()`）
3. 编写单元测试（`tests/launchpad/test_app_model.cpp`）

**验收标准**：
- ✅ `AppListModel` 正确加载应用列表（从 `apps.json` 读取）
- ✅ 模型数据变化信号（`AppAdded`、`AppRemoved`、`AppUpdated`、`ModelChanged`）正常发射
- ✅ 过滤和排序功能正常
- ✅ 单元测试覆盖率 ≥ 80%

---

### 2.2 任务 T7.2：实现主窗口 UI（应用卡片、搜索栏、过滤器）

**工时**：2 天

**产出**：`src/launchpad/ui/main_window.h/cpp`（重构）

**详细任务**：
1. 修改 `main_window.h`
   - 添加成员变量（`QListView *app_list_view_`、`QStandardItemModel *app_model_`、`QSearchBox *search_box_`）
   - 添加信号（`void appLaunched(QString app_path)`、`void appTermated(QString app_path)`）
   - 添加槽（`void onAppSelected(QModelIndex index)`、`void onSearchTextChanged(QString text)`）

2. 修改 `main_window.cpp`
   - 实现构造函数（初始化 UI、连接信号/槽、加载应用列表）
   - 实现 `initUI()` 方法（创建左侧导航栏、右侧应用网格、顶部搜索栏）
   - 实现 `loadAppList()` 方法（从 `apps.json` 读取应用列表，填充 `QStandardItemModel`）
   - 实现 `onAppSelected()` 槽（双击应用卡片 → 发射 `appLaunched` 信号）
   - 实现 `onSearchTextChanged()` 槽（实时过滤应用列表）
   - 实现 `contextMenuEvent()` 方法（右键菜单：启动/打开目录/删除）

3. 创建 `app_card.h/cpp`
   - 定义 `AppCard` 类（继承 `QWidget`）
   - 实现构造函数（初始化 UI、设置样式表）
   - 实现 `setAppInfo()` 方法（设置应用图标、名称、沙箱状态）
   - 实现 `paintEvent()` 方法（绘制沙箱标识边框）

4. 编写单元测试（`tests/launchpad/test_main_window.cpp`）

**验收标准**：
- ✅ 主窗口正常显示（800×600，应用卡片网格布局）
- ✅ 应用列表正常加载（从 `apps.json` 读取）
- ✅ 搜索框正常过滤（实时过滤应用列表）
- ✅ 右键菜单正常显示（启动/打开目录/删除）
- ✅ 双击应用卡片正常启动应用（调用 `AppManager::launchApp()`）
- ✅ 单元测试覆盖率 ≥ 80%

---

### 2.3 任务 T7.3：实现托盘模块（图标、右键菜单、Balloon 通知）

**工时**：1 天

**产出**：`src/launchpad/ui/tray_manager.h/cpp`

**详细任务**：
1. 创建 `tray_manager.h`
   - 定义 `TrayManager` 类（继承 `QObject`）
   - 添加成员变量（`QSystemTrayIcon *tray_icon_`、`QMenu *tray_menu_`）
   - 添加方法（`ShowNotification()`、`SetTrayIcon()`）

2. 创建 `tray_manager.cpp`
   - 实现构造函数（初始化托盘图标、创建右键菜单、连接信号/槽）
   - 实现 `ShowNotification()` 方法（调用 `QSystemTrayIcon::showMessage()`）
   - 实现 `SetTrayIcon()` 方法（根据应用状态设置托盘图标）

3. 修改 `main.cpp`
   - 添加 `QSystemTrayIcon *tray_icon_` 成员变量
   - 在 `main()` 中初始化托盘图标（`QIcon(":/icons/tray_icon.png")`）
   - 连接托盘图标的信号/槽（`activated()` → 显示/隐藏主窗口）

4. 创建 `tray_menu.h/cpp`
   - 定义 `TrayMenu` 类（继承 `QMenu`）
   - 实现构造函数（创建菜单项、连接信号/槽）
   - 实现 `showLaunchpad()` 槽（显示主窗口）
   - 实现 `showSettings()` 槽（显示设置界面）
   - 实现 `exitApp()` 槽（退出应用）

5. 编写单元测试（`tests/launchpad/test_tray_manager.cpp`）

**验收标准**：
- ✅ 托盘图标正常显示（含沙箱标识绿色圆点）
- ✅ 右键菜单正常显示（显示 Launchpad/设置/退出）
- ✅ 单击托盘图标正常显示/隐藏主窗口
- ✅ 应用启动/停止后正常显示 Balloon 通知（标题 + 内容 + 图标）
- ✅ 退出菜单项正常退出应用（保存应用列表、清理资源）
- ✅ 单元测试覆盖率 ≥ 80%

---

### 2.4 任务 T7.4：实现应用启动/停止（Hook DLL 注入、进程管理）

**工时**：2 天

**产出**：`src/launchpad/app_launcher.h/cpp`

**详细任务**：
1. 修改 `app_manager.h`
   - 添加 `launchApp()` 方法声明
   - 添加 `terminateApp()` 方法声明
   - 添加 `addApp()` 方法声明
   - 添加 `removeApp()` 方法声明
   - 添加 `showBalloonNotification()` 方法声明

2. 修改 `app_manager.cpp`
   - 实现 `launchApp()` 方法（调用 `HookEngine::InjectHookDll()` 注入 Hook DLL，然后 `QProcess::start()` 启动应用）
   - 实现 `terminateApp()` 方法（遍历进程树，强制终止）
   - 实现 `addApp()` 方法（浏览 .vapp 包，解压，注册到 `apps.json`）
   - 实现 `removeApp()` 方法（从 `apps.json` 删除，删除应用目录）
   - 实现 `showBalloonNotification()` 方法（调用 `QSystemTrayIcon::showMessage()`）

3. 实现 Hook DLL 注入（`InjectHookDll()`）
   - 设置环境变量（`AI_THINAPP_APP_DIR`）
   - 远程线程注入 Hook DLL（使用 Windows API：`VirtualAllocEx()`、`WriteProcessMemory()`、`CreateRemoteThread()`）

4. 实现进程树遍历（`TerminateProcessTree()`）
   - 使用 `CreateToolhelp32Snapshot()` 遍历进程
   - 递归终止子进程

5. 编写单元测试（`tests/launchpad/test_app_launcher.cpp`）

**验收标准**：
- ✅ 应用启动功能正常（调用 `AppManager::launchApp()`）
- ✅ 应用停止功能正常（调用 `AppManager::terminateApp()`）
- ✅ Hook DLL 注入成功（调用 `HookEngine::InjectHookDll()`）
- ✅ 应用启动/停止后正常显示 Balloon 通知
- ✅ 单元测试覆盖率 ≥ 80%

---

### 2.5 任务 T7.5：代码审查 + 修复 Bug

**工时**：1 天

**产出**：代码审查报告

**详细任务**：
1. 代码审查（Dev A + QA）
   - 检查代码规范（命名、注释、格式）
   - 检查线程安全（信号/槽跨线程通信、共享数据保护）
   - 检查内存管理（智能指针使用、对象树管理、缓存管理）
   - 检查性能（启动速度、内存占用、响应速度）

2. 修复 Bug（根据代码审查报告）
   - 修复功能 Bug
   - 修复性能问题
   - 修复线程安全问题
   - 修复内存泄漏

3. 编写代码审查报告（`docs/MVP-WEEK7-CODE-REVIEW.md`）

**验收标准**：
- ✅ 代码审查完成（所有问题已记录）
- ✅ Bug 修复完成（所有严重 Bug 已修复）
- ✅ 代码审查报告输出

---

## 3. 技术方案理解

### 3.1 Qt 6 框架

- **GUI 框架**：Qt 6（POC 阶段验证通过，原生性能，LGPL 协议兼容）
- **核心控件**：
  - `QSystemTrayIcon`：系统托盘图标
  - `QMenu`：右键菜单
  - `QAbstractListModel`：应用列表模型
  - `QListView`：应用列表视图
  - `QProcess`：进程管理
- **信号/槽机制**：跨线程通信（自动队列化，线程安全）

### 3.2 信号/槽设计（跨模块通信）

**核心信号/槽**：

| 信号发送者 | 信号 | 接收者 | 槽 | 用途 |
|------------|------|--------|-----|------|
| `AppManager` | `AppLaunched(appDir)` | `MainWindow` | `OnAppLaunched(appDir)` | 更新应用状态（运行中） |
| `AppManager` | `AppStopped(appDir)` | `MainWindow` | `OnAppStopped(appDir)` | 更新应用状态（已停止） |
| `AppManager` | `AppCrashed(appDir, exitCode)` | `TrayManager` | `ShowCrashNotification(appDir, exitCode)` | 显示崩溃通知 |
| `AppLauncher` | `HookInjectionFailed(appDir, error)` | `TrayManager` | `ShowErrorNotification(error)` | 显示 Hook 注入失败通知 |
| `MainWindow` | `AppSelected(appDir)` | `AppDetailWidget` | `ShowAppDetail(appDir)` | 显示应用详情 |
| `MainWindow` | `SearchQueryChanged(query)` | `AppListModel` | `FilterApps(query)` | 过滤应用列表 |

**信号/槽连接示例**：

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

### 3.3 线程模型

**线程划分**：

| 线程 | 职责 | 说明 |
|------|------|------|
| **UI 线程**（主线程） | 界面渲染、用户交互 | Qt 主事件循环（`QApplication::exec()`） |
| **Hook 引擎线程** | Hook DLL 注入、API Hook 安装 | 使用 `QtConcurrent` 或 `QThread` 异步执行，避免阻塞 UI |
| **应用启动线程** | 创建进程、等待启动完成 | 使用 `QProcess::start()` 异步启动，通过 `QProcess::finished` 信号通知完成 |
| **文件浏览器后台线程** | 遍历沙箱目录、计算文件大小 | 使用 `QtConcurrent::run()` 后台遍历，通过信号通知 UI 更新 |
| **应用数据加载线程** | 从配置文件加载应用列表 | 启动时后台加载，避免界面卡顿 |

**线程安全策略**：
- **UI 线程与其他线程通信**：使用 Qt 信号/槽（自动队列化，跨线程安全）
- **共享数据保护**：使用 `QMutex` 或 `QReadWriteLock` 保护共享数据（如应用列表）
- **后台任务取消**：使用 `QFutureWatcher` + `QFuture::cancel()` 支持任务取消

**线程模型示例代码**：

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

### 3.4 内存管理

**智能指针使用策略**：

| 场景 | 智能指针类型 | 说明 |
|------|--------------|------|
| **QObject 父子关系** | 不使用智能指针 | Qt 对象树自动管理（父对象销毁时自动销毁子对象） |
| **非 QObject 对象** | `std::unique_ptr` | 独占所有权，自动释放 |
| **共享数据** | `std::shared_ptr` | 共享所有权，引用计数为 0 时释放 |
| **交叉引用** | `std::weak_ptr` | 避免循环引用（如 `AppInfo` 引用 `AppManager`） |

**对象树管理**：
- **原则**：所有 `QObject` 派生类对象必须设置 parent（父对象）
- **示例**：
  ```cpp
  // 正确：设置 parent，由 Qt 对象树管理生命周期
  QPushButton* button = new QPushButton("启动", this);
  
  // 错误：未设置 parent，可能导致内存泄漏
  QPushButton* button = new QPushButton("启动");
  ```

**缓存管理**：
- **应用图标缓存**：使用 `QCache<QString, QIcon>` 缓存应用图标（key 为应用路径）
- **文件信息缓存**：使用 `QCache<QString, QFileInfo>` 缓存文件信息（避免重复读取磁盘）

---

## 4. 验收标准（详细）

### 4.1 功能验收标准

| ID | 验收项 | 成功标准 | 测试方法 | 优先级 |
|----|--------|----------|----------|--------|
| A1 | 应用列表加载成功 | 从配置文件加载应用列表，显示正确 | 功能测试 | P0 |
| A2 | 应用启动/停止成功 | 点击启动/停止按钮，应用正常运行/停止 | 功能测试 | P0 |
| A3 | Hook DLL 注入成功 | Hook DLL 成功注入目标进程，无错误 | 功能测试 + 日志检查 | P0 |
| A4 | 托盘图标显示正确 | 托盘图标根据不同状态显示不同图标 | 功能测试 | P1 |
| A5 | 托盘右键菜单功能完整 | 所有菜单项功能正常 | 功能测试 | P1 |
| A6 | Balloon 通知显示正确 | 应用启动/停止/崩溃时显示通知 | 功能测试 | P1 |

### 4.2 性能验收标准

| ID | 验收项 | 成功标准 | 测试方法 | 优先级 |
|----|--------|----------|----------|--------|
| P1 | 启动时间 < 2 秒 | 使用 `QElapsedTimer` 测量启动时间，取平均值 | 性能测试 | P1 |
| P2 | 内存占用 < 100MB | 使用 Task Manager 或 `QProcess` 测量内存占用 | 性能测试 | P1 |
| P3 | 应用列表加载时间 < 1 秒（100 个应用以内） | 使用 `QElapsedTimer` 测量加载时间 | 性能测试 | P1 |

### 4.3 用户体验验收标准

| ID | 验收项 | 成功标准 | 测试方法 | 优先级 |
|----|--------|----------|----------|--------|
| U1 | 界面符合 Fluent UI 风格 | 界面样式与 Windows 11 风格一致 | 设计评审 | P0 |
| U2 | 交互符合 Windows 原生规范 | 交互方式符合 Windows 用户体验规范 | 专家评审 | P1 |
| U3 | 小白用户能理解界面功能 | 可用性测试中，非技术用户能正确解释界面各元素的功能 | 可用性测试（访谈） | P0 |

---

## 5. 风险与依赖

### 5.1 依赖

| 依赖项 | 来源 | 说明 |
|--------|------|------|
| `docs/RISK-REGISTER.md` (R-UX-01) | 风险登记册 | 任务栏集成被杀软拦截（需 Windows Shell API 调用，可能被杀软视为可疑行为） |
| `docs/ARCHITECTURE.md` (Hook 引擎) | 技术架构文档 | 应用启动依赖 Hook 引擎完整性（Hook DLL 需正确实现） |
| `docs/SANDBOX-DIR-VISUALIZATION.md` | 沙箱目录可视化方案 | 沙箱文件浏览器需集成方案 A（树状文件浏览器） |

### 5.2 风险

| 风险 ID | 风险描述 | 影响 | 概率 | 等级 | 缓释措施 | 负责人 |
|---------|----------|------|------|----------|----------|--------|
| R-UI-01 | 任务栏集成实现复杂（需要 Windows Shell API） | 高 | 中 | 高 | 提前研究 Windows Shell API；若实现困难，MVP 阶段仅实现基础跳转列表 | Dev B |
| R-UI-03 | Hook DLL 注入失败（杀软拦截） | 高 | 高 | 极高 | 代码签名 + 杀软白名单申请；POC 阶段提示用户关闭杀软 | PM |

### 5.3 风险记录

若发现新的技术难点（如任务栏集成复杂度），立即记录到 `docs/MVP-BLOCKERS.md` 并上报 PM。

---

## 6. 交付物清单

| 文件 | 路径 | 状态 |
|------|------|------|
| 任务理解确认 | `docs/MVP-WEEK7-TASK-CONFIRM.md` | 已完成 |
| 应用列表模型头文件 | `src/launchpad/app_model.h` | 待完成 |
| 应用列表模型实现 | `src/launchpad/app_model.cpp` | 待完成 |
| 主窗口头文件 | `src/launchpad/ui/main_window.h` | 待完成 |
| 主窗口实现 | `src/launchpad/ui/main_window.cpp` | 待完成 |
| 应用卡片头文件 | `src/launchpad/ui/app_card.h` | 待完成 |
| 应用卡片实现 | `src/launchpad/ui/app_card.cpp` | 待完成 |
| 托盘管理器头文件 | `src/launchpad/ui/tray_manager.h` | 待完成 |
| 托盘管理器实现 | `src/launchpad/ui/tray_manager.cpp` | 待完成 |
| 托盘菜单头文件 | `src/launchpad/ui/tray_menu.h` | 待完成 |
| 托盘菜单实现 | `src/launchpad/ui/tray_menu.cpp` | 待完成 |
| 应用启动器头文件 | `src/launchpad/app_launcher.h` | 待完成 |
| 应用启动器实现 | `src/launchpad/app_launcher.cpp` | 待完成 |
| main.cpp（修改） | `src/launchpad/main.cpp` | 待完成 |
| 应用管理器（修改） | `src/launchpad/app_manager.h/cpp` | 待完成 |
| Week 7 完成报告 | `docs/MVP-WEEK7-COMPLETE-REPORT.md` | 待完成 |

---

## 7. 总结

**任务理解确认**：
- ✅ 已理解 Week 7 所有开发任务（主窗口 + 托盘模块）
- ✅ 已理解技术方案（Qt 6、信号/槽、线程模型、内存管理）
- ✅ 已理解验收标准（功能、性能、用户体验）
- ✅ 已理解风险与依赖（Hook DLL 注入、杀软拦截、任务栏集成）

**下一步行动**：
1. 开始实现应用列表模型（`AppListModel`）
2. 实现主窗口 UI（应用卡片、搜索栏、过滤器）
3. 实现托盘模块（托盘图标、右键菜单、Balloon 通知）
4. 实现应用启动/停止（Hook DLL 注入、进程管理）
5. 代码审查 + 修复 Bug
6. 输出 Week 7 完成报告

---

**确认人**：Qt/C++ 软件开发工程师
**日期**：2026-05-23

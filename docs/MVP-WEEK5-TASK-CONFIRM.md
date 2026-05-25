# MVP Week 5 任务理解确认

> 本文档确认对 AI ThinApp Portable Launchpad Platform 项目 MVP Week 5 性能优化任务的理解。
> 日期：2026-05-23 | 负责人：Qt/C++ 性能优化工程师

---

## 1. 任务概述

Week 5 的任务是 **Launchpad 性能优化**，目标是提升应用启动速度、降低内存占用、提高 UI 响应速度。

### 1.1 性能目标

| 指标 | 目标值 | 测试次数 | 验收标准 |
|------|--------|----------|----------|
| 启动速度 | < 2s | 100 次 | 平均值 < 2s |
| 内存占用 | < 100MB | 100 次 | 平均值 < 100MB |
| UI 响应速度 | < 100ms | 100 次 | 平均值 < 100ms |

### 1.2 技术方案

根据设计文档第 9 章，采用以下技术方案：

| 优化方向 | 技术方案 | 预期效果 |
|----------|----------|----------|
| **启动速度** | 异步初始化（`QtConcurrent::run()`）、启动画面（`QSplashScreen`）、进度条 | 启动时间减少 50% |
| **内存占用** | 图标压缩（256x256 → 32x32）、图标缓存（`QCache`）、虚拟化（`QListView`） | 内存占用减少 40% |
| **UI 响应速度** | 搜索结果缓存（`QCache`）、去抖动（`QTimer::singleShot()`） | 响应时间减少 70% |

---

## 2. 详细任务拆解

### 2.1 第一步：读取设计文档（已完成）

- ✅ 读取 `docs/MVP-LAUNCHPAD-DESIGN.md` 中的"Launchpad 性能优化设计"章节（第 9 章）
- ✅ 提取 Week 5 所有开发任务
- ✅ 理解技术方案（懒加载、图标缓存、搜索结果缓存、异步初始化、图标压缩、虚拟化）
- ✅ 输出任务理解确认到 `docs/MVP-WEEK5-TASK-CONFIRM.md`

### 2.2 第二步：优化启动速度（Day 1-2）

**目标**：启动速度 < 2s，使用异步初始化

#### 2.2.1 修改 `src/launchpad/main.cpp`

**任务**：
1. 将 `AppManager::Instance()->LoadApps()` 改为异步加载（使用 `QtConcurrent::run()`）
2. 显示启动画面（`QSplashScreen`），显示加载进度
3. 加载完成后隐藏启动画面，显示主窗口

**关键代码逻辑**：
```cpp
// 异步加载应用列表
auto future = QtConcurrent::run([this]() {
    AppManager::Instance()->LoadApps();
});

// 使用 QFutureWatcher 监听完成
auto watcher = new QFutureWatcher<void>(this);
connect(watcher, &QFutureWatcher<void>::finished, this, [this]() {
    splashScreen_->finish(mainWindow_);
    mainWindow_->show();
});
watcher->setFuture(future);
```

#### 2.2.2 修改 `src/launchpad/app_manager.h/cpp`

**任务**：
1. 实现 `LoadAppsAsync()` 方法（异步加载应用列表，发射 `appsLoaded()` 信号）
2. 添加 `progressChanged(int progress)` 信号（加载进度 0-100%）
3. 使用 `QMutex` 保护 `apps_` 列表（线程安全）

**关键代码逻辑**：
```cpp
// 异步加载应用列表
void AppManager::LoadAppsAsync() {
    QtConcurrent::run([this]() {
        LoadApps();  // 同步加载
        emit appsLoaded();  // 发射信号
    });
}

// 线程安全的获取应用列表
QVector<AppInfo> AppManager::GetApps() const {
    QMutexLocker locker(&mutex_);
    return apps_;
}
```

#### 2.2.3 修改 `src/launchpad/ui/main_window.cpp`

**任务**：
1. 连接 `AppManager::appsLoaded()` 信号到 `onAppsLoaded()` 槽（隐藏启动画面，显示主窗口）
2. 连接 `AppManager::progressChanged(int)` 信号到 `onProgressChanged(int)` 槽（更新启动画面进度条）

**关键代码逻辑**：
```cpp
// 连接信号/槽
connect(AppManager::Instance(), &AppManager::appsLoaded,
        this, &MainWindow::onAppsLoaded);
connect(AppManager::Instance(), &AppManager::progressChanged,
        this, &MainWindow::onProgressChanged);

// 槽函数实现
void MainWindow::onAppsLoaded() {
    splashScreen_->finish(this);
    show();
}

void MainWindow::onProgressChanged(int progress) {
    splashScreen_->showMessage(QString("加载中... %1%").arg(progress),
                              Qt::AlignBottom | Qt::AlignCenter, Qt::white);
}
```

#### 2.2.4 编写测试程序 `tests/launchpad/test_startup_speed.cpp`

**测试用例**：
1. 测试用例 1：启动速度测试（记录从 `main()` 到主窗口显示的时间，验证 < 2s）
2. 测试用例 2：异步加载测试（验证加载过程中 UI 不冻结）
3. 测试用例 3：进度条更新测试（验证进度从 0% 到 100% 正确更新）

**验收标准**：
- ✅ 启动速度 < 2s（测试 100 次，平均值 < 2s）
- ✅ 异步加载成功（UI 不冻结，进度条正确更新）
- ✅ 启动画面正常显示（显示 1-2 秒，加载完成后自动隐藏）
- ✅ 单元测试覆盖率 ≥ 80%

### 2.3 第三步：优化内存占用（Day 3）

**目标**：内存占用 < 100MB，使用图标压缩 + 虚拟化

#### 2.3.1 修改 `src/launchpad/ui/app_card.cpp`

**任务**：
1. 将图标从 `QPixmap` 改为 `QIcon`（延迟加载，只在需要时加载）
2. 压缩图标大小（从 256x256 压缩到 32x32，使用 `QtImage::scaled()`）
3. 使用 `QCache<QString, QIcon>` 缓存图标（LRU 策略，最大 100 个）

**关键代码逻辑**：
```cpp
// 延迟加载图标
QIcon AppCard::GetIcon() const {
    QString appPath = appInfo_.exePath;
    
    // 查缓存
    if (iconCache_.contains(appPath)) {
        return *iconCache_.object(appPath);
    }
    
    // 加载并压缩图标
    QIcon icon(appPath);
    QPixmap pixmap = icon.pixmap(256, 256);
    QPixmap scaledPixmap = pixmap.scaled(32, 32, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    QIcon* scaledIcon = new QIcon(scaledPixmap);
    
    // 存入缓存
    iconCache_.insert(appPath, scaledIcon);
    
    return *scaledIcon;
}
```

#### 2.3.2 修改 `src/launchpad/ui/main_window.cpp`

**任务**：
1. 使用 `QListView` 虚拟化（只渲染可见区域，设置 `setUniformItemSizes(true)`）
2. 释放未使用的资源（应用退出时，调用 `QCoreApplication::removeTranslator()` 释放翻译文件）

**关键代码逻辑**：
```cpp
// 使用 QListView 虚拟化
listView_ = new QListView(this);
listView_->setUniformItemSizes(true);  // 启用虚拟化
listView_->setModel(appListModel_);
```

#### 2.3.3 编写测试程序 `tests/launchpad/test_memory_usage.cpp`

**测试用例**：
1. 测试用例 1：内存占用测试（启动 Launchpad，记录内存占用，验证 < 100MB）
2. 测试用例 2：图标缓存测试（加载 100 个应用，验证缓存命中率 ≥ 80%）
3. 测试用例 3：虚拟化测试（滚动列表，验证只渲染可见区域，内存占用不随列表长度增加）

**验收标准**：
- ✅ 内存占用 < 100MB（测试 100 次，平均值 < 100MB）
- ✅ 图标缓存命中率 ≥ 80%（测试 100 次访问，命中 ≥ 80 次）
- ✅ 虚拟化成功（列表长度 1000 个应用，内存占用 < 200MB）
- ✅ 单元测试覆盖率 ≥ 80%

### 2.4 第四步：优化 UI 响应速度（Day 4-5）

**目标**：UI 响应速度 < 100ms，使用搜索结果缓存 + 去抖动

#### 2.4.1 修改 `src/launchpad/ui/main_window.cpp`

**任务**：
1. 添加搜索结果缓存（`QCache<QString, QStandardItemModel>`）
2. 实现 `onSearchTextChangedDebounced()` 槽（去抖动，延迟 300ms 执行搜索）
3. 使用 `QTimer::singleShot()` 实现去抖动

**关键代码逻辑**：
```cpp
// 去抖动搜索
void MainWindow::onSearchTextChanged(const QString& text) {
    searchTimer_->stop();
    searchTimer_->start(300);  // 延迟 300ms
}

// 执行搜索（去抖动后）
void MainWindow::onSearchTextChangedDebounced() {
    QString text = searchEdit_->text();
    
    // 查缓存
    if (searchCache_.contains(text)) {
        appListModel_->setModel(searchCache_.object(text));
        return;
    }
    
    // 执行搜索
    QStandardItemModel* model = AppManager::Instance()->SearchApps(text);
    searchCache_.insert(text, model);
    appListModel_->setModel(model);
}
```

#### 2.4.2 修改 `src/launchpad/app_manager.cpp`

**任务**：
1. 实现 `SearchAppsCached()` 方法（优先查缓存，未命中则执行搜索并缓存结果）
2. 缓存策略：LRU，最大 50 个搜索结果

**关键代码逻辑**：
```cpp
// 缓存的搜索
QStandardItemModel* AppManager::SearchAppsCached(const QString& query) {
    // 查缓存
    if (searchCache_.contains(query)) {
        return searchCache_.object(query);
    }
    
    // 执行搜索
    QStandardItemModel* model = SearchApps(query);
    
    // 存入缓存
    searchCache_.insert(query, model);
    
    return model;
}
```

#### 2.4.3 编写测试程序 `tests/launchpad/test_ui_responsiveness.cpp`

**测试用例**：
1. 测试用例 1：搜索响应速度测试（输入搜索关键词，记录响应时间，验证 < 100ms）
2. 测试用例 2：缓存命中率测试（重复搜索 100 次，验证缓存命中率 ≥ 80%）
3. 测试用例 3：去抖动测试（快速输入 10 个字符，验证只执行 1 次搜索）

**验收标准**：
- ✅ 搜索响应速度 < 100ms（测试 100 次，平均值 < 100ms）
- ✅ 缓存命中率 ≥ 80%（重复搜索 100 次，命中 ≥ 80 次）
- ✅ 去抖动成功（快速输入 10 个字符，只执行 1 次搜索）
- ✅ 单元测试覆盖率 ≥ 80%

### 2.5 第五步：输出 Week 5 完成报告

**任务**：创建 `docs/MVP-WEEK5-COMPLETE-REPORT.md`，包含：
1. 任务完成情况（启动速度优化、内存占用优化、UI 响应速度优化）
2. 代码统计（新增代码行数、测试代码行数）
3. 性能测试结果（启动速度、内存占用、UI 响应速度）
4. 功能测试结果（异步加载、图标缓存、搜索缓存、去抖动）
5. 单元测试覆盖率
6. 已知限制（POC 阶段未解决的问题）
7. 下一步行动（Week 6 任务：应用管理模块完善、性能监控集成）

---

## 3. 交付物清单

| 文件 | 路径 | 状态 |
|------|------|------|
| 任务理解确认 | `docs/MVP-WEEK5-TASK-CONFIRM.md` | ✅ 已完成 |
| 主程序（修改） | `src/launchpad/main.cpp` | ⏳ 待完成 |
| 应用管理器（修改） | `src/launchpad/app_manager.h/cpp` | ⏳ 待完成 |
| 主窗口（修改） | `src/launchpad/ui/main_window.h/cpp` | ⏳ 待完成 |
| 应用卡片（修改） | `src/launchpad/ui/app_card.h/cpp` | ⏳ 待完成 |
| 启动速度测试 | `tests/launchpad/test_startup_speed.cpp` | ⏳ 待完成 |
| 内存占用测试 | `tests/launchpad/test_memory_usage.cpp` | ⏳ 待完成 |
| UI 响应速度测试 | `tests/launchpad/test_ui_responsiveness.cpp` | ⏳ 待完成 |
| Week 5 完成报告 | `docs/MVP-WEEK5-COMPLETE-REPORT.md` | ⏳ 待完成 |

---

## 4. 验收标准汇总

| ID | 验收项 | 成功标准 | 优先级 |
|----|--------|----------|--------|
| C1 | 启动速度优化实现完整 | 异步加载、启动画面、进度条 | P0 |
| C2 | 内存占用优化实现完整 | 图标压缩、图标缓存、虚拟化 | P0 |
| C3 | UI 响应速度优化实现完整 | 搜索缓存、去抖动 | P0 |
| C4 | 所有测试用例通过 | 3 + 3 + 3 = 9 个测试用例 | P0 |
| C5 | 性能目标达成 | 启动 < 2s、内存 < 100MB、UI 响应 < 100ms | P0 |
| C6 | 功能目标达成 | 异步加载成功、缓存命中率 ≥ 80%、去抖动成功 | P1 |
| C7 | 单元测试覆盖率 ≥ 80% | 所有模块测试覆盖率 ≥ 80% | P1 |
| C8 | 所有文件使用 UTF-8 编码（无 BOM） | 文件编码检查通过 | P2 |

---

## 5. 风险与依赖

### 5.1 风险

| 风险 ID | 风险描述 | 影响 | 概率 | 等级 | 缓释措施 |
|---------|----------|------|------|------|----------|
| R-PERF-01 | 异步加载导致崩溃（线程安全问题） | 高 | 中 | 高 | 使用 `QMutex` 保护共享数据；充分测试 |
| R-PERF-02 | 图标缓存导致内存占用反而增加 | 中 | 低 | 中 | 限制缓存大小（最大 100 个）；使用 LRU 策略 |
| R-PERF-03 | 去抖动导致搜索体验变差（延迟感） | 低 | 中 | 低 | 调整去抖动延迟时间（从 300ms 调整到 200ms） |

### 5.2 依赖

| 依赖项 | 来源 | 说明 |
|--------|------|------|
| `src/launchpad/app_manager.h/cpp` | Week 7-9 代码 | 需要修改 `AppManager` 类，添加异步加载和缓存功能 |
| `src/launchpad/ui/main_window.h/cpp` | Week 7-9 代码 | 需要修改 `MainWindow` 类，添加启动画面、虚拟化、去抖动功能 |
| `src/launchpad/ui/app_card.h/cpp` | Week 7-9 代码 | 需要修改 `AppCard` 类，添加图标延迟加载和缓存功能 |
| Qt 6 框架 | Qt 官方 | 依赖 `QtConcurrent`、`QCache`、`QTimer` 等类 |

---

## 6. 下一步行动

1. **开始第二步**：优化启动速度（Day 1-2）
   - 修改 `src/launchpad/main.cpp`
   - 修改 `src/launchpad/app_manager.h/cpp`
   - 修改 `src/launchpad/ui/main_window.cpp`
   - 编写测试程序 `tests/launchpad/test_startup_speed.cpp`

2. **完成后上报 PM**：每完成一个文件就写入到对应路径，并通知 PM 审查

3. **记录阻塞问题**：若遇到阻塞（如异步加载导致崩溃），立即记录到 `docs/MVP-BLOCKERS.md` 并上报 PM

---

## 7. 确认签名

- **确认人**：Qt/C++ 性能优化工程师
- **日期**：2026-05-23
- **状态**：✅ 任务理解确认完成，开始执行

---

## 附录 A：参考资料

1. **设计文档**：`docs/MVP-LAUNCHPAD-DESIGN.md`（第 9 章：性能优化设计）
2. **Qt 6 官方文档**：https://doc.qt.io/qt-6/qtconcurrent-index.html
3. **Qt 6 缓存框架**：https://doc.qt.io/qt-6/qcache.html
4. **Qt 6 异步编程**：https://doc.qt.io/qt-6/threads-reentrancy.html

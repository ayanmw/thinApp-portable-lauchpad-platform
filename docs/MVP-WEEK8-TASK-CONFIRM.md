# MVP Week 8 任务理解确认

> 本文档确认对 AI ThinApp Portable Launchpad Platform 项目 MVP Week 8 开发任务的理解。
> 日期：2026-05-23 | 负责人：Qt/C++ 软件开发工程师

---

## 1. 任务概述

**任务目标**：实现沙箱文件浏览器（方案 A：树状文件浏览器），完善应用管理模块

**时间**：5 天（2026-05-23 ~ 2026-05-29）

**关键交付物**：
1. 沙箱模型（`SandboxModel` 类，继承 `QAbstractItemModel`）
2. 沙箱浏览器界面（`SandboxBrowser` 类，集成 `QTreeView` + `QTableView`）
3. 应用管理模块完善（`AppManager` 类：添加/删除/启动/停止应用）
4. 单元测试代码（测试应用管理模块）

---

## 2. 设计文档理解

### 2.1 `docs/MVP-LAUNCHPAD-DESIGN.md` 关键内容

#### 2.1.1 沙箱文件浏览器设计（第 6 章）

**技术方案**：
- **目录树**：使用自定义模型（`QAbstractItemModel` 子类），因为 `QFileSystemModel` 无法直接映射虚拟路径（`{AppDir}\VFS\C\Windows\...` → 显示为 `C:\Windows\...`）
- **文件列表**：使用 `QTableView` + 自定义模型（`QAbstractTableModel` 子类），支持文件属性展示（名称、大小、类型、修改时间）
- **搜索功能**：实时搜索（debounce 300ms），支持模糊匹配和正则表达式
- **文件操作**：打开、导出、删除、属性（右键菜单）

**自定义模型设计**：
```cpp
// 沙箱目录树模型（继承 QAbstractItemModel）
class SandboxDirModel : public QAbstractItemModel {
    Q_OBJECT

public:
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

#### 2.1.2 应用管理模块设计（第 5 章）

**应用启动流程**：
1. 检查应用状态（是否已运行）
2. 检查应用 exe 是否存在
3. 设置环境变量（`AI_THINAPP_APP_DIR = appDir`）
4. 注入 Hook DLL（远程线程注入）
5. 创建 `QProcess` 启动应用
6. 监听进程状态（`finished` 信号）

**应用停止流程**：
1. 检查应用状态（是否正在运行）
2. 发送 `WM_CLOSE` 消息到应用主窗口
3. 若超时未关闭，强制终止进程树
4. 发射 `AppStopped` 信号

**应用添加流程**：
1. 打开文件选择器（过滤 `*.exe`）
2. 检查应用是否已被添加
3. 创建应用目录结构（`{AppDir}/VFS/`）
4. 提取应用图标（`ExtractIcon`）
5. 保存到应用列表配置文件（`JSON` 格式）

### 2.2 `docs/SANDBOX-DIR-VISUALIZATION.md` 关键内容

**推荐方案**：方案 A（树状文件浏览器）+ 方案 B（存储空间可视化）组合方案

**方案 A 优点**：
- 用户熟悉（Windows 用户对抗议界面认知度高，学习成本极低）
- 操作直观（符合用户已有的文件管理习惯）
- 信息完整（能展示完整的文件结构细节和文件属性）
- 实现难度低（Electron + Ant Design Tree 组件即可实现）

**方案 A 技术实现**：
- **技术栈**：Qt 6（`QTreeView` + `QFileSystemModel` 或自定义模型）
- **数据源**：`{AppDir}\VFS\` 目录
- **功能列表**：展开/折叠目录树、搜索文件、双击文件打开、右键菜单、显示文件属性
- **性能优化**：懒加载、虚拟滚动、后台索引、增量更新

---

## 3. 开发任务拆解

### 3.1 第一步：读取设计文档（已完成）

1. ✅ 读取 `docs/MVP-LAUNCHPAD-DESIGN.md` 中的"沙箱文件浏览器设计"章节
2. ✅ 读取 `docs/SANDBOX-DIR-VISUALIZATION.md`（方案 A 详细设计）
3. ✅ 提取 Week 8 所有开发任务（沙箱文件浏览器、应用管理模块完善）
4. ✅ 理解技术方案（自定义 `QAbstractItemModel`、虚拟路径映射、`QTreeView` + `QTableView`）

### 3.2 第二步：实现沙箱文件浏览器（Day 1-3）

**目标**：实现树状文件浏览器（方案 A），集成虚拟路径映射

#### 任务列表：

1. **创建 `src/launchpad/ui/sandbox_model.h`**
   - 定义 `SandboxModel` 类（继承自 `QAbstractItemModel`）
   - 重写 `data()`、`flags()`、`headerData()`、`index()`、`parent()`、`rowCount()`、`columnCount()`
   - 添加方法 `setSandboxPath(QString path)`（设置沙箱根目录）
   - 添加方法 `mapToRealPath(QModelIndex index)`（映射虚拟路径到真实路径）
   - 添加方法 `mapFromRealPath(QString real_path)`（映射真实路径到虚拟路径）

2. **创建 `src/launchpad/ui/sandbox_model.cpp`**
   - 实现构造函数（初始化沙箱根目录、文件信息列表）
   - 实现 `data()`（返回文件图标、名称、大小、类型、修改日期）
   - 实现 `index()`（根据行列创建索引）
   - 实现 `parent()`（返回父索引）
   - 实现 `rowCount()`（返回子文件/目录数量）
   - 实现 `columnCount()`（返回列数 = 5）
   - 实现 `setSandboxPath()`（设置沙箱根目录，触发 `layoutChanged()` 信号）
   - 实现 `mapToRealPath()`（将虚拟路径 `{AppDir}\VFS\C\Windows\*` 映射到真实路径 `C:\Windows\*`）
   - 实现 `mapFromRealPath()`（将真实路径映射到虚拟路径）
   - 使用 `QFileSystemWatcher` 监听文件变化（自动刷新模型）

3. **创建 `src/launchpad/ui/sandbox_browser.h`**
   - 定义 `SandboxBrowser` 类（继承自 `QWidget`）
   - 成员变量（`QTreeView *tree_view_`、`QTableView *file_view_`、`SandboxModel *model_`、`QLineEdit *search_box_`）
   - 添加信号（`void fileSelected(QString file_path)`）
   - 添加槽（`void onDirSelected(QModelIndex index)`、`void onSearchTextChanged(QString text)`、`void onFileDoubleClicked(QModelIndex index)`）

4. **创建 `src/launchpad/ui/sandbox_browser.cpp`**
   - 实现构造函数（初始化 UI、连接信号/槽）
   - 实现 `initUI()` 方法（创建目录树、文件列表、搜索框、工具栏）
   - 实现 `setSandboxPath()` 方法（设置沙箱路径，刷新模型）
   - 实现 `onDirSelected()` 槽（目录树选中 → 刷新文件列表）
   - 实现 `onSearchTextChanged()` 槽（实时过滤文件列表，调用 `QSortFilterProxyModel`）
   - 实现 `onFileDoubleClicked()` 槽（双击文件 → 发射 `fileSelected` 信号）
   - 实现 `contextMenuEvent()` 方法（右键菜单：打开/删除/导出/属性）

5. **修改 `src/launchpad/app_manager.h/cpp`**
   - 添加 `openSandboxBrowser()` 方法（打开沙箱文件浏览器对话框）
   - 添加 `exportSandboxFile()` 方法（导出沙箱文件到真实文件系统）
   - 在 `launchApp()` 成功后调用 `openSandboxBrowser()`（可选，用户设置决定）

**验收标准**：
- ✅ 目录树正常显示（虚拟路径 `{AppDir}\VFS\` 作为根节点）
- ✅ 文件列表正常显示（选中目录后，显示该目录下所有文件）
- ✅ 搜索框正常过滤（实时过滤文件列表，支持通配符）
- ✅ 右键菜单正常显示（打开/删除/导出/属性）
- ✅ 双击文件正常打开（调用 `QDesktopServices::openUrl()`）
- ✅ 文件路径映射正确（虚拟路径 ↔ 真实路径，准确率 = 100%）

### 3.3 第三步：完善应用管理模块（Day 4-5）

**目标**：完善应用添加/删除/启动/停止功能，集成沙箱文件浏览器

#### 任务列表：

1. **修改 `src/launchpad/app_manager.h/cpp`**
   - 实现 `addApp()` 方法（浏览 .vapp 包 → 解压到 `{AppDir}\apps\` → 注册到 `apps.json`）
   - 实现 `removeApp()` 方法（从 `apps.json` 删除 → 删除应用目录 `{AppDir}\apps\{app_name}`）
   - 实现 `launchApp()` 方法（调用 `HookEngine::InjectHookDll()` 注入 Hook DLL → `QProcess::start()` 启动应用）
   - 实现 `terminateApp()` 方法（遍历进程树 → `TerminateProcess()` 强制终止）
   - 实现 `openSandboxBrowser()` 方法（创建 `SandboxBrowser` 对话框 → 设置沙箱路径 `{AppDir}\VFS\` → 显示对话框）

2. **修改 `src/launchpad/ui/main_window.cpp`**
   - 在右键菜单中添加"打开沙箱目录" 动作（调用 `AppManager::openSandboxBrowser()`）
   - 在应用详情页中添加"沙箱文件浏览器" 按钮（调用 `AppManager::openSandboxBrowser()`）

3. **创建 `tests/launchpad/test_app_manager.cpp`**
   - 测试用例 1：添加应用（浏览 .vapp 包 → 解压 → 注册到 `apps.json`）
   - 测试用例 2：删除应用（从 `apps.json` 删除 → 删除应用目录）
   - 测试用例 3：启动应用（注入 Hook DLL → 启动应用进程）
   - 测试用例 4：停止应用（遍历进程树 → 强制终止）
   - 测试用例 5：打开沙箱文件浏览器（创建 `SandboxBrowser` 对话框 → 设置沙箱路径 → 显示对话框）

**验收标准**：
- ✅ 添加应用功能正常（.vapp 包解压 + 注册，成功率 = 100%）
- ✅ 删除应用功能正常（注销 + 删除目录，成功率 = 100%）
- ✅ 启动应用功能正常（Hook DLL 注入 + 进程启动，成功率 ≥ 99%）
- ✅ 停止应用功能正常（进程树遍历 + 强制终止，成功率 ≥ 99%）
- ✅ 沙箱文件浏览器集成正常（右键菜单 + 应用详情页按钮）
- ✅ 所有测试用例通过（5 个测试用例）
- ✅ 单元测试覆盖率 ≥ 80%

### 3.4 第四步：输出 Week 8 完成报告

创建 `docs/MVP-WEEK8-COMPLETE-REPORT.md`，包含：
1. 任务完成情况（沙箱文件浏览器、应用管理模块完善）
2. 代码统计（新增代码行数、测试代码行数）
3. 功能测试结果（沙箱文件浏览器 6 项 + 应用管理模块 6 项）
4. 单元测试覆盖率
5. 已知限制（POC 阶段未解决的问题）
6. 下一步行动（Week 9 任务：设置界面 + 任务栏集成）

---

## 4. 技术关键点

### 4.1 自定义 `QAbstractItemModel` 实现

**核心难点**：
- 如何正确实现 `index()` 和 `parent()` 函数（构建树状结构）
- 如何映射虚拟路径到真实路径（字符串替换或正则表达式）
- 如何使用 `QFileSystemWatcher` 监听文件变化（自动刷新模型）

**解决方案**：
- 使用 `QFileInfo` 列表存储文件和目录信息
- 使用 `createIndex()` 创建索引，传入 `QFileInfo*` 作为内部指针
- 使用 `QString::replace()` 或 `QRegularExpression` 进行路径映射
- 连接 `QFileSystemWatcher::directoryChanged()` 信号到模型的 `refresh()` 槽

### 4.2 虚拟路径映射

**映射规则**：
- 虚拟路径 → 真实路径：`{AppDir}\VFS\C\Windows\*` → `C:\Windows\*`
- 真实路径 → 虚拟路径：`C:\Windows\*` → `{AppDir}\VFS\C\Windows\*`

**实现方式**：
```cpp
// 虚拟路径 → 真实路径
QString SandboxModel::mapToRealPath(const QModelIndex& index) const {
    QString virtualPath = fileInfo(index).absoluteFilePath();
    QString realPath = virtualPath;
    realPath.replace(appDir_ + "/VFS/", "");  // 移除虚拟根目录前缀
    realPath.replace("/", "\\");               // 转换路径分隔符
    return realPath;
}

// 真实路径 → 虚拟路径
QString SandboxModel::mapFromRealPath(const QString& realPath) const {
    QString virtualPath = appDir_ + "/VFS/";
    virtualPath += realPath;
    virtualPath.replace("\\", "/");           // 转换路径分隔符
    return virtualPath;
}
```

### 4.3 Hook DLL 注入

**注入流程**：
1. 设置环境变量（`AI_THINAPP_APP_DIR = appDir`）
2. 在目标进程中分配内存（写入 DLL 路径）
3. 创建远程线程（调用 `LoadLibraryW` 加载 DLL）
4. 等待远程线程结束
5. 清理（释放内存、关闭句柄）

**关键 Windows API**：
- `VirtualAllocEx()` - 在目标进程中分配内存
- `WriteProcessMemory()` - 写入 DLL 路径到目标进程内存
- `CreateRemoteThread()` - 创建远程线程（调用 `LoadLibraryW`）
- `WaitForSingleObject()` - 等待远程线程结束
- `VirtualFreeEx()` - 释放目标进程内存

---

## 5. 项目文件结构

### 5.1 现有文件结构

```
D:\anmw_work_proj\ai-thinApp-portable-lauchpad-platform\
│
├── src\launchpad\
│   ├── app_manager.h/cpp        # 应用管理器（POC 阶段实现）
│   ├── ui\
│   │   ├── main_window.h/cpp    # 主窗口（POC 阶段实现）
│   │   └── ...                  # 其他 UI 组件
│   └── ...
│
├── tests\launchpad\
│   └── ...                      # 测试代码
│
└── docs\
    ├── MVP-LAUNCHPAD-DESIGN.md  # Launchpad UI 设计文档
    ├── SANDBOX-DIR-VISUALIZATION.md  # 沙箱目录可视化方案
    └── ...
```

### 5.2 Week 8 新增/修改文件

| 文件 | 路径 | 操作 |
|------|------|------|
| 沙箱模型头文件 | `src/launchpad/ui/sandbox_model.h` | 新建 |
| 沙箱模型实现 | `src/launchpad/ui/sandbox_model.cpp` | 新建 |
| 沙箱浏览器头文件 | `src/launchpad/ui/sandbox_browser.h` | 新建 |
| 沙箱浏览器实现 | `src/launchpad/ui/sandbox_browser.cpp` | 新建 |
| 应用管理器（修改） | `src/launchpad/app_manager.h/cpp` | 修改 |
| 主窗口（修改） | `src/launchpad/ui/main_window.cpp` | 修改 |
| 应用管理器测试 | `tests/launchpad/test_app_manager.cpp` | 新建 |
| Week 8 完成报告 | `docs/MVP-WEEK8-COMPLETE-REPORT.md` | 新建 |

---

## 6. 风险与问题

### 6.1 技术风险

| 风险 ID | 风险描述 | 影响 | 概率 | 缓释措施 |
|---------|---------|------|------|----------|
| RISK-01 | `QAbstractItemModel` 实现复杂，可能导致模型数据不一致 | 高 | 中 | 参考 Qt 官方示例（`QFileSystemModel`），编写单元测试验证模型正确性 |
| RISK-02 | 虚拟路径映射错误，导致文件操作失败 | 高 | 中 | 编写路径映射单元测试，覆盖各种边界情况 |
| RISK-03 | Hook DLL 注入失败（杀软拦截） | 高 | 高 | 代码签名 + 杀软白名单申请；POC 阶段提示用户关闭杀软 |

### 6.2 进度风险

| 风险 ID | 风险描述 | 影响 | 概率 | 缓释措施 |
|---------|---------|------|------|----------|
| RISK-04 | 沙箱文件浏览器实现复杂，可能超出预估工时 | 中 | 中 | 优先保证核心功能（目录树 + 文件列表），高级功能（搜索、右键菜单）可延后 |
| RISK-05 | 应用管理模块完善涉及进程管理，调试复杂 | 中 | 高 | 先实现基础功能（启动/停止），高级功能（进程树遍历）可延后 |

---

## 7. 确认声明

本人已仔细阅读并理解 `docs/MVP-LAUNCHPAD-DESIGN.md` 和 `docs/SANDBOX-DIR-VISUALIZATION.md` 中的设计内容，明确 Week 8 的开发任务和验收标准。

**下一步行动**：开始实现沙箱文件浏览器（创建 `sandbox_model.h/cpp` 和 `sandbox_browser.h/cpp`）。

---

**签署**：
- 姓名：Qt/C++ 软件开发工程师
- 时间：2026-05-23 18:39

# MVP Week 6 任务理解确认

> 本文档确认对 AI ThinApp Portable Launchpad Platform 项目 MVP Week 6 开发任务的理解。
> 日期：2026-05-23 | 负责人：Qt/C++ 应用管理开发工程师

---

## 1. 项目背景

### 1.1 现有技术栈

| 技术 | 选型 | 状态 |
|------|------|------|
| GUI 框架 | Qt 6 | ✅ POC 验证通过 |
| 应用管理器 | `AppManager` 类 | ✅ 基础功能已实现（Week 7-9） |
| 主窗口 | `MainWindow` 类 | ✅ 基础 UI 已实现 |
| 配置文件 | `apps.json` (JSON 格式) | ✅ 已实现加载和保存 |

### 1.2 当前实现状态（Week 7-9 已完成）

根据 `src/launchpad/app_manager.h/cpp` 代码分析：

**已实现功能**：
- ✅ 应用列表加载（从 `apps.json` 配置文件）
- ✅ 应用启动/停止（使用 `QProcess`）
- ✅ 添加应用（浏览 .vapp 包，解压，注册到 `apps.json`）
- ✅ 删除应用（从 `apps.json` 删除，删除应用目录）
- ✅ 沙箱文件浏览器（打开、导出文件）
- ✅ Balloon 通知（应用启动/停止/崩溃）
- ✅ 搜索应用（缓存版本 + 非缓存版本）

**未实现功能**（Week 6 需要完成）：
- ❌ 批量操作（批量启动、批量停止、批量删除、批量导出）
- ❌ 导入/导出配置（JSON 格式，配置冲突处理）
- ❌ 应用信息编辑（名称、图标、启动参数、沙箱模式）
- ❌ Hook DLL 注入完整实现（当前仅设置环境变量）

---

## 2. Week 6 任务拆解

### 2.1 任务 1：批量操作（Day 1-2）

#### 2.1.1 目标

实现批量启动、批量停止、批量删除、批量导出功能。

#### 2.1.2 技术方案

| 功能 | 技术方案 | 实现要点 |
|------|----------|----------|
| 批量启动 | 遍历 `QListView` 选中项，调用 `AppManager::LaunchApp()` | 使用 `QItemSelectionModel` 获取选中项 |
| 批量停止 | 遍历 `QListView` 选中项，调用 `AppManager::StopApp()` | 检查应用是否正在运行 |
| 批量删除 | 遍历 `QListView` 选中项，调用 `AppManager::RemoveApp()` | 显示确认对话框（`QMessageBox::question()`） |
| 批量导出 | 遍历 `QListView` 选中项，调用 `AppManager::ExportApp()` | 选择导出目录（`QFileDialog::getExistingDirectory()`） |

#### 2.1.3 代码修改点

1. **`src/launchpad/ui/main_window.cpp`**：
   - 在右键菜单中添加"批量操作"子菜单
   - 实现 `onBatchStart()`、`onBatchStop()`、`onBatchDelete()`、`onBatchExport()` 槽

2. **`src/launchpad/app_manager.h/cpp`**：
   - 添加 `ExportApp(const std::string& app_name, const std::string& export_dir)` 方法
   - 实现 `ExportApp()` 方法（调用 `VAppPackage::Compress()`，生成 .vapp 包）

3. **`tests/launchpad/test_batch_operations.cpp`**：
   - 编写 4 个测试用例（批量启动、批量停止、批量删除、批量导出）

#### 2.1.4 验收标准

- ✅ 批量启动功能正常（选中 5 个应用，全部启动成功）
- ✅ 批量停止功能正常（选中 3 个运行中的应用，全部停止成功）
- ✅ 批量删除功能正常（选中 2 个应用，确认后全部删除成功）
- ✅ 批量导出功能正常（选中 4 个应用，导出为 .vapp 包成功）
- ✅ 所有测试用例通过（4 个测试用例）
- ✅ 单元测试覆盖率 ≥ 80%

---

### 2.2 任务 2：导入/导出配置（Day 3）

#### 2.2.1 目标

实现应用配置导入/导出功能（JSON 格式）。

#### 2.2.2 技术方案

| 功能 | 技术方案 | 实现要点 |
|------|----------|----------|
| 导入配置 | 读取 JSON 文件，解析应用列表，添加到 `apps_` | 使用 `QJsonDocument` 解析 JSON |
| 导出配置 | 遍历 `apps_`，生成 JSON 文件 | 使用 `QJsonDocument` 生成 JSON |
| 配置冲突处理 | 导入时检查应用目录是否已存在，提示用户选择覆盖或跳过 | 使用 `QMessageBox` 显示冲突对话框 |

#### 2.2.3 代码修改点

1. **`src/launchpad/app_manager.h/cpp`**：
   - 添加 `ImportConfig(const std::string& config_file)` 方法
   - 添加 `ExportConfig(const std::string& config_file)` 方法
   - 实现 `ImportConfig()` 方法（读取 JSON 文件，解析应用列表，添加到 `apps_`）
   - 实现 `ExportConfig()` 方法（遍历 `apps_`，生成 JSON 文件）

2. **`src/launchpad/ui/main_window.cpp`**：
   - 在"文件"菜单中添加"导入配置"和"导出配置"动作
   - 实现 `onImportConfig()` 槽（打开文件对话框，选择 JSON 文件，调用 `AppManager::ImportConfig()`）
   - 实现 `onExportConfig()` 槽（打开文件对话框，选择保存路径，调用 `AppManager::ExportConfig()`）

3. **`tests/launchpad/test_config_io.cpp`**：
   - 编写 4 个测试用例（导入配置、导出配置、配置格式正确性、配置冲突处理）

#### 2.2.4 验收标准

- ✅ 导入配置功能正常（从 JSON 文件导入 5 个应用，全部成功）
- ✅ 导出配置功能正常（导出 3 个应用配置到 JSON 文件，全部成功）
- ✅ 配置格式正确性（导出 JSON 文件格式符合 `apps.json` 规范）
- ✅ 配置冲突处理正确（冲突时提示用户选择，验证用户选择生效）
- ✅ 所有测试用例通过（4 个测试用例）
- ✅ 单元测试覆盖率 ≥ 80%

---

### 2.3 任务 3：应用信息编辑（Day 4-5）

#### 2.3.1 目标

实现应用信息编辑功能（名称、图标、启动参数、工作目录）。

#### 2.3.2 技术方案

| 功能 | 技术方案 | 实现要点 |
|------|----------|----------|
| 应用信息编辑对话框 | 创建 `AppEditor` 类（继承自 `QDialog`） | 使用 `QFormLayout` 布局 |
| 名称编辑 | `QLineEdit` | 从 `AppInfo` 结构体加载/保存 |
| 图标编辑 | `QPushButton` + `QFileDialog` | 打开文件对话框选择图标文件，预览图标 |
| 启动参数编辑 | `QLineEdit` | 从 `AppInfo` 结构体加载/保存 |
| 工作目录编辑 | `QLineEdit` + `QPushButton` + `QFileDialog` | 打开目录对话框选择工作目录 |
| 沙箱模式编辑 | `QComboBox` | 选项："沙箱模式"、"真实模式" |

#### 2.3.3 代码修改点

1. **创建 `src/launchpad/ui/app_editor.h`**：
   - 定义 `AppEditor` 类（继承自 `QDialog`）
   - 成员变量（`QLineEdit *name_edit_`、`QPushButton *icon_button_`、`QLineEdit *args_edit_`、`QComboBox *sandbox_mode_combo_`）
   - 添加信号（无）
   - 添加槽（`void onIconButtonClicked()`、`void onOkClicked()`、`void onCancelClicked()`）

2. **创建 `src/launchpad/ui/app_editor.cpp`**：
   - 实现构造函数（初始化 UI，加载应用信息）
   - 实现 `initUI()` 方法（创建表单布局，添加名称、图标、启动参数、工作目录、沙箱模式字段）
   - 实现 `loadAppInfo()` 方法（从 `AppInfo` 结构体加载应用到表单）
   - 实现 `saveAppInfo()` 方法（从表单保存到 `AppInfo` 结构体）
   - 实现 `onIconButtonClicked()` 槽（打开文件对话框，选择图标文件，预览图标）
   - 实现 `onOkClicked()` 槽（保存应用信息，关闭对话框）
   - 实现 `onCancelClicked()` 槽（放弃修改，关闭对话框）

3. **修改 `src/launchpad/ui/main_window.cpp`**：
   - 在右键菜单中添加"编辑应用"动作（连接 `AppEditor::exec()`）
   - 实现 `onEditApp()` 槽（创建 `AppEditor` 对话框，传入选中应用信息，执行对话框）

4. **`tests/launchpad/test_app_editor.cpp`**：
   - 编写 4 个测试用例（编辑应用名称、编辑应用图标、编辑启动参数、编辑沙箱模式）

#### 2.3.4 验收标准

- ✅ 应用信息编辑功能正常（名称、图标、启动参数、沙箱模式全部可编辑）
- ✅ 图标选择功能正常（打开文件对话框，选择图标文件，预览正确）
- ✅ 启动参数传递正确（编辑启动参数后，启动应用验证参数已传递）
- ✅ 沙箱模式生效（编辑沙箱模式后，启动应用验证沙箱模式已生效）
- ✅ 所有测试用例通过（4 个测试用例）
- ✅ 单元测试覆盖率 ≥ 80%

---

## 3. 技术难点分析

### 3.1 难点 1：批量操作的多选实现

**问题描述**：
- 当前 `MainWindow` 使用 `QListWidget` 显示应用列表，需要支持多选
- 多选模式下，需要获取所有选中项的应用目录路径

**解决方案**：
- 设置 `QListWidget::setSelectionMode(QAbstractItemView::MultiSelection)`
- 使用 `QListWidget::selectedItems()` 获取所有选中项
- 从 `QListWidgetItem::data(Qt::UserRole)` 获取应用目录路径

**代码示例**：
```cpp
// 设置为多选模式
ui->appListWidget->setSelectionMode(QAbstractItemView::MultiSelection);

// 获取所有选中项
QList<QListWidgetItem*> selectedItems = ui->appListWidget->selectedItems();
for (QListWidgetItem* item : selectedItems) {
    QString appDir = item->data(Qt::UserRole).toString();
    // 执行批量操作...
}
```

### 3.2 难点 2：配置冲突处理

**问题描述**：
- 导入配置时，如果应用目录已存在，需要提示用户选择覆盖或跳过
- 需要提供友好的 UI 界面，显示冲突的应用信息

**解决方案**：
- 遍历导入的应用列表，检查 `appDir` 是否已存在
- 如果存在，显示冲突对话框（`QMessageBox` 或自定义对话框）
- 用户可以选择"覆盖"、"跳过"或"取消"

**代码示例**：
```cpp
// 检查配置冲突
QJsonArray importArray = ...;
for (const QJsonValue& value : importArray) {
    QString importAppDir = value.toObject()["appDir"].toString();
    
    // 检查是否已存在
    if (appManager->GetAppInfo(importAppDir).appDir != "") {
        // 显示冲突对话框
        int ret = QMessageBox::question(this, "配置冲突", 
                                        QString("应用 %1 已存在，是否覆盖？").arg(importAppDir),
                                        QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);
        if (ret == QMessageBox::Yes) {
            // 覆盖
        } else if (ret == QMessageBox::No) {
            // 跳过
        } else {
            // 取消
            break;
        }
    }
}
```

### 3.3 难点 3：应用信息编辑的 UI 设计

**问题描述**：
- 需要设计一个用户友好的应用信息编辑对话框
- 需要支持图标预览、文件选择等功能

**解决方案**：
- 使用 `QFormLayout` 布局，添加标签和输入框
- 图标选择使用 `QPushButton` + `QLabel`（预览）
- 启动参数、工作目录使用 `QLineEdit` + `QPushButton`（`QFileDialog`）

**代码示例**：
```cpp
// 图标选择按钮点击事件
void AppEditor::onIconButtonClicked() {
    QString iconPath = QFileDialog::getOpenFileName(this, "选择图标文件", "", "图标文件 (*.ico *.png *.jpg)");
    if (!iconPath.isEmpty()) {
        // 预览图标
        ui->iconPreviewLabel->setPixmap(QPixmap(iconPath).scaled(64, 64));
        // 保存图标路径
        iconPath_ = iconPath;
    }
}
```

---

## 4. 测试计划

### 4.1 单元测试

| 测试文件 | 测试用例数 | 测试内容 |
|----------|------------|----------|
| `tests/launchpad/test_batch_operations.cpp` | 4 | 批量启动、批量停止、批量删除、批量导出 |
| `tests/launchpad/test_config_io.cpp` | 4 | 导入配置、导出配置、配置格式正确性、配置冲突处理 |
| `tests/launchpad/test_app_editor.cpp` | 4 | 编辑应用名称、编辑应用图标、编辑启动参数、编辑沙箱模式 |

### 4.2 功能测试

| 功能 | 测试步骤 | 预期结果 |
|------|----------|----------|
| 批量启动 | 1. 选中 5 个应用<br>2. 右键菜单选择"批量启动" | 5 个应用全部启动成功 |
| 批量停止 | 1. 选中 3 个运行中的应用<br>2. 右键菜单选择"批量停止" | 3 个应用全部停止成功 |
| 批量删除 | 1. 选中 2 个应用<br>2. 右键菜单选择"批量删除"<br>3. 确认删除 | 2 个应用全部删除成功 |
| 批量导出 | 1. 选中 4 个应用<br>2. 右键菜单选择"批量导出"<br>3. 选择导出目录 | 4 个 .vapp 包已生成 |
| 导入配置 | 1. 点击"文件"菜单 -> "导入配置"<br>2. 选择包含 5 个应用的 JSON 文件 | 5 个应用全部导入成功 |
| 导出配置 | 1. 添加 3 个应用到应用列表<br>2. 点击"文件"菜单 -> "导出配置"<br>3. 选择保存路径 | JSON 文件已生成，包含 3 个应用配置 |
| 编辑应用名称 | 1. 右键点击应用，选择"编辑应用"<br>2. 修改名称为"Test App"<br>3. 点击"确定" | 应用列表中的名称已更新为"Test App" |
| 编辑应用图标 | 1. 右键点击应用，选择"编辑应用"<br>2. 点击图标选择按钮，选择新图标文件<br>3. 点击"确定" | 应用列表中的图标已更新 |
| 编辑启动参数 | 1. 右键点击应用，选择"编辑应用"<br>2. 修改启动参数为"--safe-mode"<br>3. 点击"确定"<br>4. 启动应用 | 启动应用时参数已传递 |
| 编辑沙箱模式 | 1. 右键点击应用，选择"编辑应用"<br>2. 修改沙箱模式为"完全隔离"<br>3. 点击"确定"<br>4. 启动应用 | 启动应用时沙箱模式已生效 |

---

## 5. 风险与依赖

### 5.1 风险

| 风险 ID | 风险描述 | 影响 | 概率 | 等级 | 缓释措施 | 负责人 |
|---------|----------|------|------|----------|----------|--------|
| R-W6-01 | 批量导出 .vapp 包失败（压缩功能未实现） | 高 | 中 | 高 | 优先实现 `VAppPackage::Compress()` 方法；若实现困难，使用 `QUazip` 或 `libzip` 库 | Dev |
| R-W6-02 | 配置冲突处理 UI 设计复杂 | 中 | 中 | 中 | 使用 `QMessageBox` 简单实现；若需要更复杂 UI，使用自定义对话框 | UX + Dev |
| R-W6-03 | 应用信息编辑对话框布局不合理 | 低 | 中 | 低 | 参考 Qt Designer 设计的 UI；使用 `QFormLayout` 自动布局 | UX + Dev |

### 5.2 依赖

| 依赖项 | 来源 | 说明 |
|--------|------|------|
| `VAppPackage::Compress()` | Week 5 任务 | 批量导出功能依赖 .vapp 包压缩功能 |
| `AppInfo` 结构体 | `app_manager.h` | 应用信息编辑功能依赖 `AppInfo` 结构体字段完整性 |
| `QFileDialog` | Qt 6 框架 | 文件选择功能依赖 `QFileDialog` |
| `QJsonDocument` | Qt 6 框架 | JSON 配置文件依赖 `QJsonDocument` |

---

## 6. 交付物清单

| 文件 | 路径 | 状态 |
|------|------|------|
| 任务理解确认 | `docs/MVP-WEEK6-TASK-CONFIRM.md` | ✅ 已完成 |
| 主窗口（修改） | `src/launchpad/ui/main_window.h/cpp` | ⏳ 待完成 |
| 应用管理器（修改） | `src/launchpad/app_manager.h/cpp` | ⏳ 待完成 |
| 应用编辑器头文件 | `src/launchpad/ui/app_editor.h` | ⏳ 待完成 |
| 应用编辑器实现 | `src/launchpad/ui/app_editor.cpp` | ⏳ 待完成 |
| 批量操作测试 | `tests/launchpad/test_batch_operations.cpp` | ⏳ 待完成 |
| 配置导入/导出测试 | `tests/launchpad/test_config_io.cpp` | ⏳ 待完成 |
| 应用信息编辑测试 | `tests/launchpad/test_app_editor.cpp` | ⏳ 待完成 |
| Week 6 完成报告 | `docs/MVP-WEEK6-COMPLETE-REPORT.md` | ⏳ 待完成 |

---

## 7. 时间规划

| 任务 | 时间 | 负责人 | 产出 |
|------|------|--------|------|
| 批量操作 | Day 1-2 | Dev | `main_window.cpp` (修改), `app_manager.cpp` (修改), `test_batch_operations.cpp` |
| 导入/导出配置 | Day 3 | Dev | `app_manager.cpp` (修改), `main_window.cpp` (修改), `test_config_io.cpp` |
| 应用信息编辑 | Day 4-5 | Dev | `app_editor.h/cpp`, `main_window.cpp` (修改), `test_app_editor.cpp` |
| 代码审查 + Bug 修复 | Day 5 | Dev + QA | 代码审查报告、Bug 修复报告 |
| 输出 Week 6 完成报告 | Day 5 | Dev + PM | `MVP-WEEK6-COMPLETE-REPORT.md` |

---

## 8. 确认意见

本人已充分理解 MVP Week 6 的开发任务，包括批量操作、导入/导出配置、应用信息编辑三大功能模块。

**技术路线确认**：
- ✅ 使用 `QListWidget` 多选实现批量操作
- ✅ 使用 `QJsonDocument` 实现配置导入/导出
- ✅ 使用 `QFormLayout` 实现应用信息编辑对话框
- ✅ 使用 `QMessageBox` 实现配置冲突处理

**验收标准确认**：
- ✅ 所有功能测试通过（12 个测试用例）
- ✅ 单元测试覆盖率 ≥ 80%
- ✅ 所有文件使用 UTF-8 编码（无 BOM）

**风险确认**：
- ⚠️ 批量导出 .vapp 包功能依赖 `VAppPackage::Compress()` 方法实现
- ⚠️ 配置冲突处理需要友好的 UI 界面

**签字**：
- 负责人：Qt/C++ 应用管理开发工程师
- 日期：2026-05-23

---

## 附录 A：参考资料

1. **设计文档**：`docs/MVP-LAUNCHPAD-DESIGN.md`
2. **当前代码**：`src/launchpad/app_manager.h/cpp`
3. **Qt 6 官方文档**：https://doc.qt.io/qt-6/
4. **JSON 处理**：https://doc.qt.io/qt-6/json.html
5. **QFileDialog 文档**：https://doc.qt.io/qt-6/qfiledialog.html
6. **QFormLayout 文档**：https://doc.qt.io/qt-6/qformlayout.html

---

**文档版本**：v1.0 | **日期**：2026-05-23 | **作者**：Qt/C++ 应用管理开发工程师

/**
 * @file settings_dialog.cpp
 * @brief 设置对话框实现文件
 * @author Qt/C++ 软件开发工程师
 * @date 2026-05-23
 */

#include "settings_dialog.h"
#include "ui_settings_dialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <QPushButton>
#include <QFileDialog>
#include <QMessageBox>
#include <QContextMenuEvent>
#include <QMenu>
#include <QDesktopServices>
#include <QUrl>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QFile>
#include <QTextStream>
#include <Windows.h>

// 注册表路径：开机自启动
const QString REG_RUN_KEY = "HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Run";
const QString APP_NAME = "AIThinAppLaunchpad";

SettingsDialog::SettingsDialog(QWidget* parent)
    : QDialog(parent)
    , tab_widget_(nullptr)
    , auto_start_checkbox_(nullptr)
    , min_to_tray_checkbox_(nullptr)
    , exit_confirm_checkbox_(nullptr)
    , sandbox_path_edit_(nullptr)
    , browse_button_(nullptr)
    , hook_enable_checkbox_(nullptr)
    , log_error_checkbox_(nullptr)
    , log_warning_checkbox_(nullptr)
    , log_info_checkbox_(nullptr)
    , detect_interval_spinbox_(nullptr)
    , startup_list_widget_(nullptr)
    , add_button_(nullptr)
    , delete_button_(nullptr)
    , move_up_button_(nullptr)
    , move_down_button_(nullptr)
    , version_label_(nullptr)
    , github_label_(nullptr)
    , license_label_(nullptr)
    , ok_button_(nullptr)
    , cancel_button_(nullptr)
    , settings_(nullptr)
{
    // 初始化 QSettings（使用注册表存储）
    settings_ = new QSettings("AI ThinApp", "Launchpad", this);
    
    // 初始化 UI
    initUI();
    
    // 加载设置
    loadSettings();
}

SettingsDialog::~SettingsDialog()
{
    // QSettings 会在父对象销毁时自动释放
}

void SettingsDialog::initUI()
{
    // 设置窗口属性
    setWindowTitle(tr("设置"));
    setMinimumSize(500, 400);
    
    // 创建主布局
    QVBoxLayout* main_layout = new QVBoxLayout(this);
    
    // 创建标签页控件
    tab_widget_ = new QTabWidget(this);
    tab_widget_->addTab(createGeneralTab(), tr("常规设置"));
    tab_widget_->addTab(createHookTab(), tr("Hook 设置"));
    tab_widget_->addTab(createStartupTab(), tr("启动项管理"));
    tab_widget_->addTab(createAboutTab(), tr("关于"));
    
    main_layout->addWidget(tab_widget_);
    
    // 创建按钮布局
    QHBoxLayout* button_layout = new QHBoxLayout();
    button_layout->addStretch();
    
    ok_button_ = new QPushButton(tr("确定"), this);
    cancel_button_ = new QPushButton(tr("取消"), this);
    
    button_layout->addWidget(ok_button_);
    button_layout->addWidget(cancel_button_);
    
    main_layout->addLayout(button_layout);
    
    // 连接信号槽
    connect(ok_button_, &QPushButton::clicked, this, &SettingsDialog::onOkClicked);
    connect(cancel_button_, &QPushButton::clicked, this, &SettingsDialog::onCancelClicked);
}

QWidget* SettingsDialog::createGeneralTab()
{
    QWidget* tab = new QWidget(this);
    QVBoxLayout* layout = new QVBoxLayout(tab);
    
    // 常规设置组
    QGroupBox* general_group = new QGroupBox(tr("常规设置"), tab);
    QVBoxLayout* general_layout = new QVBoxLayout(general_group);
    
    auto_start_checkbox_ = new QCheckBox(tr("开机自启动"), general_group);
    min_to_tray_checkbox_ = new QCheckBox(tr("最小化到托盘"), general_group);
    exit_confirm_checkbox_ = new QCheckBox(tr("退出确认"), general_group);
    
    general_layout->addWidget(auto_start_checkbox_);
    general_layout->addWidget(min_to_tray_checkbox_);
    general_layout->addWidget(exit_confirm_checkbox_);
    
    connect(auto_start_checkbox_, &QCheckBox::toggled,
            this, &SettingsDialog::onAutoStartChanged);
    
    // 沙箱路径设置组
    QGroupBox* sandbox_group = new QGroupBox(tr("沙箱设置"), tab);
    QHBoxLayout* sandbox_layout = new QHBoxLayout(sandbox_group);
    
    QLabel* path_label = new QLabel(tr("默认沙箱路径："), sandbox_group);
    sandbox_path_edit_ = new QLineEdit(sandbox_group);
    browse_button_ = new QPushButton(tr("浏览"), sandbox_group);
    
    sandbox_layout->addWidget(path_label);
    sandbox_layout->addWidget(sandbox_path_edit_);
    sandbox_layout->addWidget(browse_button_);
    
    connect(browse_button_, &QPushButton::clicked, [this]() {
        QString dir = QFileDialog::getExistingDirectory(this, tr("选择沙箱路径"),
                                                        sandbox_path_edit_->text());
        if (!dir.isEmpty()) {
            sandbox_path_edit_->setText(dir);
        }
    });
    
    // 添加到主布局
    layout->addWidget(general_group);
    layout->addWidget(sandbox_group);
    layout->addStretch();
    
    return tab;
}

QWidget* SettingsDialog::createHookTab()
{
    QWidget* tab = new QWidget(this);
    QVBoxLayout* layout = new QVBoxLayout(tab);
    
    // Hook 开关设置组
    QGroupBox* hook_group = new QGroupBox(tr("Hook 设置"), tab);
    QVBoxLayout* hook_layout = new QVBoxLayout(hook_group);
    
    hook_enable_checkbox_ = new QCheckBox(tr("启用 Hook 拦截（文件/注册表）"), hook_group);
    
    hook_layout->addWidget(hook_enable_checkbox_);
    
    // 日志级别设置组（高级选项）
    QGroupBox* log_group = new QGroupBox(tr("Hook 日志级别（高级选项）"), tab);
    QVBoxLayout* log_layout = new QVBoxLayout(log_group);
    
    log_error_checkbox_ = new QCheckBox(tr("错误"), log_group);
    log_warning_checkbox_ = new QCheckBox(tr("警告"), log_group);
    log_info_checkbox_ = new QCheckBox(tr("信息"), log_group);
    
    log_layout->addWidget(log_error_checkbox_);
    log_layout->addWidget(log_warning_checkbox_);
    log_layout->addWidget(log_info_checkbox_);
    
    // 覆盖检测频率设置组
    QGroupBox* detect_group = new QGroupBox(tr("覆盖检测"), tab);
    QHBoxLayout* detect_layout = new QHBoxLayout(detect_group);
    
    QLabel* detect_label = new QLabel(tr("覆盖检测频率（秒）："), detect_group);
    detect_interval_spinbox_ = new QSpinBox(detect_group);
    detect_interval_spinbox_->setRange(1, 60);
    detect_interval_spinbox_->setValue(5);
    
    detect_layout->addWidget(detect_label);
    detect_layout->addWidget(detect_interval_spinbox_);
    detect_layout->addStretch();
    
    connect(detect_interval_spinbox_, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &SettingsDialog::onCacheSizeChanged);
    
    // 添加到主布局
    layout->addWidget(hook_group);
    layout->addWidget(log_group);
    layout->addWidget(detect_group);
    layout->addStretch();
    
    return tab;
}

QWidget* SettingsDialog::createStartupTab()
{
    QWidget* tab = new QWidget(this);
    QVBoxLayout* layout = new QVBoxLayout(tab);
    
    // 启动项列表
    startup_list_widget_ = new QListWidget(tab);
    
    // 按钮组
    QHBoxLayout* button_layout = new QHBoxLayout();
    
    add_button_ = new QPushButton(tr("添加"), tab);
    delete_button_ = new QPushButton(tr("删除"), tab);
    move_up_button_ = new QPushButton(tr("上移"), tab);
    move_down_button_ = new QPushButton(tr("下移"), tab);
    
    button_layout->addWidget(add_button_);
    button_layout->addWidget(delete_button_);
    button_layout->addWidget(move_up_button_);
    button_layout->addWidget(move_down_button_);
    button_layout->addStretch();
    
    layout->addWidget(startup_list_widget_);
    layout->addLayout(button_layout);
    
    // 连接信号槽
    connect(add_button_, &QPushButton::clicked, [this]() {
        QString file = QFileDialog::getOpenFileName(this, tr("选择应用"),
                                                    "", tr("可执行文件 (*.exe)"));
        if (!file.isEmpty()) {
            startup_list_widget_->addItem(file);
        }
    });
    
    connect(delete_button_, &QPushButton::clicked, [this]() {
        int row = startup_list_widget_->currentRow();
        if (row >= 0) {
            delete startup_list_widget_->takeItem(row);
        }
    });
    
    connect(move_up_button_, &QPushButton::clicked, [this]() {
        int row = startup_list_widget_->currentRow();
        if (row > 0) {
            QListWidgetItem* item = startup_list_widget_->takeItem(row);
            startup_list_widget_->insertItem(row - 1, item);
            startup_list_widget_->setCurrentRow(row - 1);
        }
    });
    
    connect(move_down_button_, &QPushButton::clicked, [this]() {
        int row = startup_list_widget_->currentRow();
        if (row >= 0 && row < startup_list_widget_->count() - 1) {
            QListWidgetItem* item = startup_list_widget_->takeItem(row);
            startup_list_widget_->insertItem(row + 1, item);
            startup_list_widget_->setCurrentRow(row + 1);
        }
    });
    
    return tab;
}

QWidget* SettingsDialog::createAboutTab()
{
    QWidget* tab = new QWidget(this);
    QVBoxLayout* layout = new QVBoxLayout(tab);
    
    // 版本号
    version_label_ = new QLabel(tr("版本：v0.1.0 (MVP)"), tab);
    
    // GitHub 链接
    github_label_ = new QLabel(tr("GitHub：<a href=\"https://github.com/ai-thinapp/...\">https://github.com/ai-thinapp/...</a>"), tab);
    github_label_->setOpenExternalLinks(true);
    
    // 许可证
    license_label_ = new QLabel(tr("许可证：GPL v3"), tab);
    
    // 作者
    QLabel* author_label = new QLabel(tr("作者：AI ThinApp Team"), tab);
    
    // 感谢
    QLabel* thanks_label = new QLabel(tr("感谢：MinHook、Qt 6、zlib 等开源项目"), tab);
    
    layout->addWidget(version_label_);
    layout->addWidget(github_label_);
    layout->addWidget(license_label_);
    layout->addWidget(author_label);
    layout->addWidget(thanks_label);
    layout->addStretch();
    
    return tab;
}

void SettingsDialog::loadSettings()
{
    // 加载常规设置
    auto_start_checkbox_->setChecked(settings_->value("auto_start", false).toBool());
    min_to_tray_checkbox_->setChecked(settings_->value("min_to_tray", true).toBool());
    exit_confirm_checkbox_->setChecked(settings_->value("exit_confirm", true).toBool());
    sandbox_path_edit_->setText(settings_->value("sandbox_path", "D:/ThinAppApps").toString());
    
    // 加载 Hook 设置
    hook_enable_checkbox_->setChecked(settings_->value("hook_enable", true).toBool());
    
    int log_level = settings_->value("log_level", 3).toInt();  // 默认：错误 + 警告
    log_error_checkbox_->setChecked(log_level & 1);
    log_warning_checkbox_->setChecked(log_level & 2);
    log_info_checkbox_->setChecked(log_level & 4);
    
    detect_interval_spinbox_->setValue(settings_->value("detect_interval", 5).toInt());
    
    // 加载启动项
    int size = settings_->beginReadArray("startup");
    for (int i = 0; i < size; ++i) {
        settings_->setArrayIndex(i);
        startup_list_widget_->addItem(settings_->value("path").toString());
    }
    settings_->endArray();
}

void SettingsDialog::saveSettings()
{
    // 保存常规设置
    settings_->setValue("auto_start", auto_start_checkbox_->isChecked());
    settings_->setValue("min_to_tray", min_to_tray_checkbox_->isChecked());
    settings_->setValue("exit_confirm", exit_confirm_checkbox_->isChecked());
    settings_->setValue("sandbox_path", sandbox_path_edit_->text());
    
    // 保存 Hook 设置
    settings_->setValue("hook_enable", hook_enable_checkbox_->isChecked());
    
    int log_level = 0;
    if (log_error_checkbox_->isChecked()) log_level |= 1;
    if (log_warning_checkbox_->isChecked()) log_level |= 2;
    if (log_info_checkbox_->isChecked()) log_level |= 4;
    settings_->setValue("log_level", log_level);
    
    settings_->setValue("detect_interval", detect_interval_spinbox_->value());
    
    // 保存启动项到注册表
    onAutoStartChanged(auto_start_checkbox_->isChecked());
    
    // 保存启动项列表
    settings_->beginWriteArray("startup");
    for (int i = 0; i < startup_list_widget_->count(); ++i) {
        settings_->setArrayIndex(i);
        settings_->setValue("path", startup_list_widget_->item(i)->text());
    }
    settings_->endArray();
    
    // 同步到注册表
    settings_->sync();
}

void SettingsDialog::onAutoStartChanged(bool checked)
{
    // 修改注册表实现开机自启动
    QSettings reg_settings(REG_RUN_KEY, QSettings::NativeFormat);
    
    if (checked) {
        // 获取当前程序路径
        QString app_path = QCoreApplication::applicationFilePath();
        app_path = QDir::toNativeSeparators(app_path);
        
        // 写入注册表
        reg_settings.setValue(APP_NAME, app_path);
    } else {
        // 从注册表删除
        reg_settings.remove(APP_NAME);
    }
}

void SettingsDialog::onSandboxBorderChanged(bool checked)
{
    // 写入配置文件 settings.json，通知主窗口更新沙箱边框颜色
    QString config_path = QCoreApplication::applicationDirPath() + "/settings.json";
    
    QFile file(config_path);
    if (!file.open(QIODevice::ReadWrite | QIODevice::Text)) {
        qWarning() << "无法打开配置文件:" << config_path;
        return;
    }
    
    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    QJsonObject root = doc.object();
    
    root["sandbox_border"] = checked;
    
    file.resize(0);
    file.write(QJsonDocument(root).toJson());
    file.close();
    
    // 通知主窗口更新沙箱边框颜色（通过信号槽）
    emit sandboxBorderChanged(checked);
}

void SettingsDialog::onCacheSizeChanged(int value)
{
    // 写入配置文件 settings.json，通知 Hook 引擎调整缓存大小
    QString config_path = QCoreApplication::applicationDirPath() + "/settings.json";
    
    QFile file(config_path);
    if (!file.open(QIODevice::ReadWrite | QIODevice::Text)) {
        qWarning() << "无法打开配置文件:" << config_path;
        return;
    }
    
    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    QJsonObject root = doc.object();
    
    root["cache_size"] = value;
    
    file.resize(0);
    file.write(QJsonDocument(root).toJson());
    file.close();
    
    // 通知 Hook 引擎调整缓存大小（通过信号槽）
    emit cacheSizeChanged(value);
}

void SettingsDialog::onOkClicked()
{
    // 保存设置
    saveSettings();
    
    // 关闭对话框
    accept();
}

void SettingsDialog::onCancelClicked()
{
    // 放弃修改，关闭对话框
    reject();
}

void SettingsDialog::contextMenuEvent(QContextMenuEvent* event)
{
    QMenu context_menu(this);
    
    QAction* import_action = context_menu.addAction(tr("导入设置"));
    QAction* export_action = context_menu.addAction(tr("导出设置"));
    
    connect(import_action, &QAction::triggered, [this]() {
        QString file = QFileDialog::getOpenFileName(this, tr("导入设置"),
                                                    "", tr("配置文件 (*.json)"));
        if (!file.isEmpty()) {
            // 导入设置逻辑
            QFile src_file(file);
            if (src_file.open(QIODevice::ReadOnly | QIODevice::Text)) {
                QFile dst_file(QCoreApplication::applicationDirPath() + "/settings.json");
                if (dst_file.open(QIODevice::WriteOnly | QIODevice::Text)) {
                    dst_file.write(src_file.readAll());
                    dst_file.close();
                    src_file.close();
                    
                    // 重新加载设置
                    loadSettings();
                    
                    QMessageBox::information(this, tr("成功"), tr("设置已导入"));
                }
            }
        }
    });
    
    connect(export_action, &QAction::triggered, [this]() {
        QString file = QFileDialog::getSaveFileName(this, tr("导出设置"),
                                                    "", tr("配置文件 (*.json)"));
        if (!file.isEmpty()) {
            // 导出设置逻辑
            QFile src_file(QCoreApplication::applicationDirPath() + "/settings.json");
            if (src_file.open(QIODevice::ReadOnly | QIODevice::Text)) {
                QFile dst_file(file);
                if (dst_file.open(QIODevice::WriteOnly | QIODevice::Text)) {
                    dst_file.write(src_file.readAll());
                    dst_file.close();
                    src_file.close();
                    
                    QMessageBox::information(this, tr("成功"), tr("设置已导出"));
                }
            }
        }
    });
    
    context_menu.exec(event->globalPos());
}

#include "main_window.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QPushButton>
#include <QFileDialog>
#include <QMessageBox>
#include <QInputDialog>
#include <QProcess>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QFile>
#include <QTextStream>
#include <QDir>
#include <QFileInfo>
#include <QIcon>
#include <QFileIconProvider>
#include <QAction>
#include <QMenu>
#include <QSystemTrayIcon>
#include <QCloseEvent>
#include <QContextMenuEvent>
#include <QApplication>
#include <QSettings>
#include <QElapsedTimer>
#include <QDebug>

namespace launchpad {

// 构造函数
MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent),
    app_list_view_(nullptr),
    app_model_(nullptr),
    search_box_(nullptr),
    trayIcon_(nullptr),
    trayMenu_(nullptr),
    statusLabel_(nullptr),
    sandboxModeLabel_(nullptr) {
    
    // 初始化 UI
    initUI();
    
    // 加载应用列表
    loadAppList();
    
    // 创建托盘图标
    CreateTrayIcon();
    
    // 设置窗口属性
    setWindowTitle("AI ThinApp Launchpad");
    resize(800, 600);
}

// 析构函数
MainWindow::~MainWindow() {
    // 清理资源
    for (auto& app : apps_) {
        if (app.process && app.process->state() != QProcess::NotRunning) {
            app.process->terminate();
            app.process->waitForFinished(3000);
        }
        delete app.process;
    }
}

// 初始化 UI（创建左侧导航栏、右侧应用网格、顶部搜索栏）
void MainWindow::initUI() {
    // 创建中央部件
    QWidget* centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);
    
    // 创建主布局
    QVBoxLayout* mainLayout = new QVBoxLayout(centralWidget);
    
    // 创建顶部工具栏
    QHBoxLayout* topToolBar = new QHBoxLayout();
    
    // 搜索框
    QLabel* searchLabel = new QLabel("搜索：", this);
    search_box_ = new QLineEdit(this);
    search_box_->setPlaceholderText("输入应用名称搜索...");
    connect(search_box_, &QLineEdit::textChanged, this, &MainWindow::onSearchTextChanged);
    topToolBar->addWidget(searchLabel);
    topToolBar->addWidget(search_box_);
    topToolBar->addStretch();
    
    // 添加应用按钮
    QPushButton* addAppButton = new QPushButton("添加应用", this);
    connect(addAppButton, &QPushButton::clicked, this, &MainWindow::OnAddApp);
    topToolBar->addWidget(addAppButton);
    
    // 设置按钮
    QPushButton* settingsButton = new QPushButton("设置", this);
    connect(settingsButton, &QPushButton::clicked, this, &MainWindow::OnSettings);
    topToolBar->addWidget(settingsButton);
    
    mainLayout->addLayout(topToolBar);
    
    // 创建分类过滤器
    QHBoxLayout* filterLayout = new QHBoxLayout();
    QPushButton* allButton = new QPushButton("全部", this);
    QPushButton* runningButton = new QPushButton("运行中", this);
    QPushButton* installedButton = new QPushButton("已安装", this);
    QPushButton* notInstalledButton = new QPushButton("未安装", this);
    QPushButton* updatedButton = new QPushButton("有更新", this);
    
    filterLayout->addWidget(allButton);
    filterLayout->addWidget(runningButton);
    filterLayout->addWidget(installedButton);
    filterLayout->addWidget(notInstalledButton);
    filterLayout->addWidget(updatedButton);
    filterLayout->addStretch();
    
    mainLayout->addLayout(filterLayout);
    
    // 创建应用列表视图
    app_list_view_ = new QListView(this);
    app_model_ = new QStandardItemModel(this);
    app_list_view_->setModel(app_model_);
    app_list_view_->setViewMode(QListView::IconMode);
    app_list_view_->setIconSize(QSize(64, 64));
    app_list_view_->setGridSize(QSize(100, 100));
    app_list_view_->setSpacing(10);
    app_list_view_->setMovement(QListView::Static);
    app_list_view_->setResizeMode(QListView::Adjust);
    
    // 连接点击事件
    connect(app_list_view_, &QListView::clicked, this, &MainWindow::onAppSelected);
    connect(app_list_view_, &QListView::doubleClicked, this, [this](const QModelIndex& index) {
        emit appLaunched(app_model_->itemFromIndex(index)->data(Qt::UserRole).toString());
    });
    
    mainLayout->addWidget(app_list_view_);
    
    // 创建状态栏
    statusLabel_ = new QLabel("0 个应用运行中", this);
    sandboxModeLabel_ = new QLabel("沙箱模式已启用", this);
    statusBar()->addWidget(statusLabel_);
    statusBar()->addPermanentWidget(sandboxModeLabel_);
}

// 加载应用列表（从 apps.json 读取应用列表，填充 QStandardItemModel）
void MainWindow::loadAppList() {
    // 清空模型
    app_model_->clear();
    apps_.clear();
    
    // 读取 apps.json 文件
    QString configPath = QCoreApplication::applicationDirPath() + "/apps.json";
    QFile configFile(configPath);
    
    if (!configFile.exists()) {
        qWarning() << "配置文件不存在:" << configPath;
        return;
    }
    
    if (!configFile.open(QIODevice::ReadOnly)) {
        qWarning() << "无法打开配置文件:" << configPath;
        return;
    }
    
    QByteArray jsonData = configFile.readAll();
    configFile.close();
    
    QJsonDocument jsonDoc = QJsonDocument::fromJson(jsonData);
    if (!jsonDoc.isArray()) {
        qWarning() << "配置文件格式错误，应为 JSON 数组";
        return;
    }
    
    QJsonArray jsonArray = jsonDoc.array();
    for (const QJsonValue& value : jsonArray) {
        if (!value.isObject()) continue;
        
        QJsonObject obj = value.toObject();
        AppInfo appInfo;
        appInfo.name = obj["name"].toString();
        appInfo.exePath = obj["exePath"].toString();
        appInfo.iconPath = obj["iconPath"].toString();
        appInfo.appDir = obj["appDir"].toString();
        appInfo.isRunning = false;
        appInfo.process = nullptr;
        
        apps_.append(appInfo);
        
        // 添加到模型
        QStandardItem* item = new QStandardItem(QIcon(appInfo.iconPath), appInfo.name);
        item->setData(appInfo.exePath, Qt::UserRole);
        app_model_->appendRow(item);
    }
    
    qInfo() << "加载了" << apps_.size() << "个应用";
}

// 应用选中槽（单击应用卡片）
void MainWindow::onAppSelected(const QModelIndex& index) {
    QStandardItem* item = app_model_->itemFromIndex(index);
    if (!item) return;
    
    QString appPath = item->data(Qt::UserRole).toString();
    qInfo() << "选中应用:" << appPath;
}

// 搜索文本变化槽（实时过滤应用列表）
void MainWindow::onSearchTextChanged(const QString& text) {
    for (int i = 0; i < app_model_->rowCount(); ++i) {
        QStandardItem* item = app_model_->item(i);
        if (!item) continue;
        
        QString appName = item->text();
        bool visible = text.isEmpty() || appName.contains(text, Qt::CaseInsensitive);
        app_list_view_->setRowHidden(i, !visible);
    }
}

// 托盘图标点击（显示/隐藏窗口）
void MainWindow::OnTrayIconActivated(QSystemTrayIcon::ActivationReason reason) {
    if (reason == QSystemTrayIcon::Trigger) {
        if (isVisible()) {
            hide();
        } else {
            show();
            raise();
            activateWindow();
        }
    }
}

// 添加应用（打开文件选择器，选择 exe 文件）
void MainWindow::OnAddApp() {
    QString exePath = QFileDialog::getOpenFileName(this, "选择应用 exe 文件", "", "可执行文件 (*.exe)");
    if (exePath.isEmpty()) return;
    
    QFileInfo fileInfo(exePath);
    AppInfo appInfo;
    appInfo.name = fileInfo.baseName();
    appInfo.exePath = exePath;
    appInfo.iconPath = "";
    appInfo.appDir = QCoreApplication::applicationDirPath() + "/apps/" + appInfo.name;
    appInfo.isRunning = false;
    appInfo.process = nullptr;
    
    // 提取图标
    QFileIconProvider iconProvider;
    QIcon icon = iconProvider.icon(fileInfo);
    
    apps_.append(appInfo);
    
    // 添加到模型
    QStandardItem* item = new QStandardItem(icon, appInfo.name);
    item->setData(appInfo.exePath, Qt::UserRole);
    app_model_->appendRow(item);
    
    // 保存到配置文件
    // TODO: 实现保存逻辑
    
    qInfo() << "添加应用:" << appInfo.name;
}

// 删除应用（从列表中移除，可选删除沙箱数据）
void MainWindow::OnRemoveApp() {
    QModelIndex currentIndex = app_list_view_->currentIndex();
    if (!currentIndex.isValid()) {
        QMessageBox::warning(this, "警告", "请先选择一个应用");
        return;
    }
    
    QStandardItem* item = app_model_->itemFromIndex(currentIndex);
    QString appName = item->text();
    
    int ret = QMessageBox::question(this, "确认删除", 
                                    QString("确定要删除应用 %1 吗？").arg(appName),
                                    QMessageBox::Yes | QMessageBox::No);
    if (ret == QMessageBox::Yes) {
        // 从模型中移除
        app_model_->removeRow(currentIndex.row());
        
        // 从应用列表中移除
        for (int i = 0; i < apps_.size(); ++i) {
            if (apps_[i].name == appName) {
                apps_.removeAt(i);
                break;
            }
        }
        
        qInfo() << "删除应用:" << appName;
    }
}

// 打开应用目录（在文件资源管理器中显示应用目录）
void MainWindow::OnOpenAppDir() {
    QModelIndex currentIndex = app_list_view_->currentIndex();
    if (!currentIndex.isValid()) {
        QMessageBox::warning(this, "警告", "请先选择一个应用");
        return;
    }
    
    QStandardItem* item = app_model_->itemFromIndex(currentIndex);
    QString appPath = item->data(Qt::UserRole).toString();
    QFileInfo fileInfo(appPath);
    QString dirPath = fileInfo.absolutePath();
    
    QDesktopServices::openUrl(QUrl::fromLocalFile(dirPath));
}

// 设置（打开设置对话框）
void MainWindow::OnSettings() {
    QMessageBox::information(this, "设置", "设置对话框（待实现）");
}

// 退出应用（完全退出 Launchpad）
void MainWindow::OnExit() {
    QApplication::quit();
}

// 关于（显示关于对话框）
void MainWindow::OnAbout() {
    QMessageBox::about(this, "关于 AI ThinApp Launchpad", 
                       "AI ThinApp Launchpad\n版本: v0.1.0 (MVP)\n\nGitHub: https://github.com/ai-thinapp/...\n许可证: GPL v3");
}

// 关闭事件（点击窗口关闭按钮时最小化到托盘，而非退出）
void MainWindow::closeEvent(QCloseEvent* event) {
    if (trayIcon_ && trayIcon_->isVisible()) {
        hide();
        event->ignore();
    } else {
        event->accept();
    }
}

// 右键菜单事件（显示上下文菜单）
void MainWindow::contextMenuEvent(QContextMenuEvent* event) {
    QMenu contextMenu(this);
    
    QAction* launchAction = contextMenu.addAction("启动");
    QAction* openDirAction = contextMenu.addAction("打开目录");
    QAction* deleteAction = contextMenu.addAction("删除");
    
    QAction* selectedAction = contextMenu.exec(event->globalPos());
    if (selectedAction == launchAction) {
        QModelIndex currentIndex = app_list_view_->currentIndex();
        if (currentIndex.isValid()) {
            QStandardItem* item = app_model_->itemFromIndex(currentIndex);
            emit appLaunched(item->data(Qt::UserRole).toString());
        }
    } else if (selectedAction == openDirAction) {
        OnOpenAppDir();
    } else if (selectedAction == deleteAction) {
        OnRemoveApp();
    }
}

// 创建托盘图标和右键菜单
void MainWindow::CreateTrayIcon() {
    // 创建托盘图标
    trayIcon_ = new QSystemTrayIcon(this);
    trayIcon_->setIcon(QIcon(":/icons/tray_icon.png"));
    trayIcon_->setToolTip("AI ThinApp Launchpad");
    
    // 创建右键菜单
    trayMenu_ = new QMenu(this);
    
    QAction* showAction = trayMenu_->addAction("显示/隐藏 Launchpad");
    connect(showAction, &QAction::triggered, this, &MainWindow::ShowHide);
    
    trayMenu_->addSeparator();
    
    QAction* settingsAction = trayMenu_->addAction("设置");
    connect(settingsAction, &QAction::triggered, this, &MainWindow::OnSettings);
    
    QAction* aboutAction = trayMenu_->addAction("关于");
    connect(aboutAction, &QAction::triggered, this, &MainWindow::OnAbout);
    
    QAction* exitAction = trayMenu_->addAction("退出");
    connect(exitAction, &QAction::triggered, this, &MainWindow::OnExit);
    
    trayIcon_->setContextMenu(trayMenu_);
    
    // 连接信号
    connect(trayIcon_, &QSystemTrayIcon::activated, this, &MainWindow::OnTrayIconActivated);
    
    // 显示托盘图标
    trayIcon_->show();
}

// 显示/隐藏窗口（用于托盘图标切换）
void MainWindow::ShowHide() {
    if (isVisible()) {
        hide();
    } else {
        show();
        raise();
        activateWindow();
    }
}

}  // namespace launchpad

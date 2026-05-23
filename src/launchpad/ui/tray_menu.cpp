#include "tray_menu.h"
#include <QMenu>
#include <QAction>
#include <QMessageBox>

namespace launchpad {

// 构造函数
TrayMenu::TrayMenu(QWidget* parent) : QMenu(parent),
    show_action_(nullptr),
    settings_action_(nullptr),
    about_action_(nullptr),
    exit_action_(nullptr) {
    
    // 初始化 UI
    initUI();
}

// 析构函数
TrayMenu::~TrayMenu() {
    // 清理资源（Qt 对象树自动管理子对象）
}

// 初始化 UI（创建菜单项、连接信号/槽）
void TrayMenu::initUI() {
    // 显示/隐藏 Launchpad
    show_action_ = new QAction("显示/隐藏 Launchpad", this);
    connect(show_action_, &QAction::triggered, this, &TrayMenu::onShowLaunchpad);
    addAction(show_action_);
    
    // 分隔符
    addSeparator();
    
    // 启动应用子菜单（待实现）
    QMenu* launchMenu = addMenu("启动应用");
    QAction* placeholder = launchMenu->addAction("（待实现）");
    placeholder->setEnabled(false);
    
    // 打开应用目录
    QAction* openDirAction = new QAction("打开应用目录", this);
    connect(openDirAction, &QAction::triggered, this, [this]() {
        // TODO: 实现打开应用目录功能
        QMessageBox::information(this, "提示", "打开应用目录（待实现）");
    });
    addAction(openDirAction);
    
    // 分隔符
    addSeparator();
    
    // 设置
    settings_action_ = new QAction("设置", this);
    connect(settings_action_, &QAction::triggered, this, &TrayMenu::onShowSettings);
    addAction(settings_action_);
    
    // 关于
    about_action_ = new QAction("关于", this);
    connect(about_action_, &QAction::triggered, this, [this]() {
        QMessageBox::about(this, "关于 AI ThinApp Launchpad", 
                           "AI ThinApp Launchpad\n版本: v0.1.0 (MVP)\n\nGitHub: https://github.com/ai-thinapp/...\n许可证: GPL v3");
    });
    addAction(about_action_);
    
    // 退出
    exit_action_ = new QAction("退出", this);
    connect(exit_action_, &QAction::triggered, this, &TrayMenu::onExitApp);
    addAction(exit_action_);
}

// 显示 Launchpad 槽
void TrayMenu::onShowLaunchpad() {
    emit showLaunchpad();
}

// 显示设置槽
void TrayMenu::onShowSettings() {
    emit showSettings();
}

// 退出应用槽
void TrayMenu::onExitApp() {
    emit exitApp();
}

}  // namespace launchpad

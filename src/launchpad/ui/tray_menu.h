#pragma once

#include <QMenu>
#include <QAction>
#include <QWidget>

namespace launchpad {

// 托盘菜单（右键菜单）
class TrayMenu : public QMenu {
    Q_OBJECT

public:
    // 构造函数
    // parent: 父窗口指针
    explicit TrayMenu(QWidget* parent = nullptr);

    // 析构函数
    ~TrayMenu();

signals:
    // 显示 Launchpad 信号
    void showLaunchpad();

    // 显示设置信号
    void showSettings();

    // 退出应用信号
    void exitApp();

private slots:
    // 显示 Launchpad 槽
    void onShowLaunchpad();

    // 显示设置槽
    void onShowSettings();

    // 退出应用槽
    void onExitApp();

private:
    // 初始化 UI（创建菜单项、连接信号/槽）
    void initUI();

    // 菜单项
    QAction* show_action_;      // 显示/隐藏 Launchpad
    QAction* settings_action_;  // 设置
    QAction* about_action_;      // 关于
    QAction* exit_action_;      // 退出
};

}  // namespace launchpad

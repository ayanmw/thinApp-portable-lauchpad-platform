#include <QApplication>
#include <QSystemTrayIcon>
#include <QStyleFactory>
#include <QPalette>
#include <QMessageBox>
#include "ui/main_window.h"

int main(int argc, char* argv[]) {
    // 创建 Qt 应用程序对象
    QApplication app(argc, argv);
    
    // 设置应用程序元数据
    QCoreApplication::setApplicationName("AI ThinApp Launchpad");
    QCoreApplication::setApplicationVersion("1.0.0");
    QCoreApplication::setOrganizationName("AI ThinApp");
    QCoreApplication::setOrganizationDomain("aithinapp.com");
    
    // 检查系统托盘是否可用
    if (!QSystemTrayIcon::isSystemTrayAvailable()) {
        QMessageBox::critical(nullptr, "错误", 
                            "系统不支持托盘图标，无法启动 Launchpad");
        return 1;
    }
    
    // 设置应用程序样式（使用 Fusion 样式，跨平台一致）
    QApplication::setStyle(QStyleFactory::create("Fusion"));
    
    // 设置深色主题（可选，POC 阶段使用浅色主题）
    // QPalette darkPalette;
    // darkPalette.setColor(QPalette::Window, QColor(53, 53, 53));
    // darkPalette.setColor(QPalette::WindowText, Qt::white);
    // darkPalette.setColor(QPalette::Base, QColor(25, 25, 25));
    // darkPalette.setColor(QPalette::AlternateBase, QColor(53, 53, 53));
    // darkPalette.setColor(QPalette::ToolTipBase, Qt::white);
    // darkPalette.setColor(QPalette::ToolTipText, Qt::white);
    // darkPalette.setColor(QPalette::Text, Qt::white);
    // darkPalette.setColor(QPalette::Button, QColor(53, 53, 53));
    // darkPalette.setColor(QPalette::ButtonText, Qt::white);
    // darkPalette.setColor(QPalette::BrightText, Qt::red);
    // darkPalette.setColor(QPalette::Link, QColor(42, 130, 218));
    // darkPalette.setColor(QPalette::Highlight, QColor(42, 130, 218));
    // darkPalette.setColor(QPalette::HighlightedText, Qt::black);
    // QApplication::setPalette(darkPalette);
    
    // 创建主窗口
    launchpad::MainWindow mainWindow;
    
    // 连接托盘图标的退出信号（完全退出应用）
    QObject::connect(&mainWindow, &QObject::destroyed, 
                     []() {
                         QApplication::quit();
                     });
    
    // 显示主窗口（POC 阶段直接显示，正式版可最小化到托盘）
    mainWindow.show();
    
    // 显示启动通知（托盘气泡通知）
    if (QSystemTrayIcon::supportsMessages()) {
        mainWindow.findChild<QSystemTrayIcon*>()->showMessage(
            "AI ThinApp Launchpad",
            "Launchpad 已启动，双击托盘图标显示/隐藏窗口",
            QSystemTrayIcon::Information,
            3000  // 3 秒后自动关闭
        );
    }
    
    // 进入 Qt 事件循环
    return QApplication::exec();
}

#pragma once

#include <QMainWindow>
#include <QListView>
#include <QStandardItemModel>
#include <QLineEdit>
#include <QSystemTrayIcon>
#include <QMenu>
#include <QAction>
#include <QPushButton>
#include <QLabel>
#include <QVector>
#include <QProcess>
#include <QModelIndex>
#include <QContextMenuEvent>

namespace launchpad {

// Launchpad 主窗口
class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    // 构造函数
    // parent: 父窗口指针
    explicit MainWindow(QWidget* parent = nullptr);

    // 析构函数
    ~MainWindow();

signals:
    // 应用启动信号
    // app_path: 应用路径
    void appLaunched(QString app_path);

    // 应用终止信号
    // app_path: 应用路径
    void appTermated(QString app_path);

private slots:
    // 应用选中槽（单击应用卡片）
    // index: 选中的模型索引
    void onAppSelected(const QModelIndex& index);

    // 搜索文本变化槽（实时过滤应用列表）
    // text: 搜索文本
    void onSearchTextChanged(const QString& text);

    // 托盘图标点击（显示/隐藏窗口）
    // reason: 激活原因（点击、双击等）
    void OnTrayIconActivated(QSystemTrayIcon::ActivationReason reason);

    // 添加应用（打开文件选择器，选择 exe 文件）
    void OnAddApp();

    // 删除应用（从列表中移除，可选删除沙箱数据）
    void OnRemoveApp();

    // 打开应用目录（在文件资源管理器中显示应用目录）
    void OnOpenAppDir();

    // 设置（打开设置对话框）
    void OnSettings();

    // 退出应用（完全退出 Launchpad）
    void OnExit();

    // 关于（显示关于对话框）
    void OnAbout();

protected:
    // 关闭事件（点击窗口关闭按钮时最小化到托盘，而非退出）
    void closeEvent(QCloseEvent* event) override;

    // 右键菜单事件（显示上下文菜单）
    // event: 右键菜单事件
    void contextMenuEvent(QContextMenuEvent* event) override;

private:
    // 初始化 UI（创建左侧导航栏、右侧应用网格、顶部搜索栏）
    void initUI();

    // 加载应用列表（从 apps.json 读取应用列表，填充 QStandardItemModel）
    void loadAppList();

    // 创建托盘图标和右键菜单
    void CreateTrayIcon();

    // UI 组件
    QListView* app_list_view_;           // 应用列表视图
    QStandardItemModel* app_model_;      // 应用列表模型
    QLineEdit* search_box_;              // 搜索框
    QSystemTrayIcon* trayIcon_;         // 系统托盘图标
    QMenu* trayMenu_;                   // 托盘右键菜单

    // 状态栏标签
    QLabel* statusLabel_;               // 状态栏左侧标签（显示运行中的应用数量）
    QLabel* sandboxModeLabel_;          // 状态栏右侧标签（显示沙箱模式状态）

    // 应用数据
    struct AppInfo {
        QString name;       // 应用名称
        QString exePath;    // exe 文件路径
        QString iconPath;   // 图标文件路径
        QString appDir;     // 应用目录路径（沙箱根目录）
        bool isRunning;     // 是否正在运行
        QProcess* process;  // 进程指针（运行中非空）
    };
    QVector<AppInfo> apps_; // 应用列表
};

}  // namespace launchpad

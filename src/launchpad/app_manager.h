#pragma once

#include <QObject>
#include <QProcess>
#include <QVector>
#include <QMap>
#include <QString>
#include <QSystemTrayIcon>

namespace launchpad {

// 应用管理器（负责启动/停止应用，管理沙箱进程）
class AppManager : public QObject {
    Q_OBJECT

public:
    // 构造函数
    // parent: 父对象指针
    explicit AppManager(QObject* parent = nullptr);

    // 析构函数
    ~AppManager();

    // 启动应用（注入 Hook DLL，设置环境变量）
    // appDir: 应用目录路径（沙箱根目录）
    // 返回值：启动成功返回 true，失败返回 false
    bool LaunchApp(const QString& appDir);

    // 停止应用（终止进程）
    // appDir: 应用目录路径
    // 返回值：停止成功返回 true，失败返回 false
    bool StopApp(const QString& appDir);

    // 添加应用（浏览 .vapp 包，解压，注册到 apps.json）
    // vappPath: .vapp 包路径（可选，为空则打开文件选择器）
    // 返回值：添加成功返回 true，失败返回 false
    bool addApp(const QString& vappPath = "");

    // 删除应用（从 apps.json 删除，删除应用目录）
    // appDir: 应用目录路径
    // 返回值：删除成功返回 true，失败返回 false
    bool removeApp(const QString& appDir);

    // 显示 Balloon 通知
    // title: 通知标题
    // message: 通知内容
    // icon: 通知图标（Information/Warning/Critical）
    void showBalloonNotification(const QString& title, 
                               const QString& message, 
                               QSystemTrayIcon::MessageIcon icon = QSystemTrayIcon::Information);

    // 获取运行中的应用列表
    // 返回值：正在运行的应用目录路径列表
    QVector<QString> GetRunningApps() const;

    // 检查应用是否正在运行
    // appDir: 应用目录路径
    // 返回值：运行中返回 true，否则返回 false
    bool IsAppRunning(const QString& appDir) const;

    // 设置托盘图标指针（用于显示 Balloon 通知）
    // trayIcon: 托盘图标指针
    void setTrayIcon(QSystemTrayIcon* trayIcon);

    // 打开沙箱文件浏览器
    // appDir: 应用目录路径
    // 返回值：打开成功返回 true，失败返回 false
    bool openSandboxBrowser(const QString& appDir);

    // 导出沙箱文件到真实文件系统
    // sandboxFilePath: 沙箱内的文件路径
    // destPath: 导出目标路径（可选，为空则打开文件选择器）
    // 返回值：导出成功返回 true，失败返回 false
    bool exportSandboxFile(const QString& sandboxFilePath, const QString& destPath = "");

signals:
    // 应用启动成功信号
    // appDir: 应用目录路径
    void AppLaunched(const QString& appDir);

    // 应用停止信号
    // appDir: 应用目录路径
    void AppStopped(const QString& appDir);

    // 应用崩溃信号
    // appDir: 应用目录路径
    // exitCode: 退出码
    void AppCrashed(const QString& appDir, int exitCode);

    // Hook 注入失败信号
    // appDir: 应用目录路径
    // error: 错误信息
    void HookInjectionFailed(const QString& appDir, const QString& error);

private slots:
    // 进程结束槽（处理进程退出事件）
    // exitCode: 退出码
    void OnProcessFinished(int exitCode);

private:
    // 查找应用 exe 文件路径
    // appDir: 应用目录路径
    // 返回值：exe 文件路径，未找到返回空字符串
    QString FindAppExecutable(const QString& appDir) const;

    // 注入 Hook DLL（设置环境变量，让应用加载 Hook DLL）
    // appDir: 应用目录路径
    // process: QProcess 指针
    // 返回值：注入成功返回 true，失败返回 false
    bool InjectHookDll(const QString& appDir, QProcess* process);

    // 终止进程树（父进程 + 所有子进程）
    // parentPid: 父进程 ID
    void TerminateProcessTree(DWORD parentPid);

    // 运行中的进程列表
    // key: 应用目录路径
    // value: QProcess 指针
    QMap<QString, QProcess*> runningProcesses_;

    // 托盘图标指针（用于显示 Balloon 通知）
    QSystemTrayIcon* trayIcon_;
};

}  // namespace launchpad

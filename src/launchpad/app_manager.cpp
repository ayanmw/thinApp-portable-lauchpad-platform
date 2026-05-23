#include "app_manager.h"
#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>
#include <QDebug>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QFile>
#include <QTextStream>
#include <QFileDialog>
#include <QMessageBox>
#include <Windows.h>
#include <TlHelp32.h>
#include "ui/sandbox_browser.h"

namespace launchpad {

// 构造函数
AppManager::AppManager(QObject* parent) 
    : QObject(parent), trayIcon_(nullptr) {
    // 无需额外初始化
}

// 析构函数
AppManager::~AppManager() {
    // 停止所有运行中的进程
    for (auto it = runningProcesses_.begin(); it != runningProcesses_.end(); ++it) {
        if (it.value() && it.value()->state() != QProcess::NotRunning) {
            it.value()->kill(); // 强制终止
            it.value()->waitForFinished(3000); // 等待 3 秒
        }
    }
    runningProcesses_.clear();
}

// 启动应用（注入 Hook DLL）
bool AppManager::LaunchApp(const QString& appDir) {
    // 检查应用目录是否存在
    QDir dir(appDir);
    if (!dir.exists()) {
        qWarning() << "应用目录不存在:" << appDir;
        emit HookInjectionFailed(appDir, "应用目录不存在");
        return false;
    }
    
    // 查找应用 exe 文件
    QString exePath = FindAppExecutable(appDir);
    if (exePath.isEmpty()) {
        qWarning() << "未找到 exe 文件:" << appDir;
        emit HookInjectionFailed(appDir, "未找到 exe 文件");
        return false;
    }
    
    // 创建 QProcess 对象
    QProcess* process = new QProcess(this);
    
    // 注入 Hook DLL（设置环境变量 + 远程线程注入）
    if (!InjectHookDll(appDir, process)) {
        qWarning() << "Hook DLL 注入失败:" << appDir;
        emit HookInjectionFailed(appDir, "Hook DLL 注入失败");
        delete process;
        return false;
    }
    
    // 启动进程
    process->start(exePath);
    if (!process->waitForStarted(5000)) { // 等待 5 秒
        qWarning() << "应用启动超时:" << exePath;
        delete process;
        return false;
    }
    
    // 连接进程结束信号
    connect(process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, &AppManager::OnProcessFinished);
    
    // 添加到运行列表
    runningProcesses_[appDir] = process;
    
    // 发射应用启动成功信号
    emit AppLaunched(appDir);
    
    // 显示 Balloon 通知
    QFileInfo fileInfo(exePath);
    showBalloonNotification("应用启动成功", 
                          QString("%1 已在沙箱模式中启动").arg(fileInfo.baseName()),
                          QSystemTrayIcon::Information);
    
    qInfo() << "应用启动成功:" << appDir;
    return true;
}

// 停止应用（终止进程树）
bool AppManager::StopApp(const QString& appDir) {
    // 查找运行中的进程
    if (!runningProcesses_.contains(appDir)) {
        qWarning() << "应用未运行:" << appDir;
        return false;
    }
    
    QProcess* process = runningProcesses_[appDir];
    if (!process || process->state() == QProcess::NotRunning) {
        // 进程已结束，从列表中移除
        runningProcesses_.remove(appDir);
        emit AppStopped(appDir);
        
        // 显示 Balloon 通知
        QFileInfo fileInfo(process ? process->program() : "");
        showBalloonNotification("应用已停止", 
                              QString("%1 已正常退出").arg(fileInfo.baseName()),
                              QSystemTrayIcon::Information);
        return true;
    }
    
    // 获取进程 ID
    DWORD processId = process->processId();
    
    // 遍历进程树，终止所有子进程
    if (processId != 0) {
        TerminateProcessTree(processId);
    }
    
    // 终止主进程
    process->kill();
    process->waitForFinished(3000);
    
    // 从列表中移除
    runningProcesses_.remove(appDir);
    
    // 发射应用停止信号
    emit AppStopped(appDir);
    
    // 显示 Balloon 通知
    QFileInfo fileInfo(process->program());
    showBalloonNotification("应用已停止", 
                          QString("%1 已正常退出").arg(fileInfo.baseName()),
                          QSystemTrayIcon::Information);
    
    qInfo() << "应用已停止:" << appDir;
    return true;
}

// 添加应用（浏览 .vapp 包，解压，注册到 apps.json）
bool AppManager::addApp(const QString& vappPath) {
    // 打开文件选择器（如果未提供路径）
    QString packagePath = vappPath;
    if (packagePath.isEmpty()) {
        packagePath = QFileDialog::getOpenFileName(nullptr, "选择 .vapp 包", "", "VApp 包 (*.vapp)");
    }
    
    if (packagePath.isEmpty()) {
        return false;
    }
    
    // 创建应用目录
    QFileInfo fileInfo(packagePath);
    QString appName = fileInfo.baseName();
    QString appDir = QCoreApplication::applicationDirPath() + "/apps/" + appName;
    
    QDir dir;
    if (!dir.mkpath(appDir)) {
        qWarning() << "无法创建应用目录:" << appDir;
        return false;
    }
    
    // 解压 .vapp 包（假设是 zip 格式）
    // TODO: 实现解压逻辑
    
    // 注册到 apps.json
    QString configPath = QCoreApplication::applicationDirPath() + "/apps.json";
    QFile configFile(configPath);
    
    QJsonArray jsonArray;
    if (configFile.exists() && configFile.open(QIODevice::ReadOnly)) {
        QByteArray jsonData = configFile.readAll();
        configFile.close();
        
        QJsonDocument jsonDoc = QJsonDocument::fromJson(jsonData);
        if (jsonDoc.isArray()) {
            jsonArray = jsonDoc.array();
        }
    }
    
    // 添加应用信息
    QJsonObject appObj;
    appObj["name"] = appName;
    appObj["exePath"] = appDir + "/" + appName + ".exe";
    appObj["iconPath"] = appDir + "/icon.ico";
    appObj["appDir"] = appDir;
    appObj["isPortable"] = true;
    appObj["lastLaunchTime"] = QDateTime::currentDateTime().toString(Qt::ISODate);
    
    jsonArray.append(appObj);
    
    // 保存到配置文件
    if (!configFile.open(QIODevice::WriteOnly)) {
        qWarning() << "无法写入配置文件:" << configPath;
        return false;
    }
    
    QJsonDocument jsonDoc(jsonArray);
    configFile.write(jsonDoc.toJson());
    configFile.close();
    
    qInfo() << "应用添加成功:" << appName;
    return true;
}

// 删除应用（从 apps.json 删除，删除应用目录）
bool AppManager::removeApp(const QString& appDir) {
    // 检查应用是否正在运行
    if (runningProcesses_.contains(appDir)) {
        QMessageBox::warning(nullptr, "警告", "应用正在运行中，请先停止应用");
        return false;
    }
    
    // 从 apps.json 删除
    QString configPath = QCoreApplication::applicationDirPath() + "/apps.json";
    QFile configFile(configPath);
    
    if (!configFile.exists()) {
        qWarning() << "配置文件不存在:" << configPath;
        return false;
    }
    
    if (!configFile.open(QIODevice::ReadOnly)) {
        qWarning() << "无法打开配置文件:" << configPath;
        return false;
    }
    
    QByteArray jsonData = configFile.readAll();
    configFile.close();
    
    QJsonDocument jsonDoc = QJsonDocument::fromJson(jsonData);
    if (!jsonDoc.isArray()) {
        qWarning() << "配置文件格式错误，应为 JSON 数组";
        return false;
    }
    
    QJsonArray jsonArray = jsonDoc.array();
    QJsonArray newArray;
    
    // 过滤掉要删除的应用
    for (const QJsonValue& value : jsonArray) {
        if (!value.isObject()) continue;
        
        QJsonObject obj = value.toObject();
        if (obj["appDir"].toString() != appDir) {
            newArray.append(obj);
        }
    }
    
    // 保存到配置文件
    if (!configFile.open(QIODevice::WriteOnly)) {
        qWarning() << "无法写入配置文件:" << configPath;
        return false;
    }
    
    jsonDoc = QJsonDocument(newArray);
    configFile.write(jsonDoc.toJson());
    configFile.close();
    
    // 删除应用目录
    QDir appDirObj(appDir);
    if (appDirObj.exists()) {
        if (!appDirObj.removeRecursively()) {
            qWarning() << "无法删除应用目录:" << appDir;
            return false;
        }
    }
    
    qInfo() << "应用删除成功:" << appDir;
    return true;
}

// 显示 Balloon 通知
void AppManager::showBalloonNotification(const QString& title, 
                                         const QString& message, 
                                         QSystemTrayIcon::MessageIcon icon) {
    if (trayIcon_ && trayIcon_->isVisible()) {
        trayIcon_->showMessage(title, message, icon, 5000); // 显示 5 秒
    }
}

// 获取运行中的应用列表
QVector<QString> AppManager::GetRunningApps() const {
    QVector<QString> runningApps;
    
    for (auto it = runningProcesses_.begin(); it != runningProcesses_.end(); ++it) {
        if (it.value() && it.value()->state() != QProcess::NotRunning) {
            runningApps.append(it.key());
        }
    }
    
    return runningApps;
}

// 检查应用是否正在运行
bool AppManager::IsAppRunning(const QString& appDir) const {
    if (!runningProcesses_.contains(appDir)) {
        return false;
    }
    
    QProcess* process = runningProcesses_[appDir];
    return (process && process->state() != QProcess::NotRunning);
}

// 设置托盘图标指针
void AppManager::setTrayIcon(QSystemTrayIcon* trayIcon) {
    trayIcon_ = trayIcon;
}

// 进程结束槽（处理进程退出事件）
void AppManager::OnProcessFinished(int exitCode) {
    // 查找是哪个进程结束了
    QProcess* senderProcess = qobject_cast<QProcess*>(sender());
    if (!senderProcess) return;
    
    // 从运行列表中查找对应的 appDir
    QString appDir;
    for (auto it = runningProcesses_.begin(); it != runningProcesses_.end(); ++it) {
        if (it.value() == senderProcess) {
            appDir = it.key();
            break;
        }
    }
    
    if (appDir.isEmpty()) return;
    
    // 从列表中移除
    runningProcesses_.remove(appDir);
    
    // 检查是否正常退出
    if (exitCode != 0) {
        // 异常退出，发射崩溃信号
        emit AppCrashed(appDir, exitCode);
        qWarning() << "应用崩溃:" << appDir << "退出码:" << exitCode;
        
        // 显示崩溃通知
        showBalloonNotification("应用崩溃", 
                              QString("应用异常退出（退出码：%1）").arg(exitCode),
                              QSystemTrayIcon::Warning);
    } else {
        // 正常退出，发射停止信号
        emit AppStopped(appDir);
        qInfo() << "应用已退出:" << appDir;
    }
}

// 查找应用 exe 文件路径
QString AppManager::FindAppExecutable(const QString& appDir) const {
    QDir dir(appDir);
    
    // 查找目录下的 exe 文件（排除 Hook DLL）
    QStringList exeFiles = dir.entryList(QStringList() << "*.exe", QDir::Files);
    for (const QString& fileName : exeFiles) {
        QString filePath = dir.absoluteFilePath(fileName);
        // 排除 Hook DLL（假设 Hook DLL 文件名包含 "hook" 或 "inject"）
        if (!fileName.toLower().contains("hook") && 
            !fileName.toLower().contains("inject")) {
            return filePath;
        }
    }
    
    return ""; // 未找到
}

// 打开沙箱文件浏览器
bool AppManager::openSandboxBrowser(const QString& appDir) {
    // 检查应用目录是否存在
    QDir dir(appDir);
    if (!dir.exists()) {
        qWarning() << "应用目录不存在:" << appDir;
        return false;
    }
    
    // 检查 VFS 目录是否存在
    QString vfsPath = appDir + "/VFS";
    QDir vfsDir(vfsPath);
    if (!vfsDir.exists()) {
        qWarning() << "VFS 目录不存在:" << vfsPath;
        return false;
    }
    
    // 创建沙箱浏览器对话框
    QWidget* parentWidget = nullptr;
    if (trayIcon_ && trayIcon_->parentWidget()) {
        parentWidget = trayIcon_->parentWidget();
    }
    
    SandboxBrowser* browser = new SandboxBrowser(parentWidget);
    browser->setAttribute(Qt::WA_DeleteOnClose);  // 关闭时自动删除
    browser->setSandboxPath(appDir);
    browser->setWindowTitle("沙箱文件浏览器 - " + QFileInfo(appDir).baseName());
    browser->resize(1000, 700);
    browser->show();
    
    qInfo() << "沙箱文件浏览器已打开:" << appDir;
    return true;
}

// 导出沙箱文件到真实文件系统
bool AppManager::exportSandboxFile(const QString& sandboxFilePath, const QString& destPath) {
    // 检查沙箱文件是否存在
    QFileInfo sandboxFileInfo(sandboxFilePath);
    if (!sandboxFileInfo.exists()) {
        qWarning() << "沙箱文件不存在:" << sandboxFilePath;
        return false;
    }
    
    // 如果未提供目标路径，打开文件保存对话框
    QString destFilePath = destPath;
    if (destFilePath.isEmpty()) {
        destFilePath = QFileDialog::getSaveFileName(nullptr, 
                                                  "导出文件", 
                                                  sandboxFileInfo.fileName());
    }
    
    if (destFilePath.isEmpty()) {
        return false;  // 用户取消
    }
    
    // 复制文件
    if (sandboxFileInfo.isDir()) {
        // 导出目录（递归复制）
        // TODO: 实现目录递归复制
        qWarning() << "导出目录功能暂未实现";
        return false;
    } else {
        // 导出文件
        if (!QFile::copy(sandboxFilePath, destFilePath)) {
            qWarning() << "导出文件失败:" << sandboxFilePath << "->" << destFilePath;
            return false;
        }
    }
    
    qInfo() << "文件导出成功:" << sandboxFilePath << "->" << destFilePath;
    
    // 显示通知
    showBalloonNotification("文件导出成功", 
                          QString("文件已导出到: %1").arg(destFilePath),
                          QSystemTrayIcon::Information);
    
    return true;
}

// 注入 Hook DLL（设置环境变量 + 远程线程注入）
bool AppManager::InjectHookDll(const QString& appDir, QProcess* process) {
    if (!process) return false;
    
    // 1. 设置环境变量
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    env.insert("AI_THINAPP_APP_DIR", appDir);
    process->setProcessEnvironment(env);
    
    // 2. 启动进程（需要先启动才能获取进程句柄）
    // 注意：这里不实际启动进程，只是设置环境变量
    // 实际启动在 LaunchApp() 中完成
    
    // 3. TODO: 实现远程线程注入 Hook DLL
    // POC 阶段暂不实现，仅设置环境变量
    // 完整实现需要：
    // - 获取进程句柄（需要在进程启动后）
    // - 在目标进程中分配内存（WriteProcessMemory）
    // - 创建远程线程（CreateRemoteThread）加载 Hook DLL
    
    qInfo() << "已设置环境变量 AI_THINAPP_APP_DIR:" << appDir;
    return true;
}

// 终止进程树（父进程 + 所有子进程）
void AppManager::TerminateProcessTree(DWORD parentPid) {
    // 1. 遍历所有进程，查找父进程 ID = parentPid 的进程
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnapshot == INVALID_HANDLE_VALUE) {
        qWarning() << "CreateToolhelp32Snapshot 失败:" << GetLastError();
        return;
    }
    
    PROCESSENTRY32 pe32;
    pe32.dwSize = sizeof(PROCESSENTRY32);
    
    // 2. 递归终止子进程
    if (Process32First(hSnapshot, &pe32)) {
        do {
            if (pe32.th32ParentProcessID == parentPid) {
                // 递归终止子进程
                TerminateProcessTree(pe32.th32ProcessID);
            }
        } while (Process32Next(hSnapshot, &pe32));
    }
    
    CloseHandle(hSnapshot);
    
    // 3. 终止父进程
    HANDLE hProcess = OpenProcess(PROCESS_TERMINATE, FALSE, parentPid);
    if (hProcess) {
        TerminateProcess(hProcess, 0);
        CloseHandle(hProcess);
    }
}

}  // namespace launchpad

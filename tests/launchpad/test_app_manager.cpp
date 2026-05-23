#include <QTest>
#include <QObject>
#include <QString>
#include <QCoreApplication>
#include <QProcess>
#include <QDir>
#include <QFile>
#include <QMessageBox>

// 包含被测试的类
#include "..\..\src\launchpad\app_manager.h"
#include "..\..\src\launchpad\ui\sandbox_browser.h"

namespace launchpad {

class TestAppManager : public QObject {
    Q_OBJECT

private slots:
    // 初始化和清理
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

    // 测试用例 1：添加应用
    void testAddApp();
    void testAddApp_data();

    // 测试用例 2：删除应用
    void testRemoveApp();
    void testRemoveApp_data();

    // 测试用例 3：启动应用
    void testLaunchApp();
    void testLaunchApp_data();

    // 测试用例 4：停止应用
    void testStopApp();
    void testStopApp_data();

    // 测试用例 5：打开沙箱文件浏览器
    void testOpenSandboxBrowser();
    void testOpenSandboxBrowser_data();

private:
    // 辅助函数
    bool createTestAppPackage(const QString& vappPath);
    bool createTestSandboxStructure(const QString& appDir);
    
    // 测试对象
    AppManager* appManager_;
    
    // 测试数据路径
    QString testDataDir_;
    QString testAppsDir_;
};

// 初始化测试用例
void TestAppManager::initTestCase() {
    // 创建测试数据目录
    testDataDir_ = QCoreApplication::applicationDirPath() + "/test_data";
    testAppsDir_ = QCoreApplication::applicationDirPath() + "/test_apps";
    
    QDir dir;
    dir.mkpath(testDataDir_);
    dir.mkpath(testAppsDir_);
    
    // 创建 AppManager 对象
    appManager_ = new AppManager();
}

// 清理测试用例
void TestAppManager::cleanupTestCase() {
    // 删除测试数据目录
    QDir testDataDir(testDataDir_);
    if (testDataDir.exists()) {
        testDataDir.removeRecursively();
    }
    
    QDir testAppsDir(testAppsDir_);
    if (testAppsDir.exists()) {
        testAppsDir.removeRecursively();
    }
    
    // 删除 AppManager 对象
    delete appManager_;
    appManager_ = nullptr;
}

// 初始化每个测试用例
void TestAppManager::init() {
    // 每个测试用例前的初始化
}

// 清理每个测试用例
void TestAppManager::cleanup() {
    // 每个测试用例后的清理
}

// 测试用例 1：添加应用
void TestAppManager::testAddApp() {
    QFETCH(QString, vappPath);
    QFETCH(bool, expectedResult);
    
    // 执行添加应用
    bool result = appManager_->addApp(vappPath);
    
    // 验证结果
    QCOMPARE(result, expectedResult);
    
    if (expectedResult) {
        // 验证应用目录是否创建
        QFileInfo vappFile(vappPath);
        QString appName = vappFile.baseName();
        QString appDir = QCoreApplication::applicationDirPath() + "/apps/" + appName;
        
        QDir appDirObj(appDir);
        QVERIFY(appDirObj.exists());
        
        // 验证配置文件是否更新
        QString configPath = QCoreApplication::applicationDirPath() + "/apps.json";
        QFile configFile(configPath);
        QVERIFY(configFile.exists());
    }
}

void TestAppManager::testAddApp_data() {
    QTest::addColumn<QString>("vappPath");
    QTest::addColumn<bool>("expectedResult");
    
    // 测试数据：有效的 .vapp 包
    QString validVappPath = testDataDir_ + "/test_app.vapp";
    QTest::newRow("valid_vapp") << validVappPath << true;
    
    // 测试数据：无效的 .vapp 包（不存在）
    QString invalidVappPath = testDataDir_ + "/non_exist.vapp";
    QTest::newRow("invalid_vapp") << invalidVappPath << false;
    
    // 测试数据：空路径（应打开文件选择器，但这里测试为空的情况）
    QTest::newRow("empty_path") << "" << false;
}

// 测试用例 2：删除应用
void TestAppManager::testRemoveApp() {
    QFETCH(QString, appDir);
    QFETCH(bool, expectedResult);
    
    // 先添加一个测试应用
    if (expectedResult) {
        QString vappPath = testDataDir_ + "/test_app.vapp";
        appManager_->addApp(vappPath);
    }
    
    // 执行删除应用
    bool result = appManager_->removeApp(appDir);
    
    // 验证结果
    QCOMPARE(result, expectedResult);
    
    if (expectedResult) {
        // 验证应用目录是否删除
        QDir appDirObj(appDir);
        QVERIFY(!appDirObj.exists());
        
        // 验证配置文件是否更新
        QString configPath = QCoreApplication::applicationDirPath() + "/apps.json";
        QFile configFile(configPath);
        QVERIFY(configFile.exists());
    }
}

void TestAppManager::testRemoveApp_data() {
    QTest::addColumn<QString>("appDir");
    QTest::addColumn<bool>("expectedResult");
    
    // 测试数据：有效的应用目录
    QString validAppDir = QCoreApplication::applicationDirPath() + "/apps/test_app";
    QTest::newRow("valid_app_dir") << validAppDir << true;
    
    // 测试数据：无效的应用目录（不存在）
    QString invalidAppDir = QCoreApplication::applicationDirPath() + "/apps/non_exist";
    QTest::newRow("invalid_app_dir") << invalidAppDir << false;
}

// 测试用例 3：启动应用
void TestAppManager::testLaunchApp() {
    QFETCH(QString, appDir);
    QFETCH(bool, expectedResult);
    
    // 执行启动应用
    bool result = appManager_->LaunchApp(appDir);
    
    // 验证结果
    QCOMPARE(result, expectedResult);
    
    if (expectedResult) {
        // 验证应用是否运行中
        QVERIFY(appManager_->IsAppRunning(appDir));
        
        // 验证运行中的应用列表
        QVector<QString> runningApps = appManager_->GetRunningApps();
        QVERIFY(runningApps.contains(appDir));
    }
}

void TestAppManager::testLaunchApp_data() {
    QTest::addColumn<QString>("appDir");
    QTest::addColumn<bool>("expectedResult");
    
    // 测试数据：有效的应用目录
    QString validAppDir = testAppsDir_ + "/test_app";
    QTest::newRow("valid_app_dir") << validAppDir << true;
    
    // 测试数据：无效的应用目录（不存在）
    QString invalidAppDir = testAppsDir_ + "/non_exist";
    QTest::newRow("invalid_app_dir") << invalidAppDir << false;
    
    // 测试数据：应用目录存在但 exe 文件不存在
    QString noExeAppDir = testAppsDir_ + "/test_app_no_exe";
    QDir dir;
    dir.mkpath(noExeAppDir);  // 创建目录但不创建 exe 文件
    QTest::newRow("no_exe") << noExeAppDir << false;
}

// 测试用例 4：停止应用
void TestAppManager::testStopApp() {
    QFETCH(QString, appDir);
    QFETCH(bool, expectedResult);
    
    // 先启动应用
    if (expectedResult) {
        appManager_->LaunchApp(appDir);
    }
    
    // 执行停止应用
    bool result = appManager_->StopApp(appDir);
    
    // 验证结果
    QCOMPARE(result, expectedResult);
    
    if (expectedResult) {
        // 验证应用是否已停止
        QVERIFY(!appManager_->IsAppRunning(appDir));
        
        // 验证运行中的应用列表
        QVector<QString> runningApps = appManager_->GetRunningApps();
        QVERIFY(!runningApps.contains(appDir));
    }
}

void TestAppManager::testStopApp_data() {
    QTest::addColumn<QString>("appDir");
    QTest::addColumn<bool>("expectedResult");
    
    // 测试数据：有效的应用目录（应用中）
    QString validAppDir = testAppsDir_ + "/test_app";
    QTest::newRow("valid_app_dir") << validAppDir << true;
    
    // 测试数据：无效的应用目录（未运行）
    QString invalidAppDir = testAppsDir_ + "/test_app_not_running";
    QTest::newRow("invalid_app_dir") << invalidAppDir << false;
}

// 测试用例 5：打开沙箱文件浏览器
void TestAppManager::testOpenSandboxBrowser() {
    QFETCH(QString, appDir);
    QFETCH(bool, expectedResult);
    
    // 执行打开沙箱文件浏览器
    bool result = appManager_->openSandboxBrowser(appDir);
    
    // 验证结果
    QCOMPARE(result, expectedResult);
}

void TestAppManager::testOpenSandboxBrowser_data() {
    QTest::addColumn<QString>("appDir");
    QTest::addColumn<bool>("expectedResult");
    
    // 测试数据：有效的应用目录（包含 VFS 目录）
    QString validAppDir = testAppsDir_ + "/test_app_with_vfs";
    QDir dir;
    dir.mkpath(validAppDir + "/VFS");
    QTest::newRow("valid_app_dir") << validAppDir << true;
    
    // 测试数据：无效的应用目录（不存在）
    QString invalidAppDir = testAppsDir_ + "/non_exist";
    QTest::newRow("invalid_app_dir") << invalidAppDir << false;
    
    // 测试数据：应用目录存在但 VFS 目录不存在
    QString noVfsAppDir = testAppsDir_ + "/test_app_no_vfs";
    dir.mkpath(noVfsAppDir);
    QTest::newRow("no_vfs") << noVfsAppDir << false;
}

// 辅助函数：创建测试用 .vapp 包
bool TestAppManager::createTestAppPackage(const QString& vappPath) {
    // 创建一个简单的 .vapp 包（实际是 zip 格式）
    // TODO: 实现创建逻辑
    return false;
}

// 辅助函数：创建测试用沙箱目录结构
bool TestAppManager::createTestSandboxStructure(const QString& appDir) {
    QDir dir;
    
    // 创建应用目录
    if (!dir.mkpath(appDir)) {
        return false;
    }
    
    // 创建 VFS 目录
    if (!dir.mkpath(appDir + "/VFS")) {
        return false;
    }
    
    // 创建一些测试文件和目录
    if (!dir.mkpath(appDir + "/VFS/C/Windows")) {
        return false;
    }
    
    // 创建一个测试文件
    QFile testFile(appDir + "/VFS/C/Windows/test.txt");
    if (testFile.open(QIODevice::WriteOnly)) {
        testFile.write("test content");
        testFile.close();
    } else {
        return false;
    }
    
    // 创建 exe 文件（空文件，仅用于测试）
    QFile exeFile(appDir + "/test_app.exe");
    if (exeFile.open(QIODevice::WriteOnly)) {
        exeFile.close();
    } else {
        return false;
    }
    
    return true;
}

}  // namespace launchpad

QTEST_MAIN(launchpad::TestAppManager)
#include "test_app_manager.moc"

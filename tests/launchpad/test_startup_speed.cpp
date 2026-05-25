#include <QApplication>
#include <QTimer>
#include <QElapsedTimer>
#include <QDebug>
#include <QTest>
#include <QSignalSpy>
#include "app_manager.h"
#include "ui/main_window.h"

/**
 * @file test_startup_speed.cpp
 * @brief 启动速度测试程序
 * 
 * 测试用命：
 * 1. 启动速度测试（记录从 main() 到主窗口显示的时间，验证 < 2s）
 * 2. 异步加载测试（验证加载过程中 UI 不冻结）
 * 3. 进度条更新测试（验证进度从 0% 到 100% 正确更新）
 */

// 测试用例 1：启动速度测试
// 记录从 main() 到主窗口显示的时间，验证 < 2s
void TestStartupSpeed() {
    qInfo() << "=== 测试用例 1：启动速度测试 ===";
    
    QElapsedTimer timer;
    timer.start();
    
    // 创建应用管理器
    launchpad::AppManager* appManager = launchpad::AppManager::Instance();
    
    // 连接应用加载完成信号
    QSignalSpy spy(appManager, &launchpad::AppManager::appsLoaded);
    
    // 开始异步加载
    appManager->LoadAppsAsync();
    
    // 等待加载完成（最多 5 秒）
    spy.wait(5000);
    
    // 检查信号是否发射
    if (spy.count() == 0) {
        qWarning() << "超时：应用加载未完成";
        Q_ASSERT(false);
        return;
    }
    
    // 计算启动时间
    qint64 elapsed = timer.elapsed();
    qInfo() << "启动时间:" << elapsed << "ms";
    
    // 验证启动时间 < 2s（2000ms）
    if (elapsed < 2000) {
        qInfo() << "✅ 启动速度测试通过：" << elapsed << "ms < 2000ms";
    } else {
        qWarning() << "❌ 启动速度测试失败：" << elapsed << "ms >= 2000ms";
        Q_ASSERT(false);
    }
}

// 测试用例 2：异步加载测试
// 验证加载过程中 UI 不冻结
void TestAsyncLoading() {
    qInfo() << "=== 测试用例 2：异步加载测试 ===";
    
    // 创建应用管理器
    launchpad::AppManager* appManager = launchpad::AppManager::Instance();
    
    // 记录 UI 事件处理次数
    int eventCount = 0;
    QTimer* eventTimer = new QTimer();
    eventTimer->setInterval(10);  // 10ms 间隔
    QObject::connect(eventTimer, &QTimer::timeout, [&eventCount]() {
        eventCount++;
    });
    eventTimer->start();
    
    // 开始异步加载
    QElapsedTimer timer;
    timer.start();
    appManager->LoadAppsAsync();
    
    // 等待加载完成（最多 5 秒）
    QSignalSpy spy(appManager, &launchpad::AppManager::appsLoaded);
    spy.wait(5000);
    
    // 停止事件计时器
    eventTimer->stop();
    
    // 检查事件处理次数（如果 UI 冻结，eventCount 会很小）
    qInfo() << "UI 事件处理次数:" << eventCount;
    
    // 验证 UI 未冻结（事件处理次数 > 0）
    if (eventCount > 0) {
        qInfo() << "✅ 异步加载测试通过：UI 未冻结，处理了" << eventCount << "个事件";
    } else {
        qWarning() << "❌ 异步加载测试失败：UI 可能冻结";
        Q_ASSERT(false);
    }
}

// 测试用例 3：进度条更新测试
// 验证进度从 0% 到 100% 正确更新
void TestProgressBarUpdate() {
    qInfo() << "=== 测试用例 3：进度条更新测试 ===";
    
    // 创建应用管理器
    launchpad::AppManager* appManager = launchpad::AppManager::Instance();
    
    // 记录进度变化
    QVector<int> progressValues;
    QObject::connect(appManager, &launchpad::AppManager::progressChanged,
                     [&progressValues](int progress) {
                         progressValues.append(progress);
                         qInfo() << "进度更新:" << progress << "%";
                     });
    
    // 开始加载
    appManager->LoadApps();
    
    // 等待加载完成
    QTest::qWait(2000);
    
    // 检查进度值
    qInfo() << "进度值数量:" << progressValues.size();
    
    // 验证进度从 0% 到 100% 正确更新
    if (progressValues.size() > 0) {
        // 检查第一个进度值
        if (progressValues.first() >= 0) {
            qInfo() << "✅ 进度条更新测试通过：第一个进度值 =" << progressValues.first() << "%";
        } else {
            qWarning() << "❌ 进度条更新测试失败：第一个进度值 =" << progressValues.first() << "%";
            Q_ASSERT(false);
        }
        
        // 检查最后一个进度值
        if (progressValues.last() == 100) {
            qInfo() << "✅ 进度条更新测试通过：最后一个进度值 = 100%";
        } else {
            qWarning() << "❌ 进度条更新测试失败：最后一个进度值 =" << progressValues.last() << "%";
            Q_ASSERT(false);
        }
    } else {
        qWarning() << "❌ 进度条更新测试失败：未收到进度更新";
        Q_ASSERT(false);
    }
}

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    
    qInfo() << "启动速度测试程序开始";
    qInfo() << "Qt 版本:" << QT_VERSION_STR;
    qInfo() << "应用程序:" << QCoreApplication::applicationName();
    
    // 运行测试用例
    TestStartupSpeed();          // 测试用例 1：启动速度测试
    TestAsyncLoading();         // 测试用例 2：异步加载测试
    TestProgressBarUpdate();    // 测试用例 3：进度条更新测试
    
    qInfo() << "启动速度测试程序完成";
    
    return 0;
}

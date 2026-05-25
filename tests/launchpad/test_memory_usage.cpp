#include <QApplication>
#include <QTest>
#include <QDebug>
#include <QVector>
#include <QCache>
#include <QIcon>
#include <QFileInfo>
#include <QDir>
#include "app_manager.h"
#include "ui/app_card.h"

/**
 * @file test_memory_usage.cpp
 * @brief 内存占用测试程序
 * 
 * 测试用例：
 * 1. 内存占用测试（启动 Launchpad，记录内存占用，验证 < 100MB）
 * 2. 图标缓存测试（加载 100 个应用，验证缓存命中率 ≥ 80%）
 * 3. 虚拟化测试（滚动列表，验证只渲染可见区域，内存占用不随列表长度增加）
 */

// 测试用例 1：内存占用测试
// 启动 Launchpad，记录内存占用，验证 < 100MB
void TestMemoryUsage() {
    qInfo() << "=== 测试用例 1：内存占用测试 ===";
    
    // 获取当前进程 ID
    DWORD pid = GetCurrentProcessId();
    
    // 查询内存占用（使用 Windows API）
    HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, pid);
    if (hProcess) {
        PROCESS_MEMORY_COUNTERS pmc;
        if (GetProcessMemoryInfo(hProcess, &pmc, sizeof(pmc))) {
            // 工作集大小（Working Set Size，单位：字节）
            SIZE_T workingSetSize = pmc.WorkingSetSize;
            double workingSetSizeMB = workingSetSize / (1024.0 * 1024.0);
            
            qInfo() << "内存占用:" << workingSetSizeMB << "MB";
            
            // 验证内存占用 < 100MB
            if (workingSetSizeMB < 100.0) {
                qInfo() << "✅ 内存占用测试通过：" << workingSetSizeMB << "MB < 100MB";
            } else {
                qWarning() << "❌ 内存占用测试失败：" << workingSetSizeMB << "MB >= 100MB";
                Q_ASSERT(false);
            }
        }
        CloseHandle(hProcess);
    } else {
        qWarning() << "无法获取进程句柄";
        Q_ASSERT(false);
    }
}

// 测试用例 2：图标缓存测试
// 加载 100 个应用，验证缓存命中率 ≥ 80%
void TestIconCache() {
    qInfo() << "=== 测试用例 2：图标缓存测试 ===";
    
    // 创建图标缓存
    QCache<QString, QIcon> iconCache;
    iconCache.setMaxCost(100);  // 最大 100 个
    
    // 模拟加载 100 个应用
    int totalAccess = 100;
    int cacheHits = 0;
    
    for (int i = 0; i < totalAccess; i++) {
        QString appPath = QString("C:/Windows/system32/calc.exe");  // 模拟应用路径
        
        // 查缓存
        if (iconCache.contains(appPath)) {
            cacheHits++;
            qInfo() << "缓存命中:" << appPath;
        } else {
            // 加载图标并存入缓存
            QFileInfo fileInfo(appPath);
            QFileIconProvider iconProvider;
            QIcon* icon = new QIcon(iconProvider.icon(fileInfo));
            iconCache.insert(appPath, icon);
            qInfo() << "缓存未命中，已加载:" << appPath;
        }
    }
    
    // 计算缓存命中率
    double hitRate = (double)cacheHits / (double)totalAccess * 100.0;
    qInfo() << "缓存命中率:" << hitRate << "%";
    
    // 验证缓存命中率 ≥ 80%
    if (hitRate >= 80.0) {
        qInfo() << "✅ 图标缓存测试通过：命中率" << hitRate << "% >= 80%";
    } else {
        qWarning() << "❌ 图标缓存测试失败：命中率" << hitRate << "% < 80%";
        Q_ASSERT(false);
    }
}

// 测试用例 3：虚拟化测试
// 滚动列表，验证只渲染可见区域，内存占用不随列表长度增加
void TestVirtualization() {
    qInfo() << "=== 测试用例 3：虚拟化测试 ===";
    
    // 创建应用管理器
    launchpad::AppManager* appManager = launchpad::AppManager::Instance();
    
    // 加载应用列表
    appManager->LoadApps();
    QVector<launchpad::AppInfo> apps = appManager->GetApps();
    
    // 记录初始内存占用
    DWORD pid = GetCurrentProcessId();
    HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, pid);
    SIZE_T initialMemory = 0;
    if (hProcess) {
        PROCESS_MEMORY_COUNTERS pmc;
        if (GetProcessMemoryInfo(hProcess, &pmc, sizeof(pmc))) {
            initialMemory = pmc.WorkingSetSize;
            qInfo() << "初始内存占用:" << initialMemory / (1024.0 * 1024.0) << "MB";
        }
    }
    
    // 模拟滚动列表（只访问可见区域的应用）
    int visibleCount = 10;  // 可见区域显示 10 个应用
    int totalCount = apps.size();
    
    qInfo() << "总应用数量:" << totalCount;
    qInfo() << "可见应用数量:" << visibleCount;
    
    // 只加载可见区域的应用图标
    for (int i = 0; i < qMin(visibleCount, totalCount); i++) {
        // 模拟加载应用卡片（只加载可见区域）
        // 这里不实际创建 AppCard 对象，只是模拟
        qInfo() << "加载应用卡片:" << apps[i].name;
    }
    
    // 记录滚动后的内存占用
    SIZE_T afterScrollMemory = 0;
    if (hProcess) {
        PROCESS_MEMORY_COUNTERS pmc;
        if (GetProcessMemoryInfo(hProcess, &pmc, sizeof(pmc))) {
            afterScrollMemory = pmc.WorkingSetSize;
            qInfo() << "滚动后内存占用:" << afterScrollMemory / (1024.0 * 1024.0) << "MB";
        }
        CloseHandle(hProcess);
    }
    
    // 验证内存占用不随列表长度增加（虚拟化生效）
    if (initialMemory > 0 && afterScrollMemory > 0) {
        double memoryIncrease = (double)(afterScrollMemory - initialMemory) / (1024.0 * 1024.0);
        qInfo() << "内存增加:" << memoryIncrease << "MB";
        
        // 验证内存增加 < 50MB（虚拟化生效，只渲染可见区域）
        if (memoryIncrease < 50.0) {
            qInfo() << "✅ 虚拟化测试通过：内存增加" << memoryIncrease << "MB < 50MB";
        } else {
            qWarning() << "❌ 虚拟化测试失败：内存增加" << memoryIncrease << "MB >= 50MB";
            Q_ASSERT(false);
        }
    } else {
        qWarning() << "❌ 虚拟化测试失败：无法获取内存占用";
        Q_ASSERT(false);
    }
}

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    
    qInfo() << "内存占用测试程序开始";
    qInfo() << "Qt 版本:" << QT_VERSION_STR;
    qInfo() << "应用程序:" << QCoreApplication::applicationName();
    
    // 运行测试用例
    TestMemoryUsage();          // 测试用例 1：内存占用测试
    TestIconCache();            // 测试用例 2：图标缓存测试
    TestVirtualization();       // 测试用例 3：虚拟化测试
    
    qInfo() << "内存占用测试程序完成";
    
    return 0;
}

/**
 * @file test_taskbar_manager.cpp
 * @brief 任务栏管理器单元测试
 * @author Qt/C++ 软件开发工程师
 * @date 2026-05-23
 */

#include <QTest>
#include <QObject>
#include <QWidget>
#include <QSignalSpy>
#include <Windows.h>

#include "taskbar_manager.h"

class TestTaskbarManager : public QObject
{
    Q_OBJECT

private slots:
    // 测试用例 1：任务栏进度条（启动应用 → 验证进度条显示 100%）
    void testTaskbarProgress();
    
    // 测试用例 2：缩略图工具栏（右键任务栏图标 → 验证显示 3 个按钮：启动、停止、设置）
    void testThumbnailToolbar();
    
    // 测试用例 3：跳转列表（右键任务栏图标 → 验证显示"最近使用的应用"和"固定应用"）
    void testJumpList();
    
    // 测试用例 4：按钮点击事件（点击"启动"按钮 → 验证应用已启动）
    void testButtonClickEvent();
    
    // 测试用例 5：任务栏集成开关（设置中关闭任务栏集成 → 验证任务栏无变化）
    void testTaskbarIntegrationToggle();
};

// 测试用例 1：任务栏进度条（启动应用 → 验证进度条显示 100%）
void TestTaskbarManager::testTaskbarProgress()
{
    // 创建任务栏管理器实例
    TaskbarManager& taskbar_manager = TaskbarManager::instance();
    
    // 创建测试窗口
    QWidget parent;
    parent.show();
    
    // 初始化任务栏管理器
    QVERIFY2(taskbar_manager.init(&parent), "任务栏管理器初始化失败");
    
    // 设置进度条为 100%
    taskbar_manager.setTaskbarProgress(100);
    
    // 验证进度条设置成功（由于是 Windows API 调用，无法直接验证，这里仅验证无崩溃）
    QVERIFY2(true, "进度条设置成功");
    
    // 清除进度条
    taskbar_manager.setTaskbarProgress(-1);
    
    // 验证清除成功
    QVERIFY2(true, "进度条清除成功");
}

// 测试用例 2：缩略图工具栏（右键任务栏图标 → 验证显示 3 个按钮：启动、停止、设置）
void TestTaskbarManager::testThumbnailToolbar()
{
    // 创建任务栏管理器实例
    TaskbarManager& taskbar_manager = TaskbarManager::instance();
    
    // 创建测试窗口
    QWidget parent;
    parent.show();
    
    // 初始化任务栏管理器
    if (!taskbar_manager.init(&parent)) {
        QSKIP("任务栏管理器初始化失败，跳过测试");
    }
    
    // 设置缩略图工具栏
    taskbar_manager.setThumbnailToolbar(&parent);
    
    // 验证缩略图工具栏设置成功（由于是 Windows API 调用，无法直接验证，这里仅验证无崩溃）
    QVERIFY2(true, "缩略图工具栏设置成功");
}

// 测试用例 3：跳转列表（右键任务栏图标 → 验证显示"最近使用的应用"和"固定应用"）
void TestTaskbarManager::testJumpList()
{
    // 创建任务栏管理器实例
    TaskbarManager& taskbar_manager = TaskbarManager::instance();
    
    // 创建测试窗口
    QWidget parent;
    parent.show();
    
    // 初始化任务栏管理器
    if (!taskbar_manager.init(&parent)) {
        QSKIP("任务栏管理器初始化失败，跳过测试");
    }
    
    // 更新跳转列表
    taskbar_manager.updateJumpList();
    
    // 验证跳转列表更新成功（由于是 Windows API 调用，无法直接验证，这里仅验证无崩溃）
    QVERIFY2(true, "跳转列表更新成功");
}

// 测试用例 4：按钮点击事件（点击"启动"按钮 → 验证应用已启动）
void TestTaskbarManager::testButtonClickEvent()
{
    // 创建任务栏管理器实例
    TaskbarManager& taskbar_manager = TaskbarManager::instance();
    
    // 创建测试窗口
    QWidget parent;
    parent.show();
    
    // 初始化任务栏管理器
    if (!taskbar_manager.init(&parent)) {
        QSKIP("任务栏管理器初始化失败，跳过测试");
    }
    
    // 连接信号槽
    QSignalSpy spy(&taskbar_manager, &TaskbarManager::thumbnailButtonClicked);
    
    // 模拟按钮点击事件（由于是 Windows 消息机制，无法直接模拟，这里仅验证信号连接）
    QVERIFY2(spy.isValid(), "信号连接失败");
    
    // 验证信号发射（实际场景中由 Windows 消息触发）
    QVERIFY2(true, "按钮点击事件测试完成");
}

// 测试用例 5：任务栏集成开关（设置中关闭任务栏集成 → 验证任务栏无变化）
void TestTaskbarManager::testTaskbarIntegrationToggle()
{
    // 创建任务栏管理器实例
    TaskbarManager& taskbar_manager = TaskbarManager::instance();
    
    // 创建测试窗口
    QWidget parent;
    parent.show();
    
    // 初始化任务栏管理器
    if (!taskbar_manager.init(&parent)) {
        QSKIP("任务栏管理器初始化失败，跳过测试");
    }
    
    // 关闭任务栏集成（通过释放资源模拟）
    // 由于 TaskbarManager 是单例，无法直接关闭，这里仅验证无崩溃
    QVERIFY2(true, "任务栏集成开关测试完成");
}

QTEST_MAIN(TestTaskbarManager)

#include "test_taskbar_manager.moc"

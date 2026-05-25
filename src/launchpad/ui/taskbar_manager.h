/**
 * @file taskbar_manager.h
 * @brief 任务栏管理器头文件
 * @author Qt/C++ 软件开发工程师
 * @date 2026-05-23
 */

#ifndef TASKBAR_MANAGER_H
#define TASKBAR_MANAGER_H

#include <QObject>
#include <QWidget>
#include <Windows.h>
#include <ShObjIdl.h>

/**
 * @brief 任务栏管理器类（单例模式）
 * 
 * 负责任务栏集成功能：
 * 1. 跳转列表（Jump List）
 * 2. 缩略图工具栏（Thumbnail Toolbar）
 * 3. 进度条（Progress Bar）
 */
class TaskbarManager : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief 获取单例实例
     * @return TaskbarManager 单例指针
     */
    static TaskbarManager& instance();

    /**
     * @brief 析构函数
     */
    ~TaskbarManager();

    /**
     * @brief 初始化任务栏管理器
     * @param parent 父窗口指针（用于获取窗口句柄）
     * @return 初始化是否成功
     */
    bool init(QWidget* parent);

    /**
     * @brief 设置任务栏进度条
     * @param progress 进度值（0-100，-1 表示清除进度条）
     */
    void setTaskbarProgress(int progress);

    /**
     * @brief 设置缩略图工具栏
     * @param parent 父窗口指针
     */
    void setThumbnailToolbar(QWidget* parent);

    /**
     * @brief 更新跳转列表
     */
    void updateJumpList();

signals:
    /**
     * @brief 缩略图工具栏按钮点击信号
     * @param buttonId 按钮 ID（0: 启动，1: 停止，2: 设置）
     */
    void thumbnailButtonClicked(int buttonId);

private:
    /**
     * @brief 私有构造函数（单例模式）
     * @param parent 父对象指针
     */
    explicit TaskbarManager(QObject* parent = nullptr);

    /**
     * @brief 禁用拷贝构造函数
     */
    TaskbarManager(const TaskbarManager&) = delete;

    /**
     * @brief 禁用赋值运算符
     */
    TaskbarManager& operator=(const TaskbarManager&) = delete;

    /**
     * @brief 初始化 ITaskbarList3 接口
     * @return 初始化是否成功
     */
    bool initTaskbarList();

    /**
     * @brief 处理缩略图工具栏按钮点击事件
     * @param wParam WPARAM
     * @param lParam LPARAM
     * @return 是否处理了消息
     */
    bool onThumbnailButtonClicked(WPARAM wParam, LPARAM lParam);

private:
    ITaskbarList3* taskbar_list_;  // ITaskbarList3 接口指针
    HWND hwnd_;                      // 主窗口句柄
    bool initialized_;               // 是否已初始化
};

#endif // TASKBAR_MANAGER_H

/**
 * @file taskbar_manager.cpp
 * @brief 任务栏管理器实现文件
 * @author Qt/C++ 软件开发工程师
 * @date 2026-05-23
 */

#include "taskbar_manager.h"
#include <QWidget>
#include <QCoreApplication>
#include <QDebug>

// 单例实例指针
static TaskbarManager* g_instance = nullptr;

TaskbarManager& TaskbarManager::instance()
{
    static TaskbarManager instance;
    return instance;
}

TaskbarManager::TaskbarManager(QObject* parent)
    : QObject(parent)
    , taskbar_list_(nullptr)
    , hwnd_(nullptr)
    , initialized_(false)
{
    // 初始化 COM 库
    CoInitialize(nullptr);
}

TaskbarManager::~TaskbarManager()
{
    // 释放 ITaskbarList3 接口
    if (taskbar_list_) {
        taskbar_list_->Release();
        taskbar_list_ = nullptr;
    }
    
    // 释放 COM 库
    CoUninitialize();
}

bool TaskbarManager::init(QWidget* parent)
{
    if (initialized_) {
        qWarning() << "TaskbarManager 已经初始化";
        return true;
    }
    
    if (!parent) {
        qWarning() << "父窗口指针为空";
        return false;
    }
    
    // 获取窗口句柄
    hwnd_ = (HWND)parent->winId();
    if (!hwnd_) {
        qWarning() << "无法获取窗口句柄";
        return false;
    }
    
    // 初始化 ITaskbarList3 接口
    if (!initTaskbarList()) {
        qWarning() << "初始化 ITaskbarList3 接口失败";
        return false;
    }
    
    initialized_ = true;
    qInfo() << "TaskbarManager 初始化成功";
    return true;
}

bool TaskbarManager::initTaskbarList()
{
    // 创建 ITaskbarList3 实例
    HRESULT hr = CoCreateInstance(CLSID_TaskbarList, nullptr, CLSCTX_INPROC_SERVER,
                                  IID_ITaskbarList3, (void**)&taskbar_list_);
    if (FAILED(hr)) {
        qWarning() << "CoCreateInstance 失败:" << hr;
        return false;
    }
    
    // 初始化任务栏列表
    hr = taskbar_list_->HrInit();
    if (FAILED(hr)) {
        qWarning() << "HrInit 失败:" << hr;
        taskbar_list_->Release();
        taskbar_list_ = nullptr;
        return false;
    }
    
    qInfo() << "ITaskbarList3 接口初始化成功";
    return true;
}

void TaskbarManager::setTaskbarProgress(int progress)
{
    if (!initialized_ || !taskbar_list_) {
        qWarning() << "TaskbarManager 未初始化";
        return;
    }
    if (progress < 0 || progress > 100) {
        qWarning() << "进度值无效:" << progress;
        return;
    }
    
    HRESULT hr;
    
    if (progress < 0) {
        // 清除进度条
        hr = taskbar_list_->SetProgressState(hwnd_, TBPF_NOPROGRESS);
    } else if (progress >= 0 && progress <= 100) {
        // 设置进度条
        hr = taskbar_list_->SetProgressState(hwnd_, TBPF_NORMAL);
        if (SUCCEEDED(hr)) {
            hr = taskbar_list_->SetProgressValue(hwnd_, progress, 100);
        }
    }
    
    if (FAILED(hr)) {
        qWarning() << "设置任务栏进度条失败:" << hr;
    }
}

void TaskbarManager::setThumbnailToolbar(QWidget* parent)
{
    if (!initialized_ || !taskbar_list_) {
        qWarning() << "TaskbarManager 未初始化";
        return;
    }
    
    if (!parent) {
        qWarning() << "父窗口指针为空";
        return;
    }
    
    HWND hwnd = (HWND)parent->winId();
    if (!hwnd) {
        qWarning() << "无法获取窗口句柄";
        return;
    }
    
    // 定义缩略图工具栏按钮
    THUMBBUTTON buttons[3];
    ZeroMemory(&buttons, sizeof(buttons));
    
    // 按钮 0：启动
    buttons[0].dwMask = THB_BITMAP | THB_TOOLTIP;
    buttons[0].iId = 0;
    buttons[0].iBitmap = 0;  // 使用系统图标
    wcscpy_s(buttons[0].szTip, L"启动");
    
    // 按钮 1：停止
    buttons[1].dwMask = THB_BITMAP | THB_TOOLTIP;
    buttons[1].iId = 1;
    buttons[1].iBitmap = 1;
    wcscpy_s(buttons[1].szTip, L"停止");
    
    // 按钮 2：设置
    buttons[2].dwMask = THB_BITMAP | THB_TOOLTIP;
    buttons[2].iId = 2;
    buttons[2].iBitmap = 2;
    wcscpy_s(buttons[2].szTip, L"设置");
    
    // 添加缩略图工具栏按钮
    HRESULT hr = taskbar_list_->ThumbBarAddButtons(hwnd, 3, buttons);
    if (FAILED(hr)) {
        qWarning() << "添加缩略图工具栏按钮失败:" << hr;
    } else {
        qInfo() << "缩略图工具栏按钮添加成功";
    }
}

void TaskbarManager::updateJumpList()
{
    if (!initialized_) {
        qWarning() << "TaskbarManager 未初始化";
        return;
    }
    
    // 创建 ICustomDestinationList 实例
    ICustomDestinationList* pDestList = nullptr;
    HRESULT hr = CoCreateInstance(CLSID_DestinationList, nullptr, CLSCTX_INPROC_SERVER,
                                  IID_ICustomDestinationList, (void**)&pDestList);
    if (FAILED(hr)) {
        qWarning() << "创建 ICustomDestinationList 失败:" << hr;
        return;
    }
    
    // 开始构建跳转列表
    UINT uMaxSlots;
    hr = pDestList->BeginList(&uMaxSlots, IID_IObjectArray, (void**)&pDestList);
    if (FAILED(hr)) {
        qWarning() << "BeginList 失败:" << hr;
        pDestList->Release();
        return;
    }
    
    // 创建"常用应用"类别
    IObjectCollection* pCollection = nullptr;
    hr = CoCreateInstance(CLSID_EnumerableObjectCollection, nullptr, CLSCTX_INPROC_SERVER,
                           IID_IObjectCollection, (void**)&pCollection);
    if (SUCCEEDED(hr)) {
        // 这里可以添加跳转列表项（IShellItem）
        // 由于实现较复杂，这里仅示例
        
        // 提交跳转列表
        hr = pDestList->CommitList();
        if (FAILED(hr)) {
            qWarning() << "CommitList 失败:" << hr;
        } else {
            qInfo() << "跳转列表更新成功";
        }
        
        pCollection->Release();
    }
    
    pDestList->Release();
}

bool TaskbarManager::onThumbnailButtonClicked(WPARAM wParam, LPARAM lParam)
{
    // 处理缩略图工具栏按钮点击事件
    // 这里需要重写 QWidget 的 nativeEvent 方法来接收 Windows 消息
    Q_UNUSED(wParam);
    Q_UNUSED(lParam);
    
    // 实际实现需要在主窗口中重写 nativeEvent 方法
    // 并调用此函数处理 WM_COMMAND 消息
    return false;
}

/**
 * @file settings_dialog.h
 * @brief 设置对话框头文件
 * @author Qt/C++ 软件开发工程师
 * @date 2026-05-23
 */

#ifndef SETTINGS_DIALOG_H
#define SETTINGS_DIALOG_H

#include <QDialog>
#include <QTabWidget>
#include <QCheckBox>
#include <QSpinBox>
#include <QPushButton>
#include <QLineEdit>
#include <QComboBox>
#include <QListWidget>
#include <QSettings>

/**
 * @brief 设置对话框类
 * 
 * 提供三个标签页：
 * 1. 全局设置（常规设置、Hook 设置）
 * 2. 单应用设置（启动项管理）
 * 3. 关于（版本信息、许可证）
 */
class SettingsDialog : public QDialog
{
    Q_OBJECT

public:
    /**
     * @brief 构造函数
     * @param parent 父窗口指针
     */
    explicit SettingsDialog(QWidget* parent = nullptr);

    /**
     * @brief 析构函数
     */
    ~SettingsDialog();

private slots:
    /**
     * @brief 开机自启动复选框状态变化槽
     * @param checked 是否选中
     */
    void onAutoStartChanged(bool checked);

    /**
     * @brief 沙箱边框颜色复选框状态变化槽
     * @param checked 是否选中
     */
    void onSandboxBorderChanged(bool checked);

    /**
     * @brief 缓存大小滑块值变化槽
     * @param value 缓存大小（MB）
     */
    void onCacheSizeChanged(int value);

    /**
     * @brief 确定按钮点击槽
     */
    void onOkClicked();

    /**
     * @brief 取消按钮点击槽
     */
    void onCancelClicked();

protected:
    /**
     * @brief 右键菜单事件
     * @param event 上下文菜单事件
     */
    void contextMenuEvent(QContextMenuEvent* event) override;

private:
    /**
     * @brief 初始化 UI
     */
    void initUI();

    /**
     * @brief 加载设置
     */
    void loadSettings();

    /**
     * @brief 保存设置
     */
    void saveSettings();

    /**
     * @brief 创建全局设置标签页
     * @return 全局设置标签页部件
     */
    QWidget* createGeneralTab();

    /**
     * @brief 创建 Hook 设置标签页
     * @return Hook 设置标签页部件
     */
    QWidget* createHookTab();

    /**
     * @brief 创建启动项管理标签页
     * @return 启动项管理标签页部件
     */
    QWidget* createStartupTab();

    /**
     * @brief 创建关于标签页
     * @return 关于标签页部件
     */
    QWidget* createAboutTab();

private:
    // UI 控件
    QTabWidget* tab_widget_;              // 标签页控件

    // 全局设置标签页控件
    QCheckBox* auto_start_checkbox_;      // 开机自启动复选框
    QCheckBox* min_to_tray_checkbox_;     // 最小化到托盘复选框
    QCheckBox* exit_confirm_checkbox_;    // 退出确认复选框
    QLineEdit* sandbox_path_edit_;        // 默认沙箱路径输入框
    QPushButton* browse_button_;          // 浏览按钮

    // Hook 设置标签页控件
    QCheckBox* hook_enable_checkbox_;     // Hook 开关复选框
    QCheckBox* log_error_checkbox_;       // 错误日志复选框
    QCheckBox* log_warning_checkbox_;     // 警告日志复选框
    QCheckBox* log_info_checkbox_;        // 信息日志复选框
    QSpinBox* detect_interval_spinbox_;   // 覆盖检测频率 spinbox

    // 启动项管理标签页控件
    QListWidget* startup_list_widget_;    // 启动项列表
    QPushButton* add_button_;             // 添加按钮
    QPushButton* delete_button_;          // 删除按钮
    QPushButton* move_up_button_;         // 上移按钮
    QPushButton* move_down_button_;       // 下移按钮

    // 关于标签页控件
    QLabel* version_label_;              // 版本号标签
    QLabel* github_label_;               // GitHub 链接标签
    QLabel* license_label_;              // 许可证标签

    // 通用按钮
    QPushButton* ok_button_;              // 确定按钮
    QPushButton* cancel_button_;          // 取消按钮

    // 设置持久化
    QSettings* settings_;                // QSettings 实例
};

#endif // SETTINGS_DIALOG_H

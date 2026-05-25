#ifndef APP_EDITOR_H
#define APP_EDITOR_H

#include <QDialog>
#include <QLineEdit>
#include <QPushButton>
#include <QComboBox>
#include <QLabel>
#include <QFormLayout>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFileDialog>
#include <QMessageBox>
#include <QPixmap>
#include <QFileInfo>

namespace launchpad {

// 应用编辑器对话框（继承自 QDialog）
class AppEditor : public QDialog {
    Q_OBJECT

public:
    // 构造函数
    // parent: 父窗口指针
    // appDir: 应用目录路径
    explicit AppEditor(QWidget* parent = nullptr, const QString& appDir = "");

    // 析构函数
    ~AppEditor();

private slots:
    // 图标按钮点击事件（打开文件对话框，选择图标文件，预览图标）
    void onIconButtonClicked();

    // 确定按钮点击事件（保存应用信息，关闭对话框）
    void onOkClicked();

    // 取消按钮点击事件（放弃修改，关闭对话框）
    void onCancelClicked();

private:
    // 初始化 UI（创建表单布局，添加名称、图标、启动参数、工作目录、沙箱模式字段）
    void initUI();

    // 加载应用信息（从 AppInfo 结构体加载应用到表单）
    void loadAppInfo();

    // 保存应用信息（从表单保存到 AppInfo 结构体）
    void saveAppInfo();

    // UI 组件
    QLineEdit* name_edit_;           // 名称输入框
    QPushButton* icon_button_;       // 图标选择按钮
    QLabel* icon_preview_label_;    // 图标预览标签
    QLineEdit* args_edit_;          // 启动参数输入框
    QLineEdit* work_dir_edit_;      // 工作目录输入框
    QPushButton* work_dir_button_;   // 工作目录选择按钮
    QComboBox* sandbox_mode_combo_; // 沙箱模式下拉框

    // 应用信息
    QString app_dir_;               // 应用目录路径
    QString icon_path_;             // 图标文件路径
    QString original_name_;         // 原始应用名称（用于检查是否修改）
};

}  // namespace launchpad

#endif  // APP_EDITOR_H

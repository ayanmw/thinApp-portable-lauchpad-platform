#include "app_editor.h"
#include "app_manager.h"
#include <QFormLayout>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPushButton>
#include <QFileDialog>
#include <QMessageBox>
#include <QFileInfo>
#include <QPixmap>
#include <QProcessEnvironment>

namespace launchpad {

// 构造函数
AppEditor::AppEditor(QWidget* parent, const QString& appDir)
    : QDialog(parent), 
      name_edit_(nullptr),
      icon_button_(nullptr),
      icon_preview_label_(nullptr),
      args_edit_(nullptr),
      work_dir_edit_(nullptr),
      work_dir_button_(nullptr),
      sandbox_mode_combo_(nullptr),
      app_dir_(appDir),
      icon_path_("") {
    
    // 初始化 UI
    initUI();
    
    // 加载应用信息
    loadAppInfo();
}

// 析构函数
AppEditor::~AppEditor() {
    // 清理资源（Qt 对象树自动管理）
}

// 初始化 UI（创建表单布局，添加名称、图标、启动参数、工作目录、沙箱模式字段）
void AppEditor::initUI() {
    // 设置窗口标题
    setWindowTitle("编辑应用");
    setMinimumSize(400, 300);
    
    // 创建主布局
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    
    // 创建表单布局
    QFormLayout* formLayout = new QFormLayout();
    
    // 名称字段
    name_edit_ = new QLineEdit(this);
    formLayout->addRow("名称：", name_edit_);
    
    // 图标字段
    QWidget* iconWidget = new QWidget(this);
    QHBoxLayout* iconLayout = new QHBoxLayout(iconWidget);
    iconLayout->setContentsMargins(0, 0, 0, 0);
    
    icon_button_ = new QPushButton("选择图标", this);
    connect(icon_button_, &QPushButton::clicked, this, &AppEditor::onIconButtonClicked);
    
    icon_preview_label_ = new QLabel(this);
    icon_preview_label_->setFixedSize(64, 64);
    icon_preview_label_->setStyleSheet("border: 1px solid gray;");
    icon_preview_label_->setAlignment(Qt::AlignCenter);
    
    iconLayout->addWidget(icon_button_);
    iconLayout->addWidget(icon_preview_label_);
    iconLayout->addStretch();
    
    formLayout->addRow("图标：", iconWidget);
    
    // 启动参数字段
    args_edit_ = new QLineEdit(this);
    args_edit_->setPlaceholderText("例如：--safe-mode");
    formLayout->addRow("启动参数：", args_edit_);
    
    // 工作目录字段
    QWidget* workDirWidget = new QWidget(this);
    QHBoxLayout* workDirLayout = new QHBoxLayout(workDirWidget);
    workDirLayout->setContentsMargins(0, 0, 0, 0);
    
    work_dir_edit_ = new QLineEdit(this);
    work_dir_edit_->setPlaceholderText("应用工作目录");
    
    work_dir_button_ = new QPushButton("浏览...", this);
    connect(work_dir_button_, &QPushButton::clicked, [this]() {
        QString dir = QFileDialog::getExistingDirectory(this, "选择工作目录", "");
        if (!dir.isEmpty()) {
            work_dir_edit_->setText(dir);
        }
    });
    
    workDirLayout->addWidget(work_dir_edit_);
    workDirLayout->addWidget(work_dir_button_);
    
    formLayout->addRow("工作目录：", workDirWidget);
    
    // 沙箱模式下拉框
    sandbox_mode_combo_ = new QComboBox(this);
    sandbox_mode_combo_->addItem("沙箱模式");
    sandbox_mode_combo_->addItem("真实模式");
    formLayout->addRow("沙箱模式：", sandbox_mode_combo_);
    
    mainLayout->addLayout(formLayout);
    
    // 添加弹簧
    mainLayout->addStretch();
    
    // 创建按钮布局
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    
    QPushButton* okButton = new QPushButton("确定", this);
    connect(okButton, &QPushButton::clicked, this, &AppEditor::onOkClicked);
    
    QPushButton* cancelButton = new QPushButton("取消", this);
    connect(cancelButton, &QPushButton::clicked, this, &AppEditor::onCancelClicked);
    
    buttonLayout->addStretch();
    buttonLayout->addWidget(okButton);
    buttonLayout->addWidget(cancelButton);
    
    mainLayout->addLayout(buttonLayout);
}

// 加载应用信息（从 AppInfo 结构体加载应用到表单）
void AppEditor::loadAppInfo() {
    if (app_dir_.isEmpty()) {
        return;
    }
    
    // 从 AppManager 获取应用信息
    AppManager* appManager = AppManager::Instance();
    QVector<AppInfo> apps = appManager->GetApps();
    
    // 查找匹配的应用
    for (const AppInfo& appInfo : apps) {
        if (appInfo.appDir == app_dir_) {
            // 加载名称
            name_edit_->setText(appInfo.name);
            
            // 加载图标
            icon_path_ = appInfo.iconPath;
            if (!icon_path_.isEmpty() && QFile::exists(icon_path_)) {
                QPixmap pixmap(icon_path_);
                icon_preview_label_->setPixmap(pixmap.scaled(64, 64, Qt::KeepAspectRatio, Qt::SmoothTransformation));
            } else {
                icon_preview_label_->setText("无图标");
            }
            
            // 加载启动参数（从环境变量或配置中读取，这里临时实现）
            // TODO: 从配置文件或注册表读取启动参数
            args_edit_->setText("");
            
            // 加载工作目录（从 AppInfo 或配置中读取）
            // TODO: 从 AppInfo 或配置文件读取工作目录
            work_dir_edit_->setText("");
            
            // 加载沙箱模式
            sandbox_mode_combo_->setCurrentIndex(appInfo.isPortable ? 0 : 1);
            
            // 保存原始名称（用于检查是否修改）
            original_name_ = appInfo.name;
            
            break;
        }
    }
}

// 保存应用信息（从表单保存到 AppInfo 结构体）
void AppEditor::saveAppInfo() {
    if (app_dir_.isEmpty()) {
        return;
    }
    
    // 从 AppManager 获取应用信息
    AppManager* appManager = AppManager::Instance();
    QVector<AppInfo> apps = appManager->GetApps();
    
    // 查找匹配的应用
    for (AppInfo& appInfo : apps) {
        if (appInfo.appDir == app_dir_) {
            // 保存名称
            appInfo.name = name_edit_->text();
            
            // 保存图标路径
            appInfo.iconPath = icon_path_;
            
            // 保存沙箱模式
            appInfo.isPortable = (sandbox_mode_combo_->currentIndex() == 0);
            
            // TODO: 保存启动参数和工作目录到配置文件或注册表
            
            break;
        }
    }
    
    // TODO: 将更新后的应用列表保存回 AppManager（需要 AppManager 提供更新接口）
}

// 图标按钮点击事件（打开文件对话框，选择图标文件，预览图标）
void AppEditor::onIconButtonClicked() {
    QString iconPath = QFileDialog::getOpenFileName(this, "选择图标文件", "", "图标文件 (*.ico *.png *.jpg *.bmp)");
    if (!iconPath.isEmpty()) {
        // 预览图标
        QPixmap pixmap(iconPath);
        if (!pixmap.isNull()) {
            icon_preview_label_->setPixmap(pixmap.scaled(64, 64, Qt::KeepAspectRatio, Qt::SmoothTransformation));
            icon_path_ = iconPath;
        } else {
            QMessageBox::warning(this, "警告", "无法加载图标文件");
        }
    }
}

// 确定按钮点击事件（保存应用信息，关闭对话框）
void AppEditor::onOkClicked() {
    // 验证输入
    if (name_edit_->text().trimmed().isEmpty()) {
        QMessageBox::warning(this, "警告", "应用名称不能为空");
        return;
    }
    
    // 保存应用信息
    saveAppInfo();
    
    // 关闭对话框
    accept();
}

// 取消按钮点击事件（放弃修改，关闭对话框）
void AppEditor::onCancelClicked() {
    // 放弃修改，关闭对话框
    reject();
}

}  // namespace launchpad

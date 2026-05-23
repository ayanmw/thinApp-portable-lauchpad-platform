#include "sandbox_browser.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QFileDialog>
#include <QMessageBox>
#include <QDesktopServices>
#include <QUrl>
#include <QTimer>
#include <QContextMenuEvent>
#include <QMenu>
#include <QFileInfo>
#include <QDir>
#include <QDebug>

namespace launchpad {

// 构造函数
SandboxBrowser::SandboxBrowser(QWidget* parent)
    : QWidget(parent)
    , treeView_(nullptr)
    , fileView_(nullptr)
    , model_(nullptr)
    , proxyModel_(nullptr)
    , searchBox_(nullptr)
    , searchTimer_(nullptr)
    , currentDirPath_("") {
    
    // 初始化 UI
    initUI();
    
    // 创建搜索防抖定时器
    searchTimer_ = new QTimer(this);
    searchTimer_->setSingleShot(true);
    connect(searchTimer_, &QTimer::timeout,
            this, &SandboxBrowser::onSearchDebounceTimeout);
}

// 析构函数
SandboxBrowser::~SandboxBrowser() {
    // 子对象会自动删除
}

// 设置沙箱路径
void SandboxBrowser::setSandboxPath(const QString& path) {
    if (model_) {
        model_->setSandboxPath(path);
        currentDirPath_ = path + "/VFS";
    }
}

// 获取沙箱路径
QString SandboxBrowser::sandboxPath() const {
    if (model_) {
        return model_->sandboxPath();
    }
    return "";
}

// 初始化 UI
void SandboxBrowser::initUI() {
    // 创建布局
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    
    // 创建工具栏
    QHBoxLayout* toolbarLayout = new QHBoxLayout();
    
    QLabel* searchLabel = new QLabel("搜索:", this);
    toolbarLayout->addWidget(searchLabel);
    
    searchBox_ = new QLineEdit(this);
    searchBox_->setPlaceholderText("输入文件名关键字...");
    toolbarLayout->addWidget(searchBox_);
    
    QPushButton* exportButton = new QPushButton("导出选中", this);
    toolbarLayout->addWidget(exportButton);
    
    QPushButton* deleteButton = new QPushButton("删除选中", this);
    toolbarLayout->addWidget(deleteButton);
    
    QPushButton* refreshButton = new QPushButton("刷新", this);
    toolbarLayout->addWidget(refreshButton);
    
    toolbarLayout->addStretch();
    
    mainLayout->addLayout(toolbarLayout);
    
    // 创建分割视图（目录树 + 文件列表）
    QHBoxLayout* viewLayout = new QHBoxLayout();
    
    // 创建目录树
    treeView_ = new QTreeView(this);
    model_ = new SandboxModel("", this);
    treeView_->setModel(model_);
    treeView_->setHeaderHidden(true);
    viewLayout->addWidget(treeView_, 1);  // 占 1 份
    
    // 创建文件列表
    fileView_ = new QTableView(this);
    proxyModel_ = new QSortFilterProxyModel(this);
    proxyModel_->setSourceModel(model_);
    fileView_->setModel(proxyModel_);
    viewLayout->addWidget(fileView_, 3);  // 占 3 份
    
    mainLayout->addLayout(viewLayout);
    
    // 创建状态栏
    QHBoxLayout* statusLayout = new QHBoxLayout();
    
    QLabel* statusLabel = new QLabel("就绪", this);
    statusLayout->addWidget(statusLabel);
    
    statusLayout->addStretch();
    
    mainLayout->addLayout(statusLayout);
    
    // 连接信号/槽
    connect(treeView_, &QTreeView::clicked,
            this, &SandboxBrowser::onDirSelected);
    
    connect(fileView_, &QTableView::doubleClicked,
            this, &SandboxBrowser::onFileDoubleClicked);
    
    connect(searchBox_, &QLineEdit::textChanged,
            this, &SandboxBrowser::onSearchTextChanged);
    
    connect(exportButton, &QPushButton::clicked, [this]() {
        // 获取选中的文件
        QModelIndexList selectedIndexes = fileView_->selectionModel()->selectedIndexes();
        if (!selectedIndexes.isEmpty()) {
            QString filePath = proxyModel_->data(selectedIndexes.first(), Qt::UserRole + 1).toString();
            exportFile(filePath);
        }
    });
    
    connect(deleteButton, &QPushButton::clicked, [this]() {
        // 获取选中的文件
        QModelIndexList selectedIndexes = fileView_->selectionModel()->selectedIndexes();
        if (!selectedIndexes.isEmpty()) {
            QString filePath = proxyModel_->data(selectedIndexes.first(), Qt::UserRole + 1).toString();
            deleteFile(filePath);
        }
    });
    
    connect(refreshButton, &QPushButton::clicked, [this]() {
        if (model_) {
            model_->setSandboxPath(model_->sandboxPath());
        }
    });
}

// 目录选中槽
void SandboxBrowser::onDirSelected(const QModelIndex& index) {
    if (!model_) {
        return;
    }
    
    // 获取选中的目录路径
    QString dirPath = model_->filePath(index);
    QFileInfo dirInfo(dirPath);
    
    if (!dirInfo.isDir()) {
        return;  // 不是目录
    }
    
    // 更新当前目录路径
    currentDirPath_ = dirPath;
    
    // 刷新文件列表（显示该目录下的文件）
    // 注意：这里需要正确实现文件列表的过滤逻辑
    // POC 阶段暂不实现
}

// 搜索文本变化槽
void SandboxBrowser::onSearchTextChanged(const QString& text) {
    Q_UNUSED(text);
    
    // 防抖：300ms 后执行搜索
    searchTimer_->stop();
    searchTimer_->start(300);
}

// 文件双击槽
void SandboxBrowser::onFileDoubleClicked(const QModelIndex& index) {
    if (!model_) {
        return;
    }
    
    // 获取文件路径
    QModelIndex sourceIndex = proxyModel_->mapToSource(index);
    QString filePath = model_->filePath(sourceIndex);
    
    // 发射文件选中信号
    emit fileSelected(filePath);
    
    // 打开文件
    openFile(filePath);
}

// 搜索防抖定时器超时槽
void SandboxBrowser::onSearchDebounceTimeout() {
    if (!proxyModel_) {
        return;
    }
    
    // 获取搜索文本
    QString text = searchBox_->text();
    
    // 设置过滤规则
    proxyModel_->setFilterFixedString(text);
    proxyModel_->setFilterCaseSensitivity(Qt::CaseInsensitive);
}

// 右键菜单事件
void SandboxBrowser::contextMenuEvent(QContextMenuEvent* event) {
    // 获取鼠标位置下的模型索引
    QPoint pos = event->pos();
    QModelIndex index = fileView_->indexAt(pos);
    
    QString filePath;
    if (index.isValid()) {
        QModelIndex sourceIndex = proxyModel_->mapToSource(index);
        filePath = model_->filePath(sourceIndex);
    }
    
    // 创建右键菜单
    QMenu contextMenu(this);
    
    QAction* openAction = contextMenu.addAction("打开");
    QAction* exportAction = contextMenu.addAction("导出");
    QAction* deleteAction = contextMenu.addAction("删除");
    contextMenu.addSeparator();
    QAction* propertiesAction = contextMenu.addAction("属性");
    
    // 执行菜单
    QAction* selectedAction = contextMenu.exec(event->globalPos());
    
    if (!selectedAction) {
        return;
    }
    
    // 处理菜单动作
    if (selectedAction == openAction) {
        openFile(filePath);
    } else if (selectedAction == exportAction) {
        exportFile(filePath);
    } else if (selectedAction == deleteAction) {
        deleteFile(filePath);
    } else if (selectedAction == propertiesAction) {
        showFileProperties(filePath);
    }
}

// 打开文件
void SandboxBrowser::openFile(const QString& filePath) {
    if (filePath.isEmpty()) {
        return;
    }
    
    QFileInfo fileInfo(filePath);
    if (!fileInfo.exists()) {
        QMessageBox::warning(this, "警告", "文件不存在");
        return;
    }
    
    // 使用系统默认程序打开文件
    QUrl fileUrl = QUrl::fromLocalFile(filePath);
    if (!QDesktopServices::openUrl(fileUrl)) {
        QMessageBox::warning(this, "警告", "无法打开文件");
    } else {
        // 发射文件打开信号
        emit fileOpened(filePath);
    }
}

// 导出文件
void SandboxBrowser::exportFile(const QString& filePath) {
    if (filePath.isEmpty()) {
        return;
    }
    
    QFileInfo fileInfo(filePath);
    if (!fileInfo.exists()) {
        QMessageBox::warning(this, "警告", "文件不存在");
        return;
    }
    
    // 打开文件保存对话框
    QString destPath = QFileDialog::getSaveFileName(
        this, "导出文件", fileInfo.fileName());
    
    if (destPath.isEmpty()) {
        return;
    }
    
    // 复制文件
    if (!QFile::copy(filePath, destPath)) {
        QMessageBox::warning(this, "警告", "导出文件失败");
    } else {
        // 发射文件导出信号
        emit fileExported(filePath, destPath);
        QMessageBox::information(this, "成功", "文件导出成功");
    }
}

// 删除文件
void SandboxBrowser::deleteFile(const QString& filePath) {
    if (filePath.isEmpty()) {
        return;
    }
    
    QFileInfo fileInfo(filePath);
    if (!fileInfo.exists()) {
        QMessageBox::warning(this, "警告", "文件不存在");
        return;
    }
    
    // 确认删除
    QMessageBox::StandardButton reply = QMessageBox::question(
        this, "确认删除", "确定要删除此文件吗？",
        QMessageBox::Yes | QMessageBox::No);
    
    if (reply == QMessageBox::Yes) {
        bool success = false;
        
        if (fileInfo.isDir()) {
            // 删除目录（递归删除）
            QDir dir(filePath);
            success = dir.removeRecursively();
        } else {
            // 删除文件
            success = QFile::remove(filePath);
        }
        
        if (!success) {
            QMessageBox::warning(this, "警告", "删除文件失败");
        } else {
            // 发射文件删除信号
            emit fileDeleted(filePath);
            
            // 刷新模型
            if (model_) {
                model_->setSandboxPath(model_->sandboxPath());
            }
        }
    }
}

// 显示文件属性
void SandboxBrowser::showFileProperties(const QString& filePath) {
    if (filePath.isEmpty()) {
        return;
    }
    
    QFileInfo fileInfo(filePath);
    if (!fileInfo.exists()) {
        QMessageBox::warning(this, "警告", "文件不存在");
        return;
    }
    
    // 创建属性对话框
    QDialog* dialog = new QDialog(this);
    dialog->setWindowTitle("文件属性");
    dialog->setMinimumSize(400, 300);
    
    QVBoxLayout* layout = new QVBoxLayout(dialog);
    
    // 文件名称
    QLabel* nameLabel = new QLabel("名称: " + fileInfo.fileName(), dialog);
    layout->addWidget(nameLabel);
    
    // 文件路径
    QLabel* pathLabel = new QLabel("路径: " + fileInfo.absoluteFilePath(), dialog);
    layout->addWidget(pathLabel);
    
    // 文件大小
    QLabel* sizeLabel = new QLabel("大小: " + QString::number(fileInfo.size()) + " 字节", dialog);
    layout->addWidget(sizeLabel);
    
    // 文件类型
    QLabel* typeLabel = new QLabel("类型: " + (fileInfo.isDir() ? "文件夹" : fileInfo.suffix().toUpper() + " 文件"), dialog);
    layout->addWidget(typeLabel);
    
    // 修改时间
    QLabel* modifiedLabel = new QLabel("修改时间: " + fileInfo.lastModified().toString("yyyy-MM-dd HH:mm:ss"), dialog);
    layout->addWidget(modifiedLabel);
    
    // 确定按钮
    QPushButton* okButton = new QPushButton("确定", dialog);
    connect(okButton, &QPushButton::clicked, dialog, &QDialog::accept);
    layout->addWidget(okButton);
    
    dialog->exec();
}

}  // namespace launchpad

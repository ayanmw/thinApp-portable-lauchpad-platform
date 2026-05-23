#pragma once

#include <QWidget>
#include <QTreeView>
#include <QTableView>
#include <QLineEdit>
#include <QSortFilterProxyModel>
#include "sandbox_model.h"

namespace launchpad {

// 沙箱文件浏览器（QWidget 子类）
// 集成目录树、文件列表、搜索框
class SandboxBrowser : public QWidget {
    Q_OBJECT

public:
    // 构造函数
    // parent: 父窗口指针
    explicit SandboxBrowser(QWidget* parent = nullptr);

    // 析构函数
    ~SandboxBrowser();

    // 设置沙箱路径
    // path: 沙箱根目录路径
    void setSandboxPath(const QString& path);

    // 获取沙箱路径
    // 返回值：沙箱根目录路径
    QString sandboxPath() const;

signals:
    // 文件选中信号
    // filePath: 选中的文件路径
    void fileSelected(const QString& filePath);

    // 文件打开信号
    // filePath: 打开的文件路径
    void fileOpened(const QString& filePath);

    // 文件导出信号
    // filePath: 导出的文件路径
    // destPath: 导出目标路径
    void fileExported(const QString& filePath, const QString& destPath);

    // 文件删除信号
    // filePath: 删除的文件路径
    void fileDeleted(const QString& filePath);

private slots:
    // 目录选中槽（目录树选中 → 刷新文件列表）
    // index: 选中的模型索引
    void onDirSelected(const QModelIndex& index);

    // 搜索文本变化槽（实时过滤文件列表）
    // text: 搜索文本
    void onSearchTextChanged(const QString& text);

    // 文件双击槽（双击文件 → 发射 fileSelected 信号）
    // index: 双击的模型索引
    void onFileDoubleClicked(const QModelIndex& index);

    // 搜索防抖定时器超时槽
    void onSearchDebounceTimeout();

private:
    // 初始化 UI
    void initUI();

    // 创建右键菜单
    // event: 上下文菜单事件
    void contextMenuEvent(QContextMenuEvent* event) override;

    // 打开文件
    // filePath: 文件路径
    void openFile(const QString& filePath);

    // 导出文件
    // filePath: 文件路径
    void exportFile(const QString& filePath);

    // 删除文件
    // filePath: 文件路径
    void deleteFile(const QString& filePath);

    // 显示文件属性
    // filePath: 文件路径
    void showFileProperties(const QString& filePath);

    // 目录树视图
    QTreeView* treeView_;

    // 文件列表视图
    QTableView* fileView_;

    // 沙箱模型
    SandboxModel* model_;

    // 搜索过滤模型
    QSortFilterProxyModel* proxyModel_;

    // 搜索框
    QLineEdit* searchBox_;

    // 搜索防抖定时器
    QTimer* searchTimer_;

    // 当前选中的目录路径
    QString currentDirPath_;
};

}  // namespace launchpad

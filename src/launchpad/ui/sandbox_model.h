#pragma once

#include <QAbstractItemModel>
#include <QFileInfo>
#include <QVector>
#include <QFileSystemWatcher>
#include <QString>
#include <QModelIndex>
#include <QVariant>
#include <QHash>

namespace launchpad {

// 沙箱文件浏览器模型（继承 QAbstractItemModel）
// 用于展示沙箱目录结构（虚拟路径映射）
class SandboxModel : public QAbstractItemModel {
    Q_OBJECT

public:
    // 构造函数
    // appDir: 应用目录路径（沙箱根目录）
    // parent: 父对象指针
    explicit SandboxModel(const QString& appDir, QObject* parent = nullptr);

    // 析构函数
    ~SandboxModel();

    // 设置沙箱根目录
    // path: 新的沙箱根目录路径
    void setSandboxPath(const QString& path);

    // 获取沙箱根目录
    // 返回值：沙箱根目录路径
    QString sandboxPath() const;

    // 映射虚拟路径到真实路径
    // index: 模型索引
    // 返回值：真实文件路径
    QString mapToRealPath(const QModelIndex& index) const;

    // 映射真实路径到虚拟路径
    // realPath: 真实文件路径
    // 返回值：虚拟文件路径
    QString mapFromRealPath(const QString& realPath) const;

    // 检查索引是否为目录
    // index: 模型索引
    // 返回值：是目录返回 true，否则返回 false
    bool isDir(const QModelIndex& index) const;

    // 获取文件路径
    // index: 模型索引
    // 返回值：文件完整路径（虚拟路径）
    QString filePath(const QModelIndex& index) const;

    // QAbstractItemModel 接口实现
    QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex& index) const override;
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, 
                       int role = Qt::DisplayRole) const override;
    Qt::ItemFlags flags(const QModelIndex& index) const override;

private slots:
    // 文件系统变化槽（由 QFileSystemWatcher 触发）
    // path: 变化的文件路径
    void OnDirectoryChanged(const QString& path);

private:
    // 获取文件信息（从内部数据结构）
    // index: 模型索引
    // 返回值：文件信息对象
    QFileInfo getFileInfo(const QModelIndex& index) const;

    // 获取父目录的文件列表
    // parentIndex: 父索引
    // 返回值：文件信息列表
    QVector<QFileInfo> getFileList(const QModelIndex& parentIndex) const;

    // 将文件路径转换为内部唯一 ID（用于在 QModelIndex 中存储）
    // filePath: 文件完整路径
    // 返回值：唯一 ID（路径的哈希值）
    quintptr pathToId(const QString& filePath) const;

    // 将内部唯一 ID 转换为文件路径
    // id: 唯一 ID
    // 返回值：文件完整路径
    QString idToPath(quintptr id) const;

    // 沙箱根目录路径
    QString appDir_;

    // VFS 目录路径（{AppDir}\VFS\）
    QString vfsDir_;

    // 文件系统监听器
    QFileSystemWatcher* watcher_;

    // 路径到 ID 的映射（用于 QModelIndex 内部指针）
    mutable QHash<QString, quintptr> pathToIdMap_;

    // ID 到路径的映射
    mutable QHash<quintptr, QString> idToPathMap_;

    // 下一个可用的 ID
    mutable quintptr nextId_;
};

}  // namespace launchpad

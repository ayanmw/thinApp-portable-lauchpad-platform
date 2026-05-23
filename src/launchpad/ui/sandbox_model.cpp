#include "sandbox_model.h"
#include <QDebug>
#include <QDir>
#include <QFileIconProvider>
#include <QApplication>

namespace launchpad {

// 构造函数
SandboxModel::SandboxModel(const QString& appDir, QObject* parent)
    : QAbstractItemModel(parent)
    , appDir_(appDir)
    , vfsDir_(appDir + "/VFS")
    , watcher_(nullptr)
    , nextId_(1) {
    
    // 创建文件系统监听器
    watcher_ = new QFileSystemWatcher(this);
    
    // 连接文件系统变化信号
    connect(watcher_, &QFileSystemWatcher::directoryChanged,
            this, &SandboxModel::OnDirectoryChanged);
    
    // 如果 VFS 目录存在，添加到监听器
    QDir vfsDir(vfsDir_);
    if (vfsDir.exists()) {
        watcher_->addPath(vfsDir_);
    }
}

// 析构函数
SandboxModel::~SandboxModel() {
    // watcher_ 是 QObject 子对象，会自动删除
}

// 设置沙箱根目录
void SandboxModel::setSandboxPath(const QString& path) {
    // 检查路径是否变化
    if (appDir_ == path) {
        return;
    }
    
    // 移除旧的监听路径
    if (!vfsDir_.isEmpty()) {
        watcher_->removePath(vfsDir_);
    }
    
    // 更新路径
    appDir_ = path;
    vfsDir_ = appDir_ + "/VFS";
    
    // 添加新的监听路径
    QDir vfsDir(vfsDir_);
    if (vfsDir.exists()) {
        watcher_->addPath(vfsDir_);
    }
    
    // 清除缓存
    pathToIdMap_.clear();
    idToPathMap_.clear();
    nextId_ = 1;
    
    // 触发布局变化信号
    emit layoutChanged();
}

// 获取沙箱根目录
QString SandboxModel::sandboxPath() const {
    return appDir_;
}

// 映射虚拟路径到真实路径
QString SandboxModel::mapToRealPath(const QModelIndex& index) const {
    if (!index.isValid()) {
        return "";
    }
    
    // 获取虚拟路径
    QString virtualPath = filePath(index);
    
    // 移除 VFS 前缀，得到真实路径
    // 例如：{AppDir}\VFS\C\Windows\System32 → C:\Windows\System32
    QString relativePath = virtualPath;
    relativePath.replace(vfsDir_ + "/", "");  // 移除 VFS 前缀
    
    // 将路径中的 / 替换为 \
    relativePath.replace("/", "\\");
    
    // 处理盘符（例如：C 变为 C:）
    if (relativePath.length() >= 1 && relativePath[1] != ':') {
        relativePath = relativePath[0] + ":" + relativePath.mid(1);
    }
    
    return relativePath;
}

// 映射真实路径到虚拟路径
QString SandboxModel::mapFromRealPath(const QString& realPath) const {
    // 将真实路径转换为虚拟路径
    // 例如：C:\Windows\System32 → {AppDir}\VFS\C\Windows\System32
    
    QString virtualPath = realPath;
    
    // 将路径中的 \ 替换为 /
    virtualPath.replace("\\", "/");
    
    // 处理盘符（例如：C: 变为 C）
    if (virtualPath.length() >= 2 && virtualPath[1] == ':') {
        virtualPath = virtualPath[0] + virtualPath.mid(2);
    }
    
    // 添加 VFS 前缀
    virtualPath = vfsDir_ + "/" + virtualPath;
    
    return virtualPath;
}

// 检查索引是否为目录
bool SandboxModel::isDir(const QModelIndex& index) const {
    if (!index.isValid()) {
        return false;
    }
    
    QString path = filePath(index);
    QFileInfo fileInfo(path);
    return fileInfo.isDir();
}

// 获取文件路径
QString SandboxModel::filePath(const QModelIndex& index) const {
    if (!index.isValid()) {
        // 根节点，返回 VFS 目录
        return vfsDir_;
    }
    
    // 从 QModelIndex 中获取文件路径
    quintptr id = index.internalId();
    return idToPath(id);
}

// 创建索引
QModelIndex SandboxModel::index(int row, int column, const QModelIndex& parent) const {
    if (!hasIndex(row, column, parent)) {
        return QModelIndex();
    }
    
    // 获取父目录的文件列表
    QVector<QFileInfo> fileList = getFileList(parent);
    
    if (row >= fileList.size() || row < 0) {
        return QModelIndex();
    }
    
    // 获取文件信息
    QFileInfo fileInfo = fileList.at(row);
    QString filePath = fileInfo.absoluteFilePath();
    
    // 获取或创建 ID
    quintptr id = pathToId(filePath);
    
    // 创建索引
    return createIndex(row, column, reinterpret_cast<void*>(id));
}

// 获取父索引
QModelIndex SandboxModel::parent(const QModelIndex& index) const {
    if (!index.isValid()) {
        return QModelIndex();
    }
    
    // 获取文件路径
    QString filePath = this->filePath(index);
    QFileInfo fileInfo(filePath);
    
    // 获取父目录路径
    QString parentPath = fileInfo.absolutePath();
    
    // 如果父目录是 VFS 目录，返回无效索引（根节点）
    if (parentPath == vfsDir_) {
        return QModelIndex();
    }
    
    // 获取父目录的文件信息
    QFileInfo parentInfo(parentPath);
    
    // 获取父目录的父目录的文件列表
    QString grandparentPath = parentInfo.absolutePath();
    QDir grandparentDir(grandparentPath);
    QFileInfoList entries = grandparentDir.entryInfoList(
        QDir::AllEntries | QDir::NoDotAndDotDot,
        QDir::DirsFirst | QDir::IgnoreCase);
    
    // 查找父目录在祖父目录中的行号
    for (int i = 0; i < entries.size(); ++i) {
        if (entries.at(i).absoluteFilePath() == parentPath) {
            // 创建父索引
            quintptr id = pathToId(parentPath);
            return createIndex(i, 0, reinterpret_cast<void*>(id));
        }
    }
    
    return QModelIndex();
}

// 获取行数
int SandboxModel::rowCount(const QModelIndex& parent) const {
    // 根节点
    if (!parent.isValid()) {
        // 返回 VFS 目录下的文件和目录数量
        QDir vfsDir(vfsDir_);
        if (!vfsDir.exists()) {
            return 0;
        }
        
        QFileInfoList entries = vfsDir.entryInfoList(
            QDir::AllEntries | QDir::NoDotAndDotDot,
            QDir::DirsFirst | QDir::IgnoreCase);
        
        return entries.size();
    }
    
    // 子节点
    if (!parent.isValid()) {
        return 0;
    }
    
    // 获取目录路径
    QString dirPath = filePath(parent);
    QFileInfo dirInfo(dirPath);
    
    if (!dirInfo.isDir()) {
        return 0;  // 不是目录，没有子节点
    }
    
    // 返回目录下的文件和目录数量
    QDir dir(dirPath);
    QFileInfoList entries = dir.entryInfoList(
        QDir::AllEntries | QDir::NoDotAndDotDot,
        QDir::DirsFirst | QDir::IgnoreCase);
    
    return entries.size();
}

// 获取列数
int SandboxModel::columnCount(const QModelIndex& /*parent*/) const {
    // 5 列：名称、大小、类型、修改时间、路径
    return 5;
}

// 获取数据
QVariant SandboxModel::data(const QModelIndex& index, int role) const {
    if (!index.isValid()) {
        return QVariant();
    }
    
    // 获取文件信息
    QString path = filePath(index);
    QFileInfo fileInfo(path);
    
    // 根据角色返回数据
    if (role == Qt::DisplayRole) {
        switch (index.column()) {
        case 0:  // 名称
            return fileInfo.fileName();
        case 1:  // 大小
            if (fileInfo.isDir()) {
                return "";
            } else {
                return fileInfo.size();
            }
        case 2:  // 类型
            if (fileInfo.isDir()) {
                return "文件夹";
            } else {
                return fileInfo.suffix().toUpper() + " 文件";
            }
        case 3:  // 修改时间
            return fileInfo.lastModified().toString("yyyy-MM-dd HH:mm");
        case 4:  // 路径
            return fileInfo.absoluteFilePath();
        default:
            return QVariant();
        }
    } else if (role == Qt::DecorationRole && index.column() == 0) {
        // 图标
        QFileIconProvider iconProvider;
        return iconProvider.icon(fileInfo);
    } else if (role == Qt::TextAlignmentRole) {
        // 文本对齐方式
        if (index.column() == 1) {  // 大小列右对齐
            return Qt::AlignRight + Qt::AlignVCenter;
        } else {
            return Qt::AlignLeft + Qt::AlignVCenter;
        }
    }
    
    return QVariant();
}

// 获取表头数据
QVariant SandboxModel::headerData(int section, Qt::Orientation orientation, 
                                 int role) const {
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        switch (section) {
        case 0:
            return "名称";
        case 1:
            return "大小";
        case 2:
            return "类型";
        case 3:
            return "修改时间";
        case 4:
            return "路径";
        default:
            return QVariant();
        }
    }
    
    return QVariant();
}

// 获取标志位
Qt::ItemFlags SandboxModel::flags(const QModelIndex& index) const {
    if (!index.isValid()) {
        return Qt::NoItemFlags;
    }
    
    return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

// 文件系统变化槽
void SandboxModel::OnDirectoryChanged(const QString& path) {
    Q_UNUSED(path);
    
    // 清除缓存
    pathToIdMap_.clear();
    idToPathMap_.clear();
    nextId_ = 1;
    
    // 触发布局变化信号
    emit layoutChanged();
}

// 获取文件信息
QFileInfo SandboxModel::getFileInfo(const QModelIndex& index) const {
    QString path = filePath(index);
    return QFileInfo(path);
}

// 获取父目录的文件列表
QVector<QFileInfo> SandboxModel::getFileList(const QModelIndex& parentIndex) const {
    QVector<QFileInfo> result;
    
    // 根节点
    if (!parentIndex.isValid()) {
        // 返回 VFS 目录下的文件和目录
        QDir vfsDir(vfsDir_);
        if (!vfsDir.exists()) {
            return result;
        }
        
        QFileInfoList entries = vfsDir.entryInfoList(
            QDir::AllEntries | QDir::NoDotAndDotDot,
            QDir::DirsFirst | QDir::IgnoreCase);
        
        for (const QFileInfo& entry : entries) {
            result.append(entry);
        }
        
        return result;
    }
    
    // 子节点
    QString dirPath = filePath(parentIndex);
    QFileInfo dirInfo(dirPath);
    
    if (!dirInfo.isDir()) {
        return result;  // 不是目录，没有子节点
    }
    
    // 返回目录下的文件和目录
    QDir dir(dirPath);
    QFileInfoList entries = dir.entryInfoList(
        QDir::AllEntries | QDir::NoDotAndDotDot,
        QDir::DirsFirst | QDir::IgnoreCase);
    
    for (const QFileInfo& entry : entries) {
        result.append(entry);
    }
    
    return result;
}

// 将文件路径转换为内部唯一 ID
quintptr SandboxModel::pathToId(const QString& filePath) const {
    // 检查是否已有 ID
    if (pathToIdMap_.contains(filePath)) {
        return pathToIdMap_[filePath];
    }
    
    // 创建新 ID
    quintptr id = nextId_++;
    pathToIdMap_[filePath] = id;
    idToPathMap_[id] = filePath;
    
    return id;
}

// 将内部唯一 ID 转换为文件路径
QString SandboxModel::idToPath(quintptr id) const {
    // 查找路径
    if (idToPathMap_.contains(id)) {
        return idToPathMap_[id];
    }
    
    return "";
}

}  // namespace launchpad

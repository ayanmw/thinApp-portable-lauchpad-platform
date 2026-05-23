/**
 * @file app_capture.h
 * @brief 应用捕获器头文件
 * 
 * 应用捕获工具（AppCapture）是 Packager 模块的核心组件，
 * 用于监控应用安装过程，捕获所有文件/注册表操作，生成 .vapp 包。
 * 
 * @author AI ThinApp Team
 * @date 2026-05-23
 */

#ifndef APP_CAPTURE_H
#define APP_CAPTURE_H

#include <Windows.h>
#include <string>
#include <vector>
#include <functional>

namespace packager {

// ============================================================================
// 枚举和结构体定义
// ============================================================================

/**
 * @brief 捕获事件类型
 * 
 * 表示在应用安装过程中捕获到的文件或注册表操作类型。
 */
enum class CaptureEvent {
    FileCreated,       // 文件创建
    FileModified,      // 文件修改
    FileDeleted,      // 文件删除
    RegKeyCreated,    // 注册表键创建
    RegValueSet,      // 注册表值设置
    RegKeyDeleted     // 注册表键删除
};

/**
 * @brief 捕获事件记录
 * 
 * 记录一个捕获事件（文件或注册表操作）的详细信息。
 */
struct CaptureRecord {
    CaptureEvent type;       // 事件类型
    std::string path;        // 文件/注册表路径
    std::string data;        // 数据（文件内容/注册表值）
    FILETIME timestamp;      // 时间戳
};

/**
 * @brief 捕获快照
 * 
 * 记录在某一时刻的文件系统和注册表状态。
 */
struct CaptureSnapshot {
    std::vector<std::string> files;   // 文件列表（完整路径）
    std::vector<std::string> regKeys; // 注册表键列表（完整路径）
};

// ============================================================================
// AppCapture 类定义
// ============================================================================

/**
 * @brief 应用捕获器类
 * 
 * 负责监控应用安装过程，捕获所有文件/注册表操作，生成 .vapp 包。
 * 
 * 使用示例：
 * @code
 * packager::AppCapture capture;
 * 
 * // 设置进度回调函数
 * capture.OnProgress = [](int progress, const std::string& status) {
 *     printf("进度: %d%%, 状态: %s\n", progress, status.c_str());
 * };
 * 
 * // 开始捕获（创建前快照）
 * if (!capture.Start()) {
 *     printf("开始捕获失败\n");
 *     return;
 * }
 * 
 * // 启动安装程序（用户手动操作）
 * printf("请安装应用，完成后点击停止捕获\n");
 * 
 * // 停止捕获（创建后快照，对比差异）
 * if (!capture.Stop()) {
 *     printf("停止捕获失败\n");
 *     return;
 * }
 * 
 * // 获取捕获记录
 * const auto& records = capture.GetRecords();
 * printf("捕获到 %zu 个事件\n", records.size());
 * 
 * // 导出 .vapp 包
 * if (!capture.ExportVApp("output.vapp")) {
 *     printf("导出 .vapp 包失败\n");
 *     return;
 * }
 * 
 * printf("捕获完成，.vapp 包已导出\n");
 * @endcode
 */
class AppCapture {
public:
    /**
     * @brief 构造函数
     * 
     * 初始化捕获器，创建临时目录。
     */
    AppCapture();

    /**
     * @brief 析构函数
     * 
     * 清理资源，删除临时目录。
     */
    ~AppCapture();

    // ========================================================================
    // 公共接口
    // ========================================================================

    /**
     * @brief 开始捕获（创建前快照）
     * 
     * 创建文件系统和注册表的前快照，用于后续对比。
     * 
     * @return TRUE: 成功; FALSE: 失败
     * 
     * @note 此函数会遍历关键目录（C:\Program Files, C:\Windows, AppData 等），
     *       可能需要几秒钟时间。
     */
    bool Start();

    /**
     * @brief 停止捕获（创建后快照，对比差异）
     * 
     * 创建文件系统和注册表的后快照，对比前后快照，生成差异列表。
     * 
     * @return TRUE: 成功; FALSE: 失败
     * 
     * @note 此函数会遍历关键目录并对比差异，可能需要几秒钟时间。
     */
    bool Stop();

    /**
     * @brief 获取捕获记录
     * 
     * 返回在捕获过程中记录的所有文件/注册表操作。
     * 
     * @return 捕获记录列表（常量引用）
     * 
     * @note 在调用 Stop() 之前，此列表为空。
     */
    const std::vector<CaptureRecord>& GetRecords() const {
        return records_;
    }

    /**
     * @brief 导出 .vapp 包
     * 
     * 将捕获的差异文件和注册表导出为 .vapp 包（ZIP 格式）。
     * 
     * @param outputPath 输出文件路径（如 "output.vapp"）
     * @return TRUE: 成功; FALSE: 失败
     * 
     * @note .vapp 包结构：
     *       - VFS\ 目录（虚拟文件系统）
     *       - VFS\registry.hive（虚拟注册表 hive）
     *       - manifest.json（元数据）
     */
    bool ExportVApp(const std::string& outputPath);

    /**
     * @brief 回调函数（进度通知）
     * 
     * 设置此回调函数可接收捕获进度通知。
     * 
     * 回调函数签名：
     * void OnProgress(int progress, const std::string& status);
     * 
     * @param progress 进度百分比（0-100）
     * @param status 状态描述字符串
     * 
     * @example
     * @code
     * capture.OnProgress = [](int progress, const std::string& status) {
     *     printf("[%d%%] %s\n", progress, status.c_str());
     * };
     * @endcode
     */
    std::function<void(int progress, const std::string& status)> OnProgress;

private:
    // ========================================================================
    // 私有方法
    // ========================================================================

    /**
     * @brief 创建快照
     * 
     * 创建文件系统和注册表的快照（枚举所有关键目录和注册表键）。
     * 
     * @param[out] snapshot 快照对象（输出参数）
     * @return TRUE: 成功; FALSE: 失败
     * 
     * @note 此函数会遍历以下目录和注册表键：
     *       - 文件系统：C:\Program Files, C:\Windows, AppData 等
     *       - 注册表：HKLM\Software, HKCU\Software 等
     */
    bool CreateSnapshot(CaptureSnapshot& snapshot);

    /**
     * @brief 对比快照
     * 
     * 对比前后快照，生成差异列表（新增/修改/删除的文件和注册表键）。
     * 
     * @param[in] before 前快照
     * @param[in] after 后快照
     * @param[out] records 捕获记录列表（输出参数）
     * @return TRUE: 成功; FALSE: 失败
     * 
     * @note 差异类型：
     *       - 新增：后快照有，前快照无
     *       - 修改：前后快照都有，但文件大小/修改时间不同
     *       - 删除：前快照有，后快照无（POC 阶段不处理）
     */
    bool CompareSnapshots(const CaptureSnapshot& before,
                         const CaptureSnapshot& after,
                         std::vector<CaptureRecord>& records);

    /**
     * @brief 枚举文件系统
     * 
     * 递归枚举指定目录下的所有文件，保存到快照中。
     * 
     * @param[in] directory 要枚举的目录路径
     * @param[out] files 文件列表（输出参数）
     * 
     * @note 此函数会递归枚举子目录。
     */
    void EnumFiles(const std::string& directory,
                   std::vector<std::string>& files);

    /**
     * @brief 枚举注册表键
     * 
     * 递归枚举指定注册表键下的所有子键和值，保存到快照中。
     * 
     * @param[in] hKey 注册表键句柄
     * @param[in] keyPath 注册表键路径（如 "HKLM\\Software"）
     * @param[out] regKeys 注册表键列表（输出参数）
     * 
     * @note 此函数会递归枚举子键。
     */
    void EnumRegKeys(HKEY hKey,
                     const std::string& keyPath,
                     std::vector<std::string>& regKeys);

    /**
     * @brief 通知进度
     * 
     * 调用 OnProgress 回调函数（若已设置）。
     * 
     * @param progress 进度百分比（0-100）
     * @param status 状态描述字符串
     */
    void NotifyProgress(int progress, const std::string& status);

    // ========================================================================
    // 私有成员变量
    // ========================================================================

    /**
     * @brief 前快照（开始捕获时创建）
     */
    CaptureSnapshot snapshotBefore_;

    /**
     * @brief 后快照（停止捕获时创建）
     */
    CaptureSnapshot snapshotAfter_;

    /**
     * @brief 捕获记录列表
     * 
     * 存储在捕获过程中记录的所有文件/注册表操作。
     */
    std::vector<CaptureRecord> records_;

    /**
     * @brief 临时目录
     * 
     * 用于存储快照、差异文件等临时数据。
     * 路径格式：`{TEMP}\AppCapture_{Timestamp}`
     */
    std::string tempDir_;

    /**
     * @brief 是否已开始捕获
     * 
     * TRUE: 已调用 Start()，但未调用 Stop()
     * FALSE: 未调用 Start()，或已调用 Stop()
     */
    bool isStarted_;

    /**
     * @brief 是否已停止捕获
     * 
     * TRUE: 已调用 Stop()
     * FALSE: 未调用 Stop()
     */
    bool isStopped_;
};

}  // namespace packager

#endif  // APP_CAPTURE_H

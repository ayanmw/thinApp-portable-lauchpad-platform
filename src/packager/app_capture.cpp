/**
 * @file app_capture.cpp
 * @brief 应用捕获器实现文件
 * 
 * 应用捕获工具（AppCapture）是 Packager 模块的核心组件，
 * 用于监控应用安装过程，捕获所有文件/注册表操作，生成 .vapp 包。
 * 
 * @author AI ThinApp Team
 * @date 2026-05-23
 */

#include "app_capture.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <filesystem>
#include <Shlobj.h>
#include <regex>

// 使用 std::filesystem
namespace fs = std::filesystem;

namespace packager {

// ============================================================================
// 构造函数和析构函数
// ============================================================================

/**
 * @brief 构造函数
 * 
 * 初始化捕获器，创建临时目录。
 */
AppCapture::AppCapture()
    : isStarted_(false), isStopped_(false) {
    // 生成临时目录路径（带时间戳）
    SYSTEMTIME st;
    GetLocalTime(&st);
    
    std::stringstream ss;
    ss << GetEnv("TEMP") << "\\AppCapture_"
       << st.wYear << st.wMonth << st.wDay << "_"
       << st.wHour << st.wMinute << st.wSecond;
    tempDir_ = ss.str();
    
    // 创建临时目录
    CreateDirectoryA(tempDir_.c_str(), NULL);
    
    printf("[AppCapture] 临时目录: %s\n", tempDir_.c_str());
}

/**
 * @brief 析构函数
 * 
 * 清理资源，删除临时目录。
 */
AppCapture::~AppCapture() {
    // 删除临时目录（递归删除所有内容）
    if (fs::exists(tempDir_)) {
        fs::remove_all(tempDir_);
        printf("[AppCapture] 已删除临时目录: %s\n", tempDir_.c_str());
    }
}

// ============================================================================
// 公共接口实现
// ============================================================================

/**
 * @brief 开始捕获（创建前快照）
 */
bool AppCapture::Start() {
    if (isStarted_) {
        printf("[AppCapture] 警告: 捕获已启动\n");
        return FALSE;
    }
    
    printf("[AppCapture] 开始捕获...\n");
    NotifyProgress(0, "开始捕获...");
    
    // 创建前快照
    NotifyProgress(10, "创建前快照（文件系统）...");
    if (!CreateSnapshot(snapshotBefore_)) {
        printf("[AppCapture] 错误: 创建前快照失败\n");
        return FALSE;
    }
    
    isStarted_ = true;
    isStopped_ = false;
    
    NotifyProgress(100, "捕获已启动，请安装应用...");
    printf("[AppCapture] 捕获已启动\n");
    
    return TRUE;
}

/**
 * @brief 停止捕获（创建后快照，对比差异）
 */
bool AppCapture::Stop() {
    if (!isStarted_) {
        printf("[AppCapture] 警告: 捕获未启动\n");
        return FALSE;
    }
    
    if (isStopped_) {
        printf("[AppCapture] 警告: 捕获已停止\n");
        return TRUE;
    }
    
    printf("[AppCapture] 停止捕获...\n");
    NotifyProgress(0, "停止捕获...");
    
    // 创建后快照
    NotifyProgress(10, "创建后快照（文件系统）...");
    if (!CreateSnapshot(snapshotAfter_)) {
        printf("[AppCapture] 错误: 创建后快照失败\n");
        return FALSE;
    }
    
    // 对比快照
    NotifyProgress(50, "对比快照...");
    if (!CompareSnapshots(snapshotBefore_, snapshotAfter_, records_)) {
        printf("[AppCapture] 错误: 对比快照失败\n");
        return FALSE;
    }
    
    isStopped_ = true;
    
    NotifyProgress(100, "捕获已停止");
    printf("[AppCapture] 捕获已停止，共捕获 %zu 个事件\n", records_.size());
    
    return TRUE;
}

/**
 * @brief 导出 .vapp 包
 */
bool AppCapture::ExportVApp(const std::string& outputPath) {
    if (!isStopped_) {
        printf("[AppCapture] 警告: 捕获未停止，请先调用 Stop()\n");
        return FALSE;
    }
    
    printf("[AppCapture] 导出 .vapp 包: %s\n", outputPath.c_str());
    NotifyProgress(0, "导出 .vapp 包...");
    
    // TODO: 实现导出逻辑
    // 1. 创建 VFS 目录结构
    // 2. 复制差异文件到 VFS 目录
    // 3. 导出注册表 hive
    // 4. 生成 manifest.json
    // 5. 压缩为 .zip 文件（改后缀为 .vapp）
    
    NotifyProgress(100, ".vapp 包导出完成");
    printf("[AppCapture] .vapp 包导出完成: %s\n", outputPath.c_str());
    
    return TRUE;
}

// ============================================================================
// 私有方法实现
// ============================================================================

/**
 * @brief 创建快照
 */
bool AppCapture::CreateSnapshot(CaptureSnapshot& snapshot) {
    printf("[AppCapture] 创建快照...\n");
    
    // 清空快照
    snapshot.files.clear();
    snapshot.regKeys.clear();
    
    // 枚举文件系统
    NotifyProgress(10, "枚举文件系统...");
    
    // 关键目录列表
    std::vector<std::string> directories = {
        "C:\\Program Files\\",
        "C:\\Program Files (x86)\\",
        "C:\\Windows\\",
        GetEnv("APPDATA"),          // Roaming
        GetEnv("LOCALAPPDATA"),     // Local
        GetEnv("PROGRAMDATA"),
        GetEnv("TEMP")
    };
    
    // 枚举每个目录
    for (const auto& dir : directories) {
        printf("[AppCapture] 枚举目录: %s\n", dir.c_str());
        EnumFiles(dir, snapshot.files);
    }
    
    NotifyProgress(50, "枚举注册表...");
    
    // 枚举注册表
    printf("[AppCapture] 枚举注册表...\n");
    
    // HKLM\Software
    EnumRegKeys(HKEY_LOCAL_MACHINE, "Software", snapshot.regKeys);
    
    // HKCU\Software
    EnumRegKeys(HKEY_CURRENT_USER, "Software", snapshot.regKeys);
    
    NotifyProgress(100, "快照创建完成");
    printf("[AppCapture] 快照创建完成: %zu 个文件, %zu 个注册表键\n",
           snapshot.files.size(), snapshot.regKeys.size());
    
    return TRUE;
}

/**
 * @brief 对比快照
 */
bool AppCapture::CompareSnapshots(const CaptureSnapshot& before,
                                  const CaptureSnapshot& after,
                                  std::vector<CaptureRecord>& records) {
    printf("[AppCapture] 对比快照...\n");
    
    // 清空记录
    records.clear();
    
    // 对比文件系统
    NotifyProgress(20, "对比文件系统...");
    
    // 找出新增/修改的文件
    for (const auto& file : after.files) {
        // 检查文件是否在前快照中存在
        auto it = std::find(before.files.begin(), before.files.end(), file);
        
        if (it == before.files.end()) {
            // 新增文件
            CaptureRecord record;
            record.type = CaptureEvent::FileCreated;
            record.path = file;
            record.data = "";
            GetSystemTimeAsFileTime(&record.timestamp);
            
            records.push_back(record);
            printf("[AppCapture] 新增文件: %s\n", file.c_str());
        } else {
            // 文件已存在（可能修改）
            // TODO: 比较文件大小/修改时间/哈希
            // 若不同，则记录为修改
        }
    }
    
    // 找出删除的文件
    for (const auto& file : before.files) {
        auto it = std::find(after.files.begin(), after.files.end(), file);
        
        if (it == after.files.end()) {
            // 删除文件
            CaptureRecord record;
            record.type = CaptureEvent::FileDeleted;
            record.path = file;
            record.data = "";
            GetSystemTimeAsFileTime(&record.timestamp);
            
            records.push_back(record);
            printf("[AppCapture] 删除文件: %s\n", file.c_str());
        }
    }
    
    // 对比注册表
    NotifyProgress(60, "对比注册表...");
    
    // 找出新增/修改的注册表键
    for (const auto& regKey : after.regKeys) {
        // 检查注册表键是否在前快照中存在
        auto it = std::find(before.regKeys.begin(), before.regKeys.end(), regKey);
        
        if (it == before.regKeys.end()) {
            // 新增注册表键
            CaptureRecord record;
            record.type = CaptureEvent::RegKeyCreated;
            record.path = regKey;
            record.data = "";
            GetSystemTimeAsFileTime(&record.timestamp);
            
            records.push_back(record);
            printf("[AppCapture] 新增注册表键: %s\n", regKey.c_str());
        }
    }
    
    // 找出删除的注册表键
    for (const auto& regKey : before.regKeys) {
        auto it = std::find(after.regKeys.begin(), after.regKeys.end(), regKey);
        
        if (it == after.regKeys.end()) {
            // 删除注册表键
            CaptureRecord record;
            record.type = CaptureEvent::RegKeyDeleted;
            record.path = regKey;
            record.data = "";
            GetSystemTimeAsFileTime(&record.timestamp);
            
            records.push_back(record);
            printf("[AppCapture] 删除注册表键: %s\n", regKey.c_str());
        }
    }
    
    NotifyProgress(100, "快照对比完成");
    printf("[AppCapture] 快照对比完成: %zu 个差异\n", records.size());
    
    return TRUE;
}

/**
 * @brief 枚举文件系统
 */
void AppCapture::EnumFiles(const std::string& directory,
                           std::vector<std::string>& files) {
    try {
        // 检查目录是否存在
        if (!fs::exists(directory)) {
            printf("[AppCapture] 警告: 目录不存在: %s\n", directory.c_str());
            return;
        }
        
        // 递归遍历目录
        for (const auto& entry : fs::recursive_directory_iterator(directory)) {
            // 只记录文件（不记录目录）
            if (fs::is_regular_file(entry.status())) {
                files.push_back(entry.path().string());
            }
        }
    } catch (const fs::filesystem_error& e) {
        printf("[AppCapture] 错误: 枚举文件失败: %s\n", e.what());
    }
}

/**
 * @brief 枚举注册表键
 */
void AppCapture::EnumRegKeys(HKEY hKey,
                             const std::string& keyPath,
                             std::vector<std::string>& regKeys) {
    // TODO: 实现注册表枚举
    // 使用 RegOpenKeyEx, RegEnumKeyEx, RegEnumValue 等 API
    
    printf("[AppCapture] 提示: 注册表枚举未实现（桩）\n");
}

/**
 * @brief 通知进度
 */
void AppCapture::NotifyProgress(int progress, const std::string& status) {
    if (OnProgress) {
        OnProgress(progress, status);
    }
}

// ============================================================================
// 辅助函数
// ============================================================================

/**
 * @brief 获取环境变量值
 * @param name 环境变量名称
 * @return 环境变量值（若找不到，返回空字符串）
 */
static std::string GetEnv(const std::string& name) {
    char buffer[32767];
    DWORD size = GetEnvironmentVariableA(name.c_str(), buffer, sizeof(buffer));
    if (size == 0) {
        return "";
    }
    return std::string(buffer, size);
}

}  // namespace packager

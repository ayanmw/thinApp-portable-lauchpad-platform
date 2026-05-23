/**
 * @file hive_manager.h
 * @brief Hive 管理器头文件
 * 
 * 管理虚拟注册表 Hive 的加载、保存和 CoW（Copy-on-Write）重定向。
 * 实现注册表虚拟化的核心逻辑。
 * 
 * @author AI ThinApp Team
 * @date 2026-05-23
 */

#ifndef HIVE_MANAGER_H
#define HIVE_MANAGER_H

#include <Windows.h>
#include <string>
#include <vector>
#include <mutex>
#include <unordered_map>

#ifdef __cplusplus
extern "C" {
#endif

namespace ThinApp {
namespace Engine {
namespace VReg {

/**
 * @brief 注册表值类型
 */
struct RegValue {
    std::wstring name;           // 值名称
    DWORD type;                  // 值类型（REG_SZ, REG_DWORD 等）
    std::vector<BYTE> data;     // 值数据
    
    RegValue() : type(REG_NONE) {}
};

/**
 * @brief 注册表键类型
 */
struct RegKey {
    std::wstring name;                      // 键名称
    std::unordered_map<std::wstring, RegValue> values;  // 值列表
    std::unordered_map<std::wstring, RegKey> subKeys;  // 子键列表
    
    /**
     * @brief 查找子键（不区分大小写）
     */
    RegKey* FindSubKey(const std::wstring& keyName) {
        for (auto& pair : subKeys) {
            if (_wcsicmp(pair.first.c_str(), keyName.c_str()) == 0) {
                return &pair.second;
            }
        }
        return nullptr;
    }
    
    /**
     * @brief 查找值（不区分大小写）
     */
    RegValue* FindValue(const std::wstring& valueName) {
        for (auto& pair : values) {
            if (_wcsicmp(pair.first.c_str(), valueName.c_str()) == 0) {
                return &pair.second;
            }
        }
        return nullptr;
    }
};

/**
 * @brief Hive 管理器类（单例模式）
 * 
 * 管理虚拟注册表 Hive 的加载、保存和 CoW 重定向。
 */
class HiveManager {
public:
    /**
     * @brief 获取单例实例
     */
    static HiveManager& GetInstance();
    
    /**
     * @brief 初始化 Hive 管理器
     * 
     * @param appDir 应用目录（用于构建 Hive 文件路径）
     * @return TRUE: 成功; FALSE: 失败
     */
    BOOL Initialize(const std::wstring& appDir);
    
    /**
     * @brief 清理 Hive 管理器
     */
    void Shutdown();
    
    /**
     * @brief CoW（Copy-on-Write）重定向
     * 
     * 实现注册表虚拟化的核心逻辑：
     * - 读取时：优先查虚拟 Hive，不存在则透传真实注册表
     * - 写入时：只修改虚拟 Hive
     * 
     * @param hKey 注册表键句柄
     * @param valueName 值名称
     * @param type 值类型
     * @param data 数据缓冲区
     * @param dataSize 数据大小
     * @param isRead TRUE: 读取操作; FALSE: 写入操作
     * @return TRUE: 在虚拟 Hive 中找到/写入成功; FALSE: 需要透传到真实注册表
     */
    BOOL CoWRedirect(HKEY hKey, const std::wstring& valueName, 
                     DWORD type, const BYTE* data, DWORD dataSize, BOOL isRead);
    
    /**
     * @brief 保存 Hive 到文件
     * 
     * 将内存中的 Hive 数据保存到 `{AppDir}\VFS\registry.hive`。
     * 
     * @return TRUE: 成功; FALSE: 失败
     */
    BOOL SaveHive();
    
    /**
     * @brief 从文件加载 Hive
     * 
     * 从 `{AppDir}\VFS\registry.hive` 加载 Hive 数据到内存。
     * 
     * @return TRUE: 成功; FALSE: 失败
     */
    BOOL LoadHive();
    
    /**
     * @brief 创建注册表键（虚拟）
     * 
     * @param path 键路径（如 "HKEY_CURRENT_USER\Software\TestApp"）
     * @return TRUE: 成功; FALSE: 失败
     */
    BOOL CreateKey(const std::wstring& path);
    
    /**
     * @brief 设置注册表值（虚拟）
     * 
     * @param path 键路径
     * @param valueName 值名称
     * @param type 值类型
     * @param data 数据缓冲区
     * @param dataSize 数据大小
     * @return TRUE: 成功; FALSE: 失败
     */
    BOOL SetValue(const std::wstring& path, const std::wstring& valueName,
                  DWORD type, const BYTE* data, DWORD dataSize);
    
    /**
     * @brief 查询注册表值（虚拟）
     * 
     * @param path 键路径
     * @param valueName 值名称
     * @param type 输出值类型
     * @param data 输出数据缓冲区
     * @param dataSize 输入输出数据大小
     * @return TRUE: 成功; FALSE: 失败
     */
    BOOL QueryValue(const std::wstring& path, const std::wstring& valueName,
                    DWORD* type, BYTE* data, DWORD* dataSize);
    
    /**
     * @brief 检查是否已初始化
     */
    BOOL IsInitialized() const {
        return initialized_;
    }
    
    /**
     * @brief 获取 Hive 文件路径
     */
    std::wstring GetHivePath() const {
        return hive_path_;
    }
    
private:
    /**
     * @brief 私有构造函数（单例模式）
     */
    HiveManager();
    
    /**
     * @brief 私有析构函数
     */
    ~HiveManager();
    
    /**
     * @brief 禁用拷贝构造函数
     */
    HiveManager(const HiveManager&) = delete;
    
    /**
     * @brief 禁用赋值运算符
     */
    HiveManager& operator=(const HiveManager&) = delete;
    
    /**
     * @brief 解析注册表路径
     * 
     * @param path 注册表路径
     * @return 根键 + 子键路径
     */
    std::pair<HKEY, std::wstring> ParseRegPath(const std::wstring& path);
    
    /**
     * @brief 查找或创建键（递归）
     */
    RegKey* FindOrCreateKey(RegKey* parent, const std::wstring& keyPath, BOOL createIfNotExist);
    
    /**
     * @brief 保存 Hive 到 JSON 格式（简化版）
     */
    BOOL SaveHiveToJson(const std::wstring& filePath);
    
    /**
     * @brief 从 JSON 格式加载 Hive（简化版）
     */
    BOOL LoadHiveFromJson(const std::wstring& filePath);
    
    /**
     * @brief 写入 JSON 键值对（辅助函数）
     */
    void WriteJsonKeyValue(std::wofstream& file, const std::wstring& key, 
                           const std::wstring& value, int indent);
    
    /**
     * @brief 写入 JSON 键（辅助函数）
     */
    void WriteJsonKey(std::wofstream& file, const std::wstring& key, int indent);
    
    /**
     * @brief 写入 JSON 值（辅助函数）
     */
    void WriteJsonValue(std::wofstream& file, const RegValue& value);
    
    /**
     * @brief 转义 JSON 字符串
     */
    std::wstring EscapeJsonString(const std::wstring& str);
    
private:
    /** @brief 是否已初始化 */
    BOOL initialized_;
    
    /** @brief 应用目录 */
    std::wstring app_dir_;
    
    /** @brief Hive 文件路径 */
    std::wstring hive_path_;
    
    /** @brief 虚拟 Hive 根键（内存） */
    RegKey root_key_;
    
    /** @brief 线程安全锁（互斥量） */
    std::mutex mutex_;
    
    /** @brief 是否已修改（需要保存） */
    BOOL modified_;
};

} // namespace VReg
} // namespace Engine
} // namespace ThinApp

#ifdef __cplusplus
}
#endif

#endif // HIVE_MANAGER_H

/**
 * @file hive_manager.cpp
 * @brief Hive 管理器源文件
 * 
 * 管理虚拟注册表 Hive 的加载、保存和 CoW（Copy-on-Write）重定向。
 * 使用 JSON 格式存储 Hive 数据（POC 阶段），V2 阶段迁移到二进制格式。
 * 
 * @author AI ThinApp Team
 * @date 2026-05-23
 */

#include "hive_manager.h"
#include <stdio.h>
#include <string.h>
#include <algorithm>
#include <fstream>
#include <sstream>
#include <iomanip>

// Windows API
#include <Windows.h>
#include <shlwapi.h>

#pragma comment(lib, "Shlwapi.lib")

// ============================================================================
// HiveManager 类实现
// ============================================================================

namespace ThinApp {
namespace Engine {
namespace VReg {

// 静态成员函数：获取单例实例
HiveManager& HiveManager::GetInstance() {
    static HiveManager instance;
    return instance;
}

/**
 * @brief 私有构造函数（单例模式）
 */
HiveManager::HiveManager() 
    : initialized_(FALSE)
    , modified_(FALSE) {
    // 初始化根键
    root_key_.name = L"ROOT";
}

/**
 * @brief 私有析构函数
 */
HiveManager::~HiveManager() {
    if (initialized_) {
        Shutdown();
    }
}

/**
 * @brief 初始化 Hive 管理器
 */
BOOL HiveManager::Initialize(const std::wstring& appDir) {
    if (initialized_) {
        printf("[HiveManager] Warning: Already initialized\n");
        return TRUE;
    }
    
    if (appDir.empty()) {
        printf("[HiveManager] Error: appDir is empty\n");
        return FALSE;
    }
    
    // 保存应用目录
    app_dir_ = appDir;
    
    // 构建 Hive 文件路径
    hive_path_ = appDir + L"\\VFS\\registry.hive";
    
    // 创建 VFS 目录（如果不存在）
    std::wstring vfsDir = appDir + L"\\VFS";
    if (!CreateDirectoryW(vfsDir.c_str(), nullptr)) {
        if (GetLastError() != ERROR_ALREADY_EXISTS) {
            printf("[HiveManager] Error: Cannot create VFS directory\n");
            return FALSE;
        }
    }
    
    // 尝试加载 Hive 文件
    if (!LoadHive()) {
        printf("[HiveManager] Warning: Cannot load hive, creating new one\n");
    }
    
    initialized_ = TRUE;
    printf("[HiveManager] Initialized successfully, hive path: %ls\n", hive_path_.c_str());
    
    return TRUE;
}

/**
 * @brief 清理 Hive 管理器
 */
void HiveManager::Shutdown() {
    if (!initialized_) {
        return;
    }
    
    // 保存 Hive 到文件
    SaveHive();
    
    // 清理内存中的数据
    root_key_.subKeys.clear();
    root_key_.values.clear();
    
    initialized_ = FALSE;
    app_dir_.clear();
    hive_path_.clear();
    modified_ = FALSE;
    
    printf("[HiveManager] Shutdown complete\n");
}

/**
 * @brief CoW（Copy-on-Write）重定向
 * 
 * 实现注册表虚拟化的核心逻辑：
 * - 读取时：优先查虚拟 Hive，不存在则透传真实注册表
 * - 写入时：只修改虚拟 Hive
 */
BOOL HiveManager::CoWRedirect(HKEY hKey, const std::wstring& valueName,
                                DWORD type, const BYTE* data, DWORD dataSize,
                                BOOL isRead) {
    if (!initialized_) {
        printf("[HiveManager] Error: Not initialized\n");
        return FALSE;
    }
    
    // 获取键路径（简化版：使用传入的 hKey 查询）
    // TODO: V2 通过 hKey 查询真实键路径
    std::wstring keyPath = L"HKCU\\Software\\TestApp"; // 示例路径
    
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (isRead) {
        // 读取操作：先查虚拟 Hive
        DWORD queryType = 0;
        BYTE queryData[1024] = {0};
        DWORD queryDataSize = sizeof(queryData);
        
        if (QueryValue(keyPath, valueName, &queryType, queryData, &queryDataSize)) {
            // 在虚拟 Hive 中找到，返回虚拟值
            printf("[HiveManager] CoWRedirect: Read from virtual hive: %ls\\%ls\n", 
                   keyPath.c_str(), valueName.c_str());
            
            // 复制数据到输出参数（如果有）
            // TODO: V2 将数据复制到调用者提供的缓冲区
            
            return TRUE;
        } else {
            // 虚拟 Hive 中未找到，透传真实注册表
            printf("[HiveManager] CoWRedirect: Not found in virtual hive, pass through to real registry: %ls\\%ls\n", 
                   keyPath.c_str(), valueName.c_str());
            return FALSE;
        }
    } else {
        // 写入操作：只修改虚拟 Hive
        if (SetValue(keyPath, valueName, type, data, dataSize)) {
            printf("[HiveManager] CoWRedirect: Write to virtual hive: %ls\\%ls\n", 
                   keyPath.c_str(), valueName.c_str());
            modified_ = TRUE;
            return TRUE;
        } else {
            printf("[HiveManager] CoWRedirect: Failed to write to virtual hive\n");
            return FALSE;
        }
    }
}

/**
 * @brief 保存 Hive 到文件
 * 
 * 将内存中的 Hive 数据保存到 `{AppDir}\VFS\registry.hive`。
 */
BOOL HiveManager::SaveHive() {
    if (!initialized_) {
        printf("[HiveManager] Error: Not initialized\n");
        return FALSE;
    }
    
    std::lock_guard<std::mutex> lock(mutex_);
    
    // 如果未修改，跳过保存
    if (!modified_) {
        printf("[HiveManager] SaveHive: No modification, skip saving\n");
        return TRUE;
    }
    
    // 保存到 JSON 格式（POC 阶段）
    if (!SaveHiveToJson(hive_path_)) {
        printf("[HiveManager] Error: Failed to save hive to JSON\n");
        return FALSE;
    }
    
    modified_ = FALSE;
    printf("[HiveManager] SaveHive: Hive saved to %ls\n", hive_path_.c_str());
    
    return TRUE;
}

/**
 * @brief 从文件加载 Hive
 * 
 * 从 `{AppDir}\VFS\registry.hive` 加载 Hive 数据到内存。
 */
BOOL HiveManager::LoadHive() {
    if (!initialized_ && hive_path_.empty()) {
        printf("[HiveManager] Error: Not initialized\n");
        return FALSE;
    }
    
    std::lock_guard<std::mutex> lock(mutex_);
    
    // 检查文件是否存在
    if (!PathFileExistsW(hive_path_.c_str())) {
        printf("[HiveManager] LoadHive: Hive file not found, creating new hive\n");
        
        // 创建空 Hive
        root_key_.subKeys.clear();
        root_key_.values.clear();
        modified_ = TRUE;
        
        // 保存到文件
        return SaveHiveToJson(hive_path_);
    }
    
    // 从 JSON 格式加载（POC 阶段）
    if (!LoadHiveFromJson(hive_path_)) {
        printf("[HiveManager] Error: Failed to load hive from JSON\n");
        return FALSE;
    }
    
    modified_ = FALSE;
    printf("[HiveManager] LoadHive: Hive loaded from %ls\n", hive_path_.c_str());
    
    return TRUE;
}

/**
 * @brief 创建注册表键（虚拟）
 */
BOOL HiveManager::CreateKey(const std::wstring& path) {
    if (!initialized_ || path.empty()) {
        return FALSE;
    }
    
    std::lock_guard<std::mutex> lock(mutex_);
    
    // 查找或创建键（递归）
    RegKey* key = FindOrCreateKey(&root_key_, path, TRUE);
    if (key == nullptr) {
        printf("[HiveManager] CreateKey: Failed to create key: %ls\n", path.c_str());
        return FALSE;
    }
    
    modified_ = TRUE;
    printf("[HiveManager] CreateKey: Key created: %ls\n", path.c_str());
    
    return TRUE;
}

/**
 * @brief 设置注册表值（虚拟）
 */
BOOL HiveManager::SetValue(const std::wstring& path, const std::wstring& valueName,
                            DWORD type, const BYTE* data, DWORD dataSize) {
    if (!initialized_ || path.empty() || valueName.empty()) {
        return FALSE;
    }
    
    std::lock_guard<std::mutex> lock(mutex_);
    
    // 查找或创建键
    RegKey* key = FindOrCreateKey(&root_key_, path, TRUE);
    if (key == nullptr) {
        printf("[HiveManager] SetValue: Failed to find/create key: %ls\n", path.c_str());
        return FALSE;
    }
    
    // 设置值
    RegValue& regValue = key->values[valueName];
    regValue.name = valueName;
    regValue.type = type;
    regValue.data.clear();
    if (data != nullptr && dataSize > 0) {
        regValue.data.resize(dataSize);
        CopyMemory(regValue.data.data(), data, dataSize);
    }
    
    modified_ = TRUE;
    printf("[HiveManager] SetValue: Value set: %ls\\%ls\n", path.c_str(), valueName.c_str());
    
    return TRUE;
}

/**
 * @brief 查询注册表值（虚拟）
 */
BOOL HiveManager::QueryValue(const std::wstring& path, const std::wstring& valueName,
                              DWORD* type, BYTE* data, DWORD* dataSize) {
    if (!initialized_ || path.empty() || valueName.empty()) {
        return FALSE;
    }
    
    std::lock_guard<std::mutex> lock(mutex_);
    
    // 查找键
    RegKey* key = FindOrCreateKey(&root_key_, path, FALSE);
    if (key == nullptr) {
        printf("[HiveManager] QueryValue: Key not found: %ls\n", path.c_str());
        return FALSE;
    }
    
    // 查找值
    RegValue* value = key->FindValue(valueName);
    if (value == nullptr) {
        printf("[HiveManager] QueryValue: Value not found: %ls\\%ls\n", path.c_str(), valueName.c_str());
        return FALSE;
    }
    
    // 返回值
    if (type != nullptr) {
        *type = value->type;
    }
    
    if (dataSize != nullptr) {
        if (data != nullptr && *dataSize >= value->data.size()) {
            CopyMemory(data, value->data.data(), value->data.size());
        }
        *dataSize = static_cast<DWORD>(value->data.size());
    }
    
    printf("[HiveManager] QueryValue: Value queried: %ls\\%ls\n", path.c_str(), valueName.c_str());
    
    return TRUE;
}

// ============================================================================
// 私有辅助函数实现
// ============================================================================

/**
 * @brief 解析注册表路径
 */
std::pair<HKEY, std::wstring> HiveManager::ParseRegPath(const std::wstring& path) {
    std::wstring upperPath = path;
    std::transform(upperPath.begin(), upperPath.end(), upperPath.begin(),
                   [](wchar_t c) { return std::towupper(c); });
    
    // 提取根键
    HKEY rootKey = nullptr;
    std::wstring subKeyPath;
    
    if (upperPath.find(L"HKEY_LOCAL_MACHINE") == 0 || upperPath.find(L"HKLM") == 0) {
        rootKey = HKEY_LOCAL_MACHINE;
        subKeyPath = path.substr(upperPath.find(L"HKEY_LOCAL_MACHINE") == 0 ? 18 : 5);
    } else if (upperPath.find(L"HKEY_CURRENT_USER") == 0 || upperPath.find(L"HKCU") == 0) {
        rootKey = HKEY_CURRENT_USER;
        subKeyPath = path.substr(upperPath.find(L"HKEY_CURRENT_USER") == 0 ? 17 : 5);
    } else if (upperPath.find(L"HKEY_CLASSES_ROOT") == 0 || upperPath.find(L"HKCR") == 0) {
        rootKey = HKEY_CLASSES_ROOT;
        subKeyPath = path.substr(upperPath.find(L"HKEY_CLASSES_ROOT") == 0 ? 17 : 5);
    } else if (upperPath.find(L"HKEY_USERS") == 0 || upperPath.find(L"HKU") == 0) {
        rootKey = HKEY_USERS;
        subKeyPath = path.substr(upperPath.find(L"HKEY_USERS") == 0 ? 11 : 4);
    } else {
        // 默认使用 HKEY_CURRENT_USER
        rootKey = HKEY_CURRENT_USER;
        subKeyPath = path;
    }
    
    // 移除前导反斜杠
    if (!subKeyPath.empty() && subKeyPath[0] == L'\\') {
        subKeyPath = subKeyPath.substr(1);
    }
    
    return {rootKey, subKeyPath};
}

/**
 * @brief 查找或创建键（递归）
 */
RegKey* HiveManager::FindOrCreateKey(RegKey* parent, const std::wstring& keyPath, BOOL createIfNotExist) {
    if (parent == nullptr || keyPath.empty()) {
        return nullptr;
    }
    
    // 解析路径（去除根键）
    auto [rootKey, subKeyPath] = ParseRegPath(keyPath);
    
    // 按反斜杠分割路径
    std::wstring pathCopy = subKeyPath;
    std::vector<std::wstring> keyNames;
    size_t pos = 0;
    while ((pos = pathCopy.find(L'\\')) != std::wstring::npos) {
        keyNames.push_back(pathCopy.substr(0, pos));
        pathCopy.erase(0, pos + 1);
    }
    if (!pathCopy.empty()) {
        keyNames.push_back(pathCopy);
    }
    
    // 递归查找或创建键
    RegKey* currentKey = parent;
    for (const auto& keyName : keyNames) {
        RegKey* subKey = currentKey->FindSubKey(keyName);
        if (subKey == nullptr) {
            if (!createIfNotExist) {
                return nullptr;
            }
            
            // 创建子键
            RegKey newKey;
            newKey.name = keyName;
            currentKey->subKeys[keyName] = newKey;
            subKey = &currentKey->subKeys[keyName];
        }
        currentKey = subKey;
    }
    
    return currentKey;
}

/**
 * @brief 保存 Hive 到 JSON 格式（简化版）
 * 
 * POC 阶段使用 JSON 格式，V2 阶段迁移到二进制格式。
 */
BOOL HiveManager::SaveHiveToJson(const std::wstring& filePath) {
    // 确保目录存在
    std::wstring dir = filePath.substr(0, filePath.find_last_of(L'\\'));
    CreateDirectoryW(dir.c_str(), nullptr);
    
    // 打开文件
    std::wofstream file(filePath, std::ios::trunc);
    if (!file.is_open()) {
        printf("[HiveManager] SaveHiveToJson: Cannot open file: %ls\n", filePath.c_str());
        return FALSE;
    }
    
    // 写入 JSON
    file << L"{\n";
    
    // 递归写入根键的子键
    int index = 0;
    for (auto& pair : root_key_.subKeys) {
        if (index > 0) {
            file << L",\n";
        }
        WriteJsonKey(file, pair.first, 1);
        WriteJsonValue(file, pair.second, 1);
        index++;
    }
    
    file << L"\n}\n";
    file.close();
    
    printf("[HiveManager] SaveHiveToJson: Hive saved to JSON: %ls\n", filePath.c_str());
    
    return TRUE;
}

/**
 * @brief 从 JSON 格式加载 Hive（简化版）
 * 
 * POC 阶段使用 JSON 格式，V2 阶段迁移到二进制格式。
 */
BOOL HiveManager::LoadHiveFromJson(const std::wstring& filePath) {
    // 打开文件
    std::wifstream file(filePath);
    if (!file.is_open()) {
        printf("[HiveManager] LoadHiveFromJson: Cannot open file: %ls\n", filePath.c_str());
        return FALSE;
    }
    
    // TODO: V2 实现完整的 JSON 解析
    // POC 阶段：仅清空内存中的 Hive，重新开始
    root_key_.subKeys.clear();
    root_key_.values.clear();
    
    file.close();
    
    printf("[HiveManager] LoadHiveFromJson: Hive loaded from JSON (stub): %ls\n", filePath.c_str());
    
    return TRUE;
}

/**
 * @brief 写入 JSON 键值对（辅助函数）
 */
void HiveManager::WriteJsonKeyValue(std::wofstream& file, const std::wstring& key,
                                    const std::wstring& value, int indent) {
    std::wstring indentStr(indent * 2, L' ');
    file << indentStr << L"\"" << EscapeJsonString(key) << L"\": \"" << EscapeJsonString(value) << L"\"";
}

/**
 * @brief 写入 JSON 键（辅助函数）
 */
void HiveManager::WriteJsonKey(std::wofstream& file, const std::wstring& key, int indent) {
    std::wstring indentStr(indent * 2, L' ');
    file << indentStr << L"\"" << EscapeJsonString(key) << L"\": ";
}

/**
 * @brief 写入 JSON 值（辅助函数）
 */
void HiveManager::WriteJsonValue(std::wofstream& file, const RegValue& value) {
    // 简化版：仅支持字符串类型
    if (value.type == REG_SZ && !value.data.empty()) {
        std::wstring strValue(reinterpret_cast<const wchar_t*>(value.data.data()), 
                               value.data.size() / sizeof(wchar_t));
        file << L"\"" << EscapeJsonString(strValue) << L"\"";
    } else {
        file << L"null";
    }
}

/**
 * @brief 转义 JSON 字符串
 */
std::wstring HiveManager::EscapeJsonString(const std::wstring& str) {
    std::wstring result;
    for (wchar_t c : str) {
        switch (c) {
            case L'\\': result += L"\\\\"; break;
            case L'"': result += L"\\\""; break;
            case L'\n': result += L"\\n"; break;
            case L'\r': result += L"\\r"; break;
            case L'\t': result += L"\\t"; break;
            default: result += c; break;
        }
    }
    return result;
}

} // namespace VReg
} // namespace Engine
} // namespace ThinApp

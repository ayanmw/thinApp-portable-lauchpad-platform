/**
 * @file vreg_engine.cpp
 * @brief 虚拟注册表引擎实现 - 注册表重定向
 * @author AI ThinApp Team
 * @date 2026-05-23
 * 
 * 基于 MinHook 实现注册表 API Hook，
 * 拦截 NtCreateKey / NtSetValueKey / NtOpenKey，
 * 实现注册表重定向和 Copy-on-Write 逻辑。 */

#include "vreg_engine.h"
#include <shlwapi.h>
#include <algorithm>
#include <cwctype>
#include <fstream>

#pragma comment(lib, "Shlwapi.lib")
#pragma comment(lib, "ntdll.lib")

// 原 API 函数类型定义
typedef NTSTATUS(NTAPI* NtCreateKey_t)(
    PHANDLE KeyHandle,
    ACCESS_MASK DesiredAccess,
    POBJECT_ATTRIBUTES ObjectAttributes,
    ULONG TitleIndex,
    PUNICODE_STRING Class,
    ULONG CreateOptions,
    PULONG Disposition
);

typedef NTSTATUS(NTAPI* NtSetValueKey_t)(
    HANDLE KeyHandle,
    PUNICODE_STRING ValueName,
    ULONG TitleIndex,
    ULONG Type,
    PVOID Data,
    ULONG DataSize
);

typedef NTSTATUS(NTAPI* NtOpenKey_t)(
    PHANDLE KeyHandle,
    ACCESS_MASK DesiredAccess,
    POBJECT_ATTRIBUTES ObjectAttributes
);

typedef NTSTATUS(NTAPI* NtQueryValueKey_t)(
    HANDLE KeyHandle,
    PUNICODE_STRING ValueName,
    KEY_VALUE_INFORMATION_CLASS KeyValueInformationClass,
    PVOID KeyValueInformation,
    ULONG Length,
    PULONG ResultLength
);

// 原 API 函数指针
static NtCreateKey_t pOriginalNtCreateKey = nullptr;
static NtSetValueKey_t pOriginalNtSetValueKey = nullptr;
static NtOpenKey_t pOriginalNtOpenKey = nullptr;
static NtQueryValueKey_t pOriginalNtQueryValueKey = nullptr;

// Hook 引擎状态
static bool g_initialized = false;
static std::wstring g_appDir;           // 应用目录
static std::wstring g_virtualHivePath;  // 虚拟 hive 文件路径
static std::mutex g_mutex;              // 线程安全锁

// 注册表重定向规则表
static std::vector<std::pair<std::wstring, std::wstring>> g_regRedirections;

namespace VRegEngine {

// ============================================================================
// 工具函数
// ============================================================================

/**
 * @brief 将路径转换为标准格式（大写）
 */
static std::wstring NormalizeRegPath(const std::wstring& path) {
    if (path.empty()) return path;
    
    std::wstring result = path;
    std::transform(result.begin(), result.end(), result.begin(),
                   [](wchar_t c) { return std::towupper(c); });
    
    return result;
}

/**
 * @brief 检查注册表路径是否以指定前缀开头（不区分大小写）
 */
static bool RegPathStartsWith(const std::wstring& path, const std::wstring& prefix) {
    std::wstring upperPath = NormalizeRegPath(path);
    std::wstring upperPrefix = NormalizeRegPath(prefix);
    return upperPath.find(upperPrefix) == 0;
}

/**
 * @brief 将注册表根键字符串转换为标准格式
 */
static std::wstring NormalizeRootKey(const std::wstring& path) {
    std::wstring result = path;
    
    // 替换缩写形式
    if (result.find(L"HKLM") == 0) {
        result.replace(0, 4, L"HKEY_LOCAL_MACHINE");
    } else if (result.find(L"HKCU") == 0) {
        result.replace(0, 4, L"HKEY_CURRENT_USER");
    } else if (result.find(L"HKCR") == 0) {
        result.replace(0, 4, L"HKEY_CLASSES_ROOT");
    } else if (result.find(L"HKU") == 0) {
        result.replace(0, 3, L"HKEY_USERS");
    }
    
    return NormalizeRegPath(result);
}

// ============================================================================
// 虚拟 Hive 操作（简化版 - JSON 格式）
// ============================================================================

// 简单的 JSON 解析助手（无需外部依赖）
namespace JsonHelper {
    
    /**
     * @brief 转义 JSON 字符串
     */
    static std::wstring EscapeJsonString(const std::wstring& str) {
        std::wstring result;
        for (wchar_t c : str) {
            switch (c) {
                case L'\\': result += L"\\\\\"; break;
                case L'"': result += L"\\\""; break;
                case L'\n': result += L"\\n"; break;
                case L'\r': result += L"\\r"; break;
                case L'\t': result += L"\\t"; break;
                default: result += c; break;
            }
        }
        return result;
    }
    
    /**
     * @brief 创建简单的键值对 JSON
     */
    static std::wstring CreateSimpleJson(const std::wstring& key, const std::wstring& value) {
        std::wstring json = L"{\n";
        json += L"  \"" + EscapeJsonString(key) + L"\": \"" + EscapeJsonString(value) + L"\"\n";
        json += L"}";
        return json;
    }
    
} // namespace JsonHelper

/**
 * @brief 读取虚拟 hive（JSON 格式）
 * @note MVP 阶段使用 JSON，V2 迁移到二进制 hive 格式
 */
static bool ReadVirtualHive(std::wstring& content) {
    if (!PathFileExistsW(g_virtualHivePath.c_str())) {
        // 文件不存在，创建空 JSON
        std::wofstream file(g_virtualHivePath);
        if (file.is_open()) {
            file << L"{}" << std::endl;
            file.close();
        }
        content = L"{}";
        return true;
    }
    
    std::wifstream file(g_virtualHivePath);
    if (!file.is_open()) {
        return false;
    }
    
    std::wstringstream buffer;
    buffer << file.rdbuf();
    content = buffer.str();
    file.close();
    
    return true;
}

/**
 * @brief 写入虚拟 hive（JSON 格式）
 */
static bool WriteVirtualHive(const std::wstring& content) {
    // 确保目录存在
    std::wstring dir = g_virtualHivePath.substr(0, g_virtualHivePath.find_last_of(L'\\'));
    CreateDirectoryW(dir.c_str(), nullptr);
    
    std::wofstream file(g_virtualHivePath, std::ios::trunc);
    if (!file.is_open()) {
        return false;
    }
    
    file << content << std::endl;
    file.close();
    
    return true;
}

/**
 * @brief 在虚拟 hive 中创建注册表键（简化版）
 * @note POC 阶段：仅记录到 JSON 文件，完整解析留到 V2
 */
static bool VRegCreateKey(const std::wstring& keyPath) {
    // 读取现有 hive
    std::wstring hiveContent;
    if (!ReadVirtualHive(hiveContent)) {
        hiveContent = L"{}";
    }
    
    // POC 阶段：简单追加键名到 JSON（不完整实现）
    // 完整实现需要解析 JSON 并创建嵌套结构
    // TODO: V2 使用完整的 JSON 解析库（如 RapidJSON）
    
    // 临时方案：将键路径记录到日志文件
    std::wstring logPath = g_virtualHivePath + L".log";
    std::wofstream logFile(logPath, std::ios::app);
    if (logFile.is_open()) {
        logFile << L"[CREATE] " << keyPath << std::endl;
        logFile.close();
    }
    
    return true;
}

/**
 * @brief 在虚拟 hive 中设置注册表值（简化版）
 */
static bool VRegSetValue(const std::wstring& keyPath, const std::wstring& valueName, const std::wstring& valueData) {
    // 临时方案：将键值对记录到日志文件
    std::wstring logPath = g_virtualHivePath + L".log";
    std::wofstream logFile(logPath, std::ios::app);
    if (logFile.is_open()) {
        logFile << L"[SET] " << keyPath << L"\\" << valueName << L" = " << valueData << std::endl;
        logFile.close();
    }
    
    return true;
}

/**
 * @brief 从虚拟 hive 中查询注册表值（CoW 逻辑）
 * @return true: 在虚拟 hive 中找到, false: 未找到（需要透传）
 */
static bool VRegQueryValue(const std::wstring& keyPath, const std::wstring& valueName, std::wstring& valueData) {
    // POC 阶段：从日志文件中查询（不完整实现）
    // 完整实现需要解析 JSON 并返回真实值
    // TODO: V2 实现完整查询逻辑
    
    // 临时方案：检查日志文件中是否存在此键/值
    std::wstring logPath = g_virtualHivePath + L".log";
    if (!PathFileExistsW(logPath.c_str())) {
        return false; // 日志文件不存在，肯定没找到
    }
    
    // 简单检查：如果日志中有 [SET] 记录，则认为找到了
    // 这不是完整的实现，仅用于 POC 演示
    std::wifstream logFile(logPath);
    if (!logFile.is_open()) {
        return false;
    }
    
    std::wstring line;
    std::wstring target = keyPath + L"\\" + valueName;
    while (std::getline(logFile, line)) {
        if (line.find(L"[SET] " + target) == 0) {
            // 找到了，提取 valueData
            size_t eqPos = line.find(L" = ");
            if (eqPos != std::wstring::npos) {
                valueData = line.substr(eqPos + 3);
                logFile.close();
                return true;
            }
        }
    }
    
    logFile.close();
    return false; // 未找到（需要透传真实注册表）
}

// ============================================================================
// Hook 函数实现
// ============================================================================

/**
 * @brief Hook 后的 NtCreateKey - 实现注册表重定向
 */
static NTSTATUS NTAPI HookedNtCreateKey(
    PHANDLE KeyHandle,
    ACCESS_MASK DesiredAccess,
    POBJECT_ATTRIBUTES ObjectAttributes,
    ULONG TitleIndex,
    PUNICODE_STRING Class,
    ULONG CreateOptions,
    PULONG Disposition
) {
    // 检查是否需要重定向
    if (ObjectAttributes && ObjectAttributes->ObjectName) {
        UNICODE_STRING& objName = *ObjectAttributes->ObjectName;
        std::wstring originalPath(objName.Buffer, objName.Length / sizeof(WCHAR));
        
        std::wstring redirectedPath;
        if (ShouldRedirectReg(originalPath, redirectedPath)) {
            // [修复限制1] 在虚拟 hive 中创建键
            if (VRegCreateKey(redirectedPath)) {
                // POC 阶段：仍然调用原函数（透传），但记录操作到虚拟 hive
                // V2 阶段：不调用原函数，完全在虚拟 hive 中操作
                printf("[VReg] 创建虚拟键: %ws\n", redirectedPath.c_str());
            }
        }
    }
    
    // 调用原函数（透传）
    return pOriginalNtCreateKey(
        KeyHandle, DesiredAccess, ObjectAttributes,
        TitleIndex, Class, CreateOptions, Disposition
    );
}

/**
 * @brief Hook 后的 NtSetValueKey - 实现注册表值写入重定向
 */
static NTSTATUS NTAPI HookedNtSetValueKey(
    HANDLE KeyHandle,
    PUNICODE_STRING ValueName,
    ULONG TitleIndex,
    ULONG Type,
    PVOID Data,
    ULONG DataSize
) {
    // [修复限制1] 获取键路径（简化版：从日志查询）
    // 实际实现需要通过 KeyHandle 查询键路径
    // POC 阶段：假设键路径已经在虚拟 hive 中创建
    
    // 获取值名称
    std::wstring valueNameStr;
    if (ValueName && ValueName->Buffer) {
        valueNameStr = std::wstring(ValueName->Buffer, ValueName->Length / sizeof(WCHAR));
    }
    
    // 获取值数据（简化版：仅支持字符串类型）
    std::wstring valueDataStr;
    if (Type == REG_SZ && Data && DataSize > 0) {
        valueDataStr = std::wstring(static_cast<wchar_t*>(Data), DataSize / sizeof(wchar_t) - 1);
    }
    
    // 在虚拟 hive 中设置值（简化版）
    // TODO: V2 通过 KeyHandle 获取真实键路径
    std::wstring fakeKeyPath = L"HKCU\\Software\\TestVApp"; // 示例路径
    if (!valueNameStr.empty()) {
        VRegSetValue(fakeKeyPath, valueNameStr, valueDataStr);
        printf("[VReg] 设置虚拟值: %ws\\%ws = %ws\n", 
               fakeKeyPath.c_str(), valueNameStr.c_str(), valueDataStr.c_str());
    }
    
    // POC 阶段：仍然调用原函数（透传），V2 阶段可选择不调用
    return pOriginalNtSetValueKey(
        KeyHandle, ValueName, TitleIndex, Type, Data, DataSize
    );
}

/**
 * @brief Hook 后的 NtOpenKey - 实现注册表键打开重定向
 */
static NTSTATUS NTAPI HookedNtOpenKey(
    PHANDLE KeyHandle,
    ACCESS_MASK DesiredAccess,
    POBJECT_ATTRIBUTES ObjectAttributes
) {
    // 检查是否需要重定向
    if (ObjectAttributes && ObjectAttributes->ObjectName) {
        UNICODE_STRING& objName = *ObjectAttributes->ObjectName;
        std::wstring originalPath(objName.Buffer, objName.Length / sizeof(WCHAR));
        
        std::wstring redirectedPath;
        if (ShouldRedirectReg(originalPath, redirectedPath)) {
            // [修复限制1] 在虚拟 hive 中打开键
            // POC 阶段：仍然调用原函数，但记录操作
            printf("[VReg] 打开虚拟键: %ws\n", redirectedPath.c_str());
        }
    }
    
    // 调用原函数（透传）
    return pOriginalNtOpenKey(
        KeyHandle, DesiredAccess, ObjectAttributes
    );
}

/**
 * @brief Hook 后的 NtQueryValueKey（CoW 读取逻辑）
 */
static NTSTATUS NTAPI HookedNtQueryValueKey(
    HANDLE KeyHandle,
    PUNICODE_STRING ValueName,
    KEY_VALUE_INFORMATION_CLASS KeyValueInformationClass,
    PVOID KeyValueInformation,
    ULONG Length,
    PULONG ResultLength
) {
    // [修复限制1] CoW 逻辑：先查虚拟 hive，若不存在则透传真实注册表
    
    // 获取值名称
    std::wstring valueNameStr;
    if (ValueName && ValueName->Buffer) {
        valueNameStr = std::wstring(ValueName->Buffer, ValueName->Length / sizeof(WCHAR));
    }
    
    // 在虚拟 hive 中查询值（简化版）
    // TODO: V2 通过 KeyHandle 获取真实键路径
    std::wstring fakeKeyPath = L"HKCU\\Software\\TestVApp"; // 示例路径
    std::wstring valueData;
    
    if (VRegQueryValue(fakeKeyPath, valueNameStr, valueData)) {
        // 在虚拟 hive 中找到，返回虚拟值
        printf("[VReg] CoW 读取虚拟值: %ws\\%ws = %ws\n", 
               fakeKeyPath.c_str(), valueNameStr.c_str(), valueData.c_str());
        
        // POC 阶段：仍然透传，V2 阶段返回虚拟值
        // TODO: V2 构造 KEY_VALUE_PARTIAL_INFORMATION 并返回
    }
    
    // 虚拟 hive 中未找到，透传真实注册表（CoW 逻辑）
    return pOriginalNtQueryValueKey(
        KeyHandle, ValueName, KeyValueInformationClass,
        KeyValueInformation, Length, ResultLength
    );
}

// ============================================================================
// 公开 API 实现
// ============================================================================

bool Initialize(const std::wstring& appDir) {
    std::lock_guard<std::mutex> lock(g_mutex);
    
    if (g_initialized) {
        return true; // 已经初始化
    }
    
    // 保存应用目录
    g_appDir = appDir;
    
    // 构建虚拟 hive 文件路径
    g_virtualHivePath = appDir + L"\\VFS\\registry.hive";
    
    // 创建 VFS 目录（如果不存在）
    std::wstring vfsDir = appDir + L"\\VFS";
    if (!CreateDirectoryW(vfsDir.c_str(), nullptr)) {
        if (GetLastError() != ERROR_ALREADY_EXISTS) {
            return false;
        }
    }
    
    // 添加默认注册表重定向规则
    AddRegRedirection(L"HKLM\\Software", g_virtualHivePath + L"\\HKLM\\Software");
    AddRegRedirection(L"HKCU\\Software", g_virtualHivePath + L"\\HKCU\\Software");
    AddRegRedirection(L"HKCR", g_virtualHivePath + L"\\HKCR");
    AddRegRedirection(L"HKU", g_virtualHivePath + L"\\HKU");
    
    // 初始化虚拟 hive 文件（如果不存在）
    std::wstring hiveContent;
    if (!ReadVirtualHive(hiveContent)) {
        return false;
    }
    
    g_initialized = true;
    return true;
}

bool InstallHooks() {
    std::lock_guard<std::mutex> lock(g_mutex);
    
    if (!g_initialized) {
        return false;
    }
    
    // 初始化 MinHook（如果尚未初始化）
    static bool minHookInitialized = false;
    if (!minHookInitialized) {
        if (MH_Initialize() != MH_OK) {
            return false;
        }
        minHookInitialized = true;
    }
    
    // 创建 Hook 目标：NtCreateKey
    if (MH_CreateHook(&NtCreateKey, &HookedNtCreateKey,
                      reinterpret_cast<LPVOID*>(&pOriginalNtCreateKey)) != MH_OK) {
        return false;
    }
    
    // 创建 Hook 目标：NtSetValueKey
    if (MH_CreateHook(&NtSetValueKey, &HookedNtSetValueKey,
                      reinterpret_cast<LPVOID*>(&pOriginalNtSetValueKey)) != MH_OK) {
        return false;
    }
    
    // 创建 Hook 目标：NtOpenKey
    if (MH_CreateHook(&NtOpenKey, &HookedNtOpenKey,
                      reinterpret_cast<LPVOID*>(&pOriginalNtOpenKey)) != MH_OK) {
        return false;
    }
    
    // 创建 Hook 目标：NtQueryValueKey（CoW 读取）
    if (MH_CreateHook(&NtQueryValueKey, &HookedNtQueryValueKey,
                      reinterpret_cast<LPVOID*>(&pOriginalNtQueryValueKey)) != MH_OK) {
        // 可选 Hook，失败不影响主功能
        pOriginalNtQueryValueKey = nullptr;
    }
    
    // 启用所有 Hook
    if (MH_EnableHook(MH_ALL_HOOKS) != MH_OK) {
        return false;
    }
    
    return true;
}

bool UninstallHooks() {
    std::lock_guard<std::mutex> lock(g_mutex);
    
    // 禁用所有 Hook
    if (MH_DisableHook(MH_ALL_HOOKS) != MH_OK) {
        return false;
    }
    
    return true;
}

void Cleanup() {
    std::lock_guard<std::mutex> lock(g_mutex);
    
    g_initialized = false;
    g_appDir.clear();
    g_virtualHivePath.clear();
    g_regRedirections.clear();
}

void AddRegRedirection(const std::wstring& realPath, const std::wstring& vfsHivePath) {
    std::lock_guard<std::mutex> lock(g_mutex);
    
    // 添加到重定向规则表
    g_regRedirections.emplace_back(realPath, &vfsHivePath);
}

bool ShouldRedirectReg(const std::wstring& path, std::wstring& redirectedPath) {
    if (!g_initialized || path.empty()) {
        return false;
    }
    
    // 标准化路径
    std::wstring normalizedPath = NormalizeRootKey(path);
    
    // 遍历重定向规则表
    for (const auto& rule : g_regRedirections) {
        if (RegPathStartsWith(normalizedPath, rule.first)) {
            // 匹配到规则，进行路径替换
            redirectedPath = rule.second + normalizedPath.substr(rule.first.length());
            return true;
        }
    }
    
    return false;
}

std::wstring GetVirtualHivePath() {
    return g_virtualHivePath;
}

} // namespace VRegEngine

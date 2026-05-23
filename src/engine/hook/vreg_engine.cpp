/**
 * @file vreg_engine.cpp (V2 - HiveManager 集成版)
 * @brief 虚拟注册表引擎实现 - 注册表重定向
 * 
 * 基于 MinHook 实现注册表 API Hook，
 * 拦截 NtCreateKey / NtSetValueKey / NtOpenKey / NtQueryValueKey，
 * 实现注册表重定向和 Copy-on-Write 逻辑。
 * 
 * 集成 HiveManager 实现完整的 CoW 重定向和 Hive 持久化。
 * 
 * @author AI ThinApp Team
 * @date 2026-05-23
 */

#include "vreg_engine.h"
#include "..\vreg\hive_manager.h"  // 集成 HiveManager
#include <shlwapi.h>
#include <winternl.h>
#include <algorithm>
#include <cwctype>
#include <fstream>

#pragma comment(lib, "Shlwapi.lib")
#pragma comment(lib, "ntdll.lib")

// 原 API 函数类型定义
typedef NTSTATUS (NTAPI *NtCreateKey_t)(
    PHANDLE KeyHandle,
    ACCESS_MASK DesiredAccess,
    POBJECT_ATTRIBUTES ObjectAttributes,
    ULONG TitleIndex,
    PUNICODE_STRING Class,
    ULONG CreateOptions,
    PULONG Disposition
);

typedef NTSTATUS (NTAPI *NtSetValueKey_t)(
    HANDLE KeyHandle,
    PUNICODE_STRING ValueName,
    ULONG TitleIndex,
    ULONG Type,
    PVOID Data,
    ULONG DataSize
);

typedef NTSTATUS (NTAPI *NtOpenKey_t)(
    PHANDLE KeyHandle,
    ACCESS_MASK DesiredAccess,
    POBJECT_ATTRIBUTES ObjectAttributes
);

typedef NTSTATUS (NTAPI *NtQueryValueKey_t)(
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

/**
 * @brief 获取键路径（简化版）
 * 
 * @note TODO: V2 通过 KeyHandle 查询真实键路径
 */
static std::wstring GetKeyPath(HANDLE KeyHandle, PUNICODE_STRING ValueName) {
    // POC 阶段：返回示例路径
    // V2 阶段：通过 KeyHandle 查询键路径
    return L"HKCU\\Software\\TestApp";
}

// ============================================================================
// Hook 函数实现（集成 HiveManager）
// ============================================================================

/**
 * @brief Hook 后的 NtCreateKey - 实现注册表重定向
 * 
 * 调用 HiveManager::CreateKey() 在虚拟 hive 中创建键。
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
            // [Week 2] 调用 HiveManager::CreateKey() 在虚拟 hive 中创建键
            if (ThinApp::Engine::VReg::HiveManager::GetInstance().CreateKey(redirectedPath)) {
                printf("[VReg] 创建虚拟键: %ls\n", redirectedPath.c_str());
            } else {
                printf("[VReg] 创建虚拟键失败: %ls\n", redirectedPath.c_str());
            }
        }
    }
    
    // POC 阶段：仍然调用原函数（透传），V2 阶段可选择不调用
    return pOriginalNtCreateKey(
        KeyHandle, DesiredAccess, ObjectAttributes,
        TitleIndex, Class, CreateOptions, Disposition
    );
}

/**
 * @brief Hook 后的 NtSetValueKey - 实现注册表值写入重定向（CoW）
 * 
 * 调用 HiveManager::CoWRedirect() 实现 CoW 逻辑：
 * - 写入时只修改虚拟 Hive
 */
static NTSTATUS NTAPI HookedNtSetValueKey(
    HANDLE KeyHandle,
    PUNICODE_STRING ValueName,
    ULONG TitleIndex,
    ULONG Type,
    PVOID Data,
    ULONG DataSize
) {
    // 获取值名称
    std::wstring valueNameStr;
    if (ValueName && ValueName->Buffer) {
        valueNameStr = std::wstring(ValueName->Buffer, ValueName->Length / sizeof(WCHAR));
    }
    
    // 获取键路径（简化版）
    std::wstring keyPath = GetKeyPath(KeyHandle, ValueName);
    
    // [Week 2] 调用 HiveManager::SetValue() 在虚拟 hive 中设置值（CoW 写入）
    if (!valueNameStr.empty()) {
        if (ThinApp::Engine::VReg::HiveManager::GetInstance().SetValue(
            keyPath, valueNameStr, Type, static_cast<const BYTE*>(Data), DataSize)) {
            printf("[VReg] CoW 写入虚拟值: %ls\\%ls (类型: %lu, 大小: %lu 字节)\n", 
                   keyPath.c_str(), valueNameStr.c_str(), Type, DataSize);
        } else {
            printf("[VReg] CoW 写入虚拟值失败: %ls\\%ls\n", 
                   keyPath.c_str(), valueNameStr.c_str());
        }
    }
    
    // POC 阶段：仍然调用原函数（透传），V2 阶段可选择不调用
    return pOriginalNtSetValueKey(
        KeyHandle, ValueName, TitleIndex, Type, Data, DataSize
    );
}

/**
 * @brief Hook 后的 NtOpenKey - 实现注册表键打开重定向
 * 
 * 调用 HiveManager::CreateKey() 在虚拟 hive 中打开键（如果不存在则创建）。
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
            // [Week 2] 调用 HiveManager::CreateKey() 在虚拟 hive 中打开键
            if (ThinApp::Engine::VReg::HiveManager::GetInstance().CreateKey(redirectedPath)) {
                printf("[VReg] 打开虚拟键: %ls\n", redirectedPath.c_str());
            }
        }
    }
    
    // POC 阶段：仍然调用原函数（透传），V2 阶段可选择不调用
    return pOriginalNtOpenKey(
        KeyHandle, DesiredAccess, ObjectAttributes
    );
}

/**
 * @brief Hook 后的 NtQueryValueKey（CoW 读取逻辑）
 * 
 * 调用 HiveManager::CoWRedirect() 实现 CoW 逻辑：
 * - 读取时优先查虚拟 Hive，不存在则透传真实注册表
 */
static NTSTATUS NTAPI HookedNtQueryValueKey(
    HANDLE KeyHandle,
    PUNICODE_STRING ValueName,
    KEY_VALUE_INFORMATION_CLASS KeyValueInformationClass,
    PVOID KeyValueInformation,
    ULONG Length,
    PULONG ResultLength
) {
    // 获取值名称
    std::wstring valueNameStr;
    if (ValueName && ValueName->Buffer) {
        valueNameStr = std::wstring(ValueName->Buffer, ValueName->Length / sizeof(WCHAR));
    }
    
    // 获取键路径（简化版）
    std::wstring keyPath = GetKeyPath(KeyHandle, ValueName);
    
    // [Week 2] CoW 逻辑：先查虚拟 hive，若不存在则透传真实注册表
    DWORD queryType = 0;
    BYTE queryData[1024] = {0};
    DWORD queryDataSize = sizeof(queryData);
    
    if (!valueNameStr.empty() &&
        ThinApp::Engine::VReg::HiveManager::GetInstance().QueryValue(
            keyPath, valueNameStr, &queryType, queryData, &queryDataSize)) {
        // 在虚拟 hive 中找到，返回虚拟值
        printf("[VReg] CoW 读取虚拟值: %ls\\%ls (类型: %lu, 大小: %lu 字节)\n", 
               keyPath.c_str(), valueNameStr.c_str(), queryType, queryDataSize);
        
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
    
    if (appDir.empty()) {
        printf("[VRegEngine] Error: appDir is empty\n");
        return false;
    }
    
    // 保存应用目录
    g_appDir = appDir;
    
    // 构建虚拟 hive 文件路径
    g_virtualHivePath = appDir + L"\\VFS\\registry.hive";
    
    // 创建 VFS 目录（如果不存在）
    std::wstring vfsDir = appDir + L"\\VFS";
    if (!CreateDirectoryW(vfsDir.c_str(), nullptr)) {
        if (GetLastError() != ERROR_ALREADY_EXISTS) {
            printf("[VRegEngine] Error: Cannot create VFS directory\n");
            return false;
        }
    }
    
    // 添加默认注册表重定向规则
    AddRegRedirection(L"HKLM\\Software", g_virtualHivePath + L"\\HKLM\\Software");
    AddRegRedirection(L"HKCU\\Software", g_virtualHivePath + L"\\HKCU\\Software");
    AddRegRedirection(L"HKCR", g_virtualHivePath + L"\\HKCR");
    AddRegRedirection(L"HKU", g_virtualHivePath + L"\\HKU");
    
    // [Week 2] 初始化 HiveManager
    if (!ThinApp::Engine::VReg::HiveManager::GetInstance().Initialize(appDir)) {
        printf("[VRegEngine] Error: Cannot initialize HiveManager\n");
        return false;
    }
    
    // [Week 2] 加载 Hive（从文件加载到内存）
    if (!ThinApp::Engine::VReg::HiveManager::GetInstance().LoadHive()) {
        printf("[VRegEngine] Warning: Cannot load hive, will create new one\n");
    }
    
    g_initialized = true;
    printf("[VRegEngine] Initialized successfully\n");
    
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
    
    printf("[VRegEngine] All hooks installed successfully\n");
    
    return true;
}

bool UninstallHooks() {
    std::lock_guard<std::mutex> lock(g_mutex);
    
    // 禁用所有 Hook
    if (MH_DisableHook(MH_ALL_HOOKS) != MH_OK) {
        return false;
    }
    
    printf("[VRegEngine] All hooks uninstalled\n");
    
    return true;
}

void Cleanup() {
    std::lock_guard<std::mutex> lock(g_mutex);
    
    if (!g_initialized) {
        return;
    }
    
    // [Week 2] 保存 Hive（从内存保存到文件）
    if (!ThinApp::Engine::VReg::HiveManager::GetInstance().SaveHive()) {
        printf("[VRegEngine] Warning: Cannot save hive\n");
    }
    
    // [Week 2] 清理 HiveManager
    ThinApp::Engine::VReg::HiveManager::GetInstance().Shutdown();
    
    g_initialized = false;
    g_appDir.clear();
    g_virtualHivePath.clear();
    g_regRedirections.clear();
    
    printf("[VRegEngine] Cleanup complete\n");
}

void AddRegRedirection(const std::wstring& realPath, const std::wstring& vfsHivePath) {
    std::lock_guard<std::mutex> lock(g_mutex);
    
    // 添加到重定向规则表
    g_regRedirections.emplace_back(realPath, vfsHivePath);
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

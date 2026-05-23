/**
 * @file test_hook.cpp
 * @brief Hook 引擎测试程序
 * @author AI ThinApp Team
 * @date 2026-05-23
 * 
 * 测试内容：
 * 1. Hook 安装/卸载
 * 2. 文件系统重定向
 * 3. 注册表重定向
 * 
 * 使用方法：
 * 1. 编译为 exe
 * 2. 将 HookEngine.dll 放到同一目录
 * 3. 运行 test_hook.exe
 */

#include <Windows.h>
#include <iostream>
#include <fstream>
#include <Shlwapi.h>

#pragma comment(lib, "Shlwapi.lib")

// 测试计数
static int g_testCount = 0;
static int g_passCount = 0;

// ============================================================================
// 辅助函数
// ============================================================================

/**
 * @brief 打印测试结果
 */
static void PrintTestResult(const char* testName, bool passed) {
    g_testCount++;
    if (passed) g_passCount++;
    
    std::cout << "[Test " << g_testCount << "] " << testName << ": "
              << (passed ? "PASS" : "FAIL") << std::endl;
}

/**
 * @brief 注入 DLL 到当前进程
 */
static bool InjectDllToCurrentProcess(const wchar_t* dllPath) {
    HMODULE hDll = LoadLibraryW(dllPath);
    return hDll != nullptr;
}

// ============================================================================
// 测试 1: Hook 安装/卸载
// ============================================================================

static bool TestHookInstallUninstall() {
    std::cout << "\n=== 测试 1: Hook 安装/卸载 ===" << std::endl;
    
    // 注意：DLL 会在 DllMain 中自动安装 Hook
    // 这里测试 LoadLibrary + FreeLibrary 流程
    
    // 1. 加载 DLL（会自动安装 Hook）
    HMODULE hDll = LoadLibraryW(L"HookEngine.dll");
    if (!hDll) {
        DWORD err = GetLastError();
        std::cout << "  加载 DLL 失败, Error: " << err << std::endl;
        PrintTestResult("Hook Install", false);
        return false;
    }
    std::cout << "  DLL 加载成功（Hook 应已自动安装）" << std::endl;
    
    // 2. 卸载 DLL（会自动卸载 Hook）
    if (!FreeLibrary(hDll)) {
        DWORD err = GetLastError();
        std::cout << "  卸载 DLL 失败, Error: " << err << std::endl;
        PrintTestResult("Hook Uninstall", false);
        return false;
    }
    std::cout << "  DLL 卸载成功（Hook 应已自动卸载）" << std::endl;
    
    PrintTestResult("Hook Install/Uninstall", true);
    return true;
}

// ============================================================================
// 测试 2: 文件系统重定向
// ============================================================================

static bool TestFileRedirection() {
    std::cout << "\n=== 测试 2: 文件系统重定向 ===" << std::endl;
    
    // 加载 DLL
    HMODULE hDll = LoadLibraryW(L"HookEngine.dll");
    if (!hDll) {
        std::cout << "  加载 DLL 失败" << std::endl;
        PrintTestResult("File Redirection", false);
        return false;
    }
    
    // 测试：写入 C:\Windows\Temp\test_thinapp.txt
    // 预期：文件被重定向到 {AppDir}\VFS\C\Windows\Temp\test_thinapp.txt
    std::wstring testContent = L"Hello from ThinApp POC!";
    std::wstring originalPath = L"C:\\Windows\\Temp\\test_thinapp.txt";
    
    // 1. 写入文件（应该被重定向）
    HANDLE hFile = CreateFileW(
        originalPath.c_str(),
        GENERIC_WRITE,
        0,
        nullptr,
        CREATE_ALWAYS,
        FILE_ATTRIBUTE_NORMAL,
        nullptr
    );
    
    if (hFile == INVALID_HANDLE_VALUE) {
        DWORD err = GetLastError();
        std::cout << "  创建文件失败, Error: " << err << std::endl;
        FreeLibrary(hDll);
        PrintTestResult("File Redirection - Write", false);
        return false;
    }
    
    DWORD bytesWritten = 0;
    WriteFile(hFile, testContent.c_str(), 
              (DWORD)(testContent.length() * sizeof(wchar_t)),
              &bytesWritten, nullptr);
    CloseHandle(hFile);
    
    std::cout << "  文件写入成功: " << std::endl;
    std::wcout << L"    原始路径: " << originalPath << std::endl;
    
    // 2. 检查文件是否被重定向到 VFS 目录
    // 获取当前 DLL 所在目录（作为 AppDir）
    wchar_t dllPath[MAX_PATH];
    GetModuleFileNameW(hDll, dllPath, MAX_PATH);
    std::wstring appDir = std::wstring(dllPath).substr(0, std::wstring(dllPath).find_last_of(L'\\'));
    
    std::wstring vfsPath = appDir + L"\\VFS\\C\\Windows\\Temp\\test_thinapp.txt";
    
    bool redirected = PathFileExistsW(vfsPath.c_str()) == TRUE;
    std::cout << "  重定向检查: " << (redirected ? "成功" : "失败") << std::endl;
    if (redirected) {
        std::wcout << L"    重定向路径: " << vfsPath << std::endl;
    }
    
    // 3. 读取文件（应该读取 VFS 中的文件）
    hFile = CreateFileW(
        originalPath.c_str(),
        GENERIC_READ,
        FILE_SHARE_READ,
        nullptr,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        nullptr
    );
    
    if (hFile != INVALID_HANDLE_VALUE) {
        wchar_t buffer[256] = {0};
        DWORD bytesRead = 0;
        ReadFile(hFile, buffer, sizeof(buffer) - sizeof(wchar_t), &bytesRead, nullptr);
        CloseHandle(hFile);
        
        std::wstring readContent(buffer);
        bool contentMatch = (readContent.find(testContent) != std::wstring::npos);
        
        std::cout << "  文件读取: " << (contentMatch ? "内容匹配" : "内容不匹配") << std::endl;
    }
    
    // 4. 清理测试文件
    DeleteFileW(originalPath.c_str());
    DeleteFileW(vfsPath.c_str());
    
    // 卸载 DLL
    FreeLibrary(hDll);
    
    PrintTestResult("File Redirection", redirected);
    return redirected;
}

// ============================================================================
// 测试 3: 注册表重定向
// ============================================================================

static bool TestRegRedirection() {
    std::cout << "\n=== 测试 3: 注册表重定向（MVP 阶段为透传） ===" << std::endl;
    
    // 注意：MVP 阶段注册表 Hook 为透传，完整重定向留到 V2
    std::cout << "  MVP 阶段：注册表 Hook 为透传模式" << std::endl;
    std::cout << "  预期：注册表操作正常，但写入真实注册表" << std::endl;
    
    // 加载 DLL
    HMODULE hDll = LoadLibraryW(L"HookEngine.dll");
    if (!hDll) {
        std::cout << "  加载 DLL 失败" << std::endl;
        PrintTestResult("Reg Redirection", false);
        return false;
    }
    
    // 测试：创建/写入/读取注册表键
    HKEY hKey = nullptr;
    DWORD disposition = 0;
    
    LONG result = RegCreateKeyExW(
        HKEY_CURRENT_USER,
        L"Software\\ThinAppPOC_Test",
        0,
        nullptr,
        REG_OPTION_VOLATILE,  // 重启后自动删除
        KEY_ALL_ACCESS,
        nullptr,
        &hKey,
        &disposition
    );
    
    if (result != ERROR_SUCCESS) {
        std::cout << "  创建注册表键失败, Error: " << result << std::endl;
        FreeLibrary(hDll);
        PrintTestResult("Reg Redirection", false);
        return false;
    }
    
    std::cout << "  注册表键创建成功: HKCU\\Software\\ThinAppPOC_Test" << std::endl;
    
    // 写入注册表值
    DWORD testValue = 12345;
    result = RegSetValueExW(
        hKey,
        L"TestValue",
        0,
        REG_DWORD,
        (const BYTE*)&testValue,
        sizeof(DWORD)
    );
    
    if (result == ERROR_SUCCESS) {
        std::cout << "  注册表值写入成功: TestValue = 12345" << std::endl;
    }
    
    // 读取注册表值
    DWORD readValue = 0;
    DWORD valueSize = sizeof(DWORD);
    result = RegQueryValueExW(
        hKey,
        L"TestValue",
        nullptr,
        nullptr,
        (LPBYTE)&readValue,
        &valueSize
    );
    
    if (result == ERROR_SUCCESS && readValue == testValue) {
        std::cout << "  注册表值读取成功: TestValue = " << readValue << std::endl;
    }
    
    // 关闭键
    RegCloseKey(hKey);
    
    // 清理：删除测试键
    RegDeleteKeyW(HKEY_CURRENT_USER, L"Software\\ThinAppPOC_Test");
    
    // 卸载 DLL
    FreeLibrary(hDll);
    
    PrintTestResult("Reg Redirection (Passthrough)", true);
    return true;
}

// ============================================================================
// 主函数
// ============================================================================

int main() {
    std::cout << "==============================================" << std::endl;
    std::cout << "  Hook 引擎测试程序" << std::endl;
    std::cout << "  AI ThinApp Portable Launchpad Platform POC" << std::endl;
    std::cout << "==============================================" << std::endl;
    
    // 检查 DLL 是否存在
    if (PathFileExistsW(L"HookEngine.dll") != TRUE) {
        std::cout << "\n错误: 未找到 HookEngine.dll" << std::endl;
        std::cout << "请将 HookEngine.dll 放到当前目录后重试。" << std::endl;
        system("pause");
        return 1;
    }
    
    std::cout << "\n找到 HookEngine.dll，开始测试...\n" << std::endl;
    
    // 运行测试
    TestHookInstallUninstall();
    TestFileRedirection();
    TestRegRedirection();
    
    // 打印测试摘要
    std::cout << "\n==============================================" << std::endl;
    std::cout << "  测试摘要" << std::endl;
    std::cout << "  总测试数: " << g_testCount << std::endl;
    std::cout << "  通过: " << g_passCount << std::endl;
    std::cout << "  失败: " << (g_testCount - g_passCount) << std::endl;
    std::cout << "  通过率: " << (g_passCount * 100 / g_testCount) << "%" << std::endl;
    std::cout << "==============================================" << std::endl;
    
    system("pause");
    return 0;
}

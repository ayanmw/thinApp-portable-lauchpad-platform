/**
 * @file test_process.cpp
 * @brief 进程继承测试程序
 * @author AI ThinApp Team
 * @date 2026-05-23
 * 
 * 测试内容：
 * 1. 父进程 Hook 传递到子进程
 * 2. 验证 2 层子进程继承（A→B→C）
 * 
 * 使用方法：
 * 1. 编译为 exe
 * 2. 将 HookEngine.dll 放到同一目录
 * 3. 运行 test_process.exe
 */

#include <Windows.h>
#include <iostream>
#include <string>

// ============================================================================
// 辅助函数
// ============================================================================

/**
 * @brief 打印进程信息
 */
static void PrintProcessInfo(DWORD pid, const char* role) {
    std::cout << "  [" << role << "] PID = " << pid << std::endl;
}

/**
 * @brief 创建子进程（测试进程继承）
 * @param cmdLine 命令行
 * @param inheritHandle 是否继承句柄
 * @return 成功返回 true
 */
static bool CreateChildProcess(const wchar_t* cmdLine, bool inheritHandle = false) {
    STARTUPINFOW si = {0};
    PROCESS_INFORMATION pi = {0};
    si.cb = sizeof(si);
    
    // 复制命令行（CreateProcessW 会修改它）
    wchar_t* cmdLineCopy = _wcsdup(cmdLine);
    
    BOOL result = CreateProcessW(
        nullptr,           // 应用名称（从命令行解析）
        cmdLineCopy,      // 命令行
        nullptr,           // 进程属性
        nullptr,           // 线程属性
        inheritHandle,     // 是否继承句柄
        0,                // 创建标志（0 = 正常）
        nullptr,           // 环境变量
        nullptr,           // 当前目录
        &si,              // 启动信息
        &pi               // 进程信息
    );
    
    free(cmdLineCopy);
    
    if (!result) {
        DWORD err = GetLastError();
        std::cout << "  创建子进程失败, Error: " << err << std::endl;
        return false;
    }
    
    std::cout << "  子进程创建成功:" << std::endl;
    PrintProcessInfo(pi.dwProcessId, "Child");
    
    // 关闭句柄
    CloseHandle(pi.hThread);
    CloseHandle(pi.hProcess);
    
    return true;
}

// ============================================================================
// 测试 1: 子进程 Hook 继承
// ============================================================================

static bool TestChildProcessInheritance() {
    std::cout << "\n=== 测试 1: 子进程 Hook 继承 ===" << std::endl;
    
    // 加载 Hook DLL 到当前进程
    HMODULE hDll = LoadLibraryW(L"HookEngine.dll");
    if (!hDll) {
        std::cout << "  加载 HookEngine.dll 失败" << std::endl;
        return false;
    }
    
    std::cout << "  父进程 Hook 已安装" << std::endl;
    PrintProcessInfo(GetCurrentProcessId(), "Parent");
    
    // 创建子进程（应该自动注入 Hook DLL）
    std::cout << "\n  创建子进程（预期：自动注入 Hook DLL）..." << std::endl;
    
    // 子进程运行同样的测试程序，但传入参数表示自己是子进程
    wchar_t cmdLine[MAX_PATH];
    GetModuleFileNameW(nullptr, cmdLine, MAX_PATH);
    
    std::wstring childCmdLine = std::wstring(cmdLine) + L" --child";
    
    bool childCreated = CreateChildProcess(childCmdLine.c_str());
    
    if (childCreated) {
        std::cout << "\n  子进程创建成功，检查子进程是否加载了 HookEngine.dll" << std::endl;
        std::cout << "  验证方法：使用 Process Explorer 查看子进程的 DLL 列表" << std::endl;
    }
    
    // 卸载 DLL
    FreeLibrary(hDll);
    
    return childCreated;
}

// ============================================================================
// 测试 2: 2 层子进程继承（A→B→C）
// ============================================================================

static bool TestTwoLevelInheritance() {
    std::cout << "\n=== 测试 2: 2 层子进程继承（A→B→C） ===" << std::endl;
    
    // 加载 Hook DLL 到当前进程（A）
    HMODULE hDll = LoadLibraryW(L"HookEngine.dll");
    if (!hDll) {
        std::cout << "  加载 HookEngine.dll 失败" << std::endl;
        return false;
    }
    
    std::cout << "  进程 A（当前进程）Hook 已安装" << std::endl;
    PrintProcessInfo(GetCurrentProcessId(), "A");
    
    // 创建子进程 B（应该自动注入 Hook DLL）
    std::cout << "\n  创建进程 B..." << std::endl;
    
    wchar_t cmdLine[MAX_PATH];
    GetModuleFileNameW(nullptr, cmdLine, MAX_PATH);
    
    std::wstring childCmdLine = std::wstring(cmdLine) + L" --child-level2";
    
    bool childBCreated = CreateChildProcess(childCmdLine.c_str());
    
    if (childBCreated) {
        std::cout << "\n  进程 B 创建成功，进程 B 会继续创建进程 C" << std::endl;
        std::cout << "  预期：进程 C 也会加载 HookEngine.dll" << std::endl;
    }
    
    // 卸载 DLL
    FreeLibrary(hDll);
    
    return childBCreated;
}

// ============================================================================
// 子进程逻辑（当带 --child 参数运行时执行）
// ============================================================================

static void RunAsChildProcess() {
    std::cout << "\n  [子进程] 我是子进程" << std::endl;
    PrintProcessInfo(GetCurrentProcessId(), "Child");
    
    // 检查自己是否加载了 HookEngine.dll
    HMODULE hDll = GetModuleHandleW(L"HookEngine.dll");
    if (hDll) {
        std::cout << "  ✓ 子进程已加载 HookEngine.dll" << std::endl;
        std::cout << "  ✓ Hook 继承成功！" << std::endl;
        
        // 测试：在子进程中创建文件（应该被重定向）
        std::wstring testPath = L"C:\\Windows\\Temp\\test_child.txt";
        HANDLE hFile = CreateFileW(
            testPath.c_str(),
            GENERIC_WRITE,
            0,
            nullptr,
            CREATE_ALWAYS,
            FILE_ATTRIBUTE_NORMAL,
            nullptr
        );
        
        if (hFile != INVALID_HANDLE_VALUE) {
            std::wstring content = L"Written by child process";
            DWORD written = 0;
            WriteFile(hFile, content.c_str(), 
                     (DWORD)(content.length() * sizeof(wchar_t)),
                     &written, nullptr);
            CloseHandle(hFile);
            
            std::cout << "  ✓ 子进程文件写入成功（应被重定向）" << std::endl;
            std::wcout << L"    路径: " << testPath << std::endl;
        }
        
        FreeLibrary(hDll);
    } else {
        std::cout << "  ✗ 子进程未加载 HookEngine.dll" << std::endl;
        std::cout << "  ✗ Hook 继承失败！" << std::endl;
    }
    
    system("pause");
}

/**
 * @brief 子进程（第 2 层）逻辑
 */
static void RunAsChildProcessLevel2() {
    std::cout << "\n  [子进程 - Level 2] 我是进程 B，现在创建进程 C..." << std::endl;
    PrintProcessInfo(GetCurrentProcessId(), "B");
    
    // 检查自己是否加载了 HookEngine.dll
    HMODULE hDll = GetModuleHandleW(L"HookEngine.dll");
    if (hDll) {
        std::cout << "  ✓ 进程 B 已加载 HookEngine.dll" << std::endl;
        FreeLibrary(hDll);
    }
    
    // 创建进程 C
    wchar_t cmdLine[MAX_PATH];
    GetModuleFileNameW(nullptr, cmdLine, MAX_PATH);
    
    std::wstring childCmdLine = std::wstring(cmdLine) + L" --child-level3";
    
    std::cout << "\n  进程 B 创建进程 C..." << std::endl;
    CreateChildProcess(childCmdLine.c_str());
    
    system("pause");
}

/**
 * @brief 孙进程（第 3 层）逻辑
 */
static void RunAsChildProcessLevel3() {
    std::cout << "\n  [孙进程 - Level 3] 我是进程 C" << std::endl;
    PrintProcessInfo(GetCurrentProcessId(), "C");
    
    // 检查自己是否加载了 HookEngine.dll
    HMODULE hDll = GetModuleHandleW(L"HookEngine.dll");
    if (hDll) {
        std::cout << "  ✓ 进程 C 已加载 HookEngine.dll" << std::endl;
        std::cout << "  ✓ 2 层 Hook 继承成功！" << std::endl;
        FreeLibrary(hDll);
    } else {
        std::cout << "  ✗ 进程 C 未加载 HookEngine.dll" << std::endl;
        std::cout << "  ✗ 2 层 Hook 继承失败！" << std::endl;
    }
    
    system("pause");
}

// ============================================================================
// 主函数
// ============================================================================

int main(int argc, char* argv[]) {
    std::cout << "==============================================" << std::endl;
    std::cout << "  进程继承测试程序" << std::endl;
    std::cout << "  AI ThinApp Portable Launchpad Platform POC" << std::endl;
    std::cout << "==============================================" << std::endl;
    
    // 检查是否是子进程模式
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--child") == 0) {
            RunAsChildProcess();
            return 0;
        } else if (strcmp(argv[i], "--child-level2") == 0) {
            RunAsChildProcessLevel2();
            return 0;
        } else if (strcmp(argv[i], "--child-level3") == 0) {
            RunAsChildProcessLevel3();
            return 0;
        }
    }
    
    // 主进程模式：运行测试
    std::cout << "\n主进程 PID: " << GetCurrentProcessId() << std::endl;
    
    // 检查 DLL 是否存在
    if (GetFileAttributesW(L"HookEngine.dll") == INVALID_FILE_ATTRIBUTES) {
        std::cout << "\n错误: 未找到 HookEngine.dll" << std::endl;
        std::cout << "请将 HookEngine.dll 放到当前目录后重试。" << std::endl;
        system("pause");
        return 1;
    }
    
    std::cout << "\n找到 HookEngine.dll，开始测试...\n" << std::endl;
    
    // 运行测试
    TestChildProcessInheritance();
    
    std::cout << "\n----------------------------------------------\n" << std::endl;
    
    TestTwoLevelInheritance();
    
    std::cout << "\n==============================================" << std::endl;
    std::cout << "  测试完成" << std::endl;
    std::cout << "  请查看子进程的 DLL 加载情况" << std::endl;
    std::cout << "  使用 Process Explorer 验证 HookEngine.dll" << std::endl;
    std::cout << "==============================================" << std::endl;
    
    system("pause");
    return 0;
}

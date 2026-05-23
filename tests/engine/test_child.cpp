/**
 * @file test_child.cpp
 * @brief 子进程测试程序（V4 验证 - 子进程）
 * 
 * 功能：
 * 1. 写入文件 C:\Windows\Temp\child_test.txt
 * 2. 验证文件出现在 VFS 目录
 * 
 * @author AI ThinApp Team
 * @date 2026-05-23
 */

#include <Windows.h>
#include <stdio.h>
#include <string>

/**
 * @brief 主函数
 */
int wmain(int argc, wchar_t* argv[]) {
    wprintf(L"==============================================\n");
    wprintf(L"  子进程测试程序 (test_child.exe)\n");
    wprintf(L"  PID: %lu\n", GetCurrentProcessId());
    wprintf(L"==============================================\n\n");
    
    // 测试：写入文件到"系统目录"
    const wchar_t* testFile = L"C:\\Windows\\Temp\\child_test.txt";
    const wchar_t* testContent = L"Hello from child process!";
    
    wprintf(L"[子进程] 写入文件: %ls\n", testFile);
    
    // 打开/创建文件
    HANDLE hFile = CreateFileW(
        testFile,
        GENERIC_WRITE,
        0, // 独占访问
        NULL,
        CREATE_ALWAYS,
        FILE_ATTRIBUTE_NORMAL,
        NULL
    );
    
    if (hFile == INVALID_HANDLE_VALUE) {
        DWORD error = GetLastError();
        wprintf(L"[子进程] 错误: 无法创建文件 (错误码: %lu)\n", error);
        wprintf(L"\n按任意键退出...");
        getwchar();
        return 1;
    }
    
    // 写入内容
    DWORD bytesWritten = 0;
    BOOL result = WriteFile(
        hFile,
        testContent,
        (DWORD)(wcslen(testContent) * sizeof(wchar_t)),
        &bytesWritten,
        NULL
    );
    
    CloseHandle(hFile);
    
    if (!result) {
        DWORD error = GetLastError();
        wprintf(L"[子进程] 错误: 无法写入文件 (错误码: %lu)\n", error);
        wprintf(L"\n按任意键退出...");
        getwchar();
        return 1;
    }
    
    wprintf(L"[子进程] 文件写入成功: %ls\n", testFile);
    wprintf(L"[子进程] 写入内容: %ls\n", testContent);
    wprintf(L"\n[注意] 请在 Hook 环境下运行，并验证文件出现在 VFS 目录\n");
    
    // 等待用户按键
    wprintf(L"\n按任意键退出...");
    getwchar();
    
    return 0;
}

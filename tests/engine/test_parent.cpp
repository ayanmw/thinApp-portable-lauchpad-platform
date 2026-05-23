/**
 * @file test_parent.cpp
 * @brief 父进程测试程序（V4 验证 - 父进程）
 * 
 * 功能：
 * 1. 启动子进程 test_child.exe
 * 2. 子进程尝试写入 C:\Windows\Temp\child_test.txt
 * 3. 验证子进程也受 Hook 约束（文件出现在 VFS 目录）
 * 
 * @author AI ThinApp Team
 * @date 2026-05-23
 */

#include <Windows.h>
#include <stdio.h>
#include <tchar.h>

/**
 * @brief 主函数
 */
int wmain(int argc, wchar_t* argv[]) {
    wprintf(L"==============================================\n");
    wprintf(L"  父进程测试程序 (test_parent.exe)\n");
    wprintf(L"  PID: %lu\n", GetCurrentProcessId());
    wprintf(L"==============================================\n\n");
    
    // 构建子进程命令行
    wchar_t childPath[MAX_PATH];
    GetModuleFileNameW(NULL, childPath, MAX_PATH);
    
    // 去掉文件名，得到当前目录
    wchar_t* lastSlash = wcsrchr(childPath, L'\\');
    if (lastSlash) {
        *(lastSlash + 1) = L'\0';
    }
    
    // 拼接子进程路径
    wcscat_s(childPath, L"test_child.exe");
    
    wprintf(L"[父进程] 子进程路径: %ls\n\n", childPath);
    
    // 检查子进程是否存在
    if (!PathFileExistsW(childPath)) {
        wprintf(L"[父进程] 错误: 子进程不存在: %ls\n", childPath);
        wprintf(L"[父进程] 请确保 test_child.exe 在同一目录下\n");
        wprintf(L"\n按任意键退出...");
        getwchar();
        return 1;
    }
    
    // 启动子进程
    STARTUPINFOW si = {0};
    PROCESS_INFORMATION pi = {0};
    si.cb = sizeof(si);
    
    wprintf(L"[父进程] 正在启动子进程...\n");
    
    // 注意：在 Hook 环境下，CreateProcessW 会被 Hook
    // Hook 函数会注入 Hook DLL 到子进程
    BOOL result = CreateProcessW(
        childPath,       // 子进程路径
        NULL,            // 命令行（使用子进程路径作为命令行）
        NULL,            // 进程安全属性
        NULL,            // 线程安全属性
        FALSE,           // 不继承句柄
        0,               // 无特殊标志
        NULL,            // 环境变量
        NULL,            // 当前目录
        &si,             // 启动信息
        &pi              // 进程信息
    );
    
    if (!result) {
        DWORD error = GetLastError();
        wprintf(L"[父进程] 错误: 无法启动子进程 (错误码: %lu)\n", error);
        wprintf(L"\n按任意键退出...");
        getwchar();
        return 1;
    }
    
    wprintf(L"[父进程] 子进程已启动\n");
    wprintf(L"  PID: %lu\n", pi.dwProcessId);
    wprintf(L"  TID: %lu\n\n", pi.dwThreadId);
    
    wprintf(L"[父进程] 等待子进程退出...\n");
    
    // 等待子进程退出
    WaitForSingleObject(pi.hProcess, INFINITE);
    
    // 获取退出码
    DWORD exitCode = 0;
    GetExitCodeProcess(pi.hProcess, &exitCode);
    
    wprintf(L"\n[父进程] 子进程已退出 (退出码: %lu)\n", exitCode);
    
    // 关闭句柄
    CloseHandle(pi.hThread);
    CloseHandle(pi.hProcess);
    
    wprintf(L"\n[父进程] ==========\n");
    wprintf(L"[父进程] 验证步骤：\n");
    wprintf(L"[父进程] 1. 检查 VFS 目录中是否出现 child_test.txt\n");
    wprintf(L"[父进程]    预期路径: {AppDir}\\VFS\\C\\Windows\\Temp\\child_test.txt\n");
    wprintf(L"[父进程] 2. 检查真实系统目录 C:\\Windows\\Temp\\ 是否没有 child_test.txt\n");
    wprintf(L"[父进程] 3. 如果以上两点符合，说明子进程也受 Hook 约束（V4 通过）\n");
    wprintf(L"[父进程] ==========\n");
    
    // 等待用户按键
    wprintf(L"\n按任意键退出...");
    getwchar();
    
    return exitCode > 0 ? 1 : 0;
}

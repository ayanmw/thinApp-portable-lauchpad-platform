// tests/data/benchmark_apps/notepadpp_simulator.cpp
// Notepad++ 模拟器，用于 CI/CD 自动化测试

#include <iostream>
#include <fstream>
#include <string>
#include <windows.h>

// 模拟 Notepad++ 行为：创建文件、写入注册表、启动子进程

void createTestFile() {
    // 模拟 Notepad++ 创建文件
    std::string testFilePath = "C:\\Windows\\Temp\\test_notepadpp_simulator.txt";
    std::ofstream testFile(testFilePath);
    if (testFile.is_open()) {
        testFile << "这是一个测试文件，用于验证文件重定向功能。" << std::endl;
        testFile << "创建时间: " << __DATE__ << " " << __TIME__ << std::endl;
        testFile.close();
        std::cout << "[INFO] 测试文件创建成功: " << testFilePath << std::endl;
    } else {
        std::cerr << "[ERROR] 测试文件创建失败: " << testFilePath << std::endl;
    }
}

void writeTestRegistry() {
    // 模拟 Notepad++ 写入注册表
    HKEY hKey;
    LONG result = RegCreateKeyEx(HKEY_CURRENT_USER, 
                                 "Software\\NotepadppSimulator", 
                                 0, 
                                 NULL, 
                                 REG_OPTION_NON_VOLATILE, 
                                 KEY_ALL_ACCESS, 
                                 NULL, 
                                 &hKey, 
                                 NULL);
    if (result == ERROR_SUCCESS) {
        std::string testValue = "Test Data from Notepad++ Simulator";
        RegSetValueEx(hKey, 
                      "TestValue", 
                      0, 
                      REG_SZ, 
                      (const BYTE*)testValue.c_str(), 
                      testValue.size() + 1);
        RegCloseKey(hKey);
        std::cout << "[INFO] 注册表键值写入成功: HKEY_CURRENT_USER\\Software\\NotepadppSimulator\\TestValue" << std::endl;
    } else {
        std::cerr << "[ERROR] 注册表键值写入失败: " << result << std::endl;
    }
}

void spawnChildProcess() {
    // 模拟 Notepad++ 启动子进程
    STARTUPINFO si;
    PROCESS_INFORMATION pi;
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    ZeroMemory(&pi, sizeof(pi));
    
    // 启动一个子进程（例如：notepad.exe）
    TCHAR cmdLine[] = _T("notepad.exe C:\\Windows\\Temp\\test_notepadpp_simulator.txt");
    BOOL result = CreateProcess(NULL, 
                                cmdLine, 
                                NULL, 
                                NULL, 
                                FALSE, 
                                0, 
                                NULL, 
                                NULL, 
                                &si, 
                                &pi);
    if (result) {
        std::cout << "[INFO] 子进程启动成功: notepad.exe" << std::endl;
        // 等待子进程启动
        Sleep(1000);
        CloseHandle(pi.hThread);
        CloseHandle(pi.hProcess);
    } else {
        std::cerr << "[ERROR] 子进程启动失败: " << GetLastError() << std::endl;
    }
}

int main(int argc, char* argv[]) {
    std::cout << "========================================" << std::endl;
    std::cout << "Notepad++ 模拟器 - 用于 CI/CD 自动化测试" << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << "[INFO] 启动时间: " << __DATE__ << " " << __TIME__ << std::endl;
    std::cout << std::endl;
    
    // 模拟 Notepad++ 行为
    std::cout << "[STEP 1] 创建测试文件..." << std::endl;
    createTestFile();
    std::cout << std::endl;
    
    std::cout << "[STEP 2] 写入测试注册表..." << std::endl;
    writeTestRegistry();
    std::cout << std::endl;
    
    std::cout << "[STEP 3] 启动子进程..." << std::endl;
    spawnChildProcess();
    std::cout << std::endl;
    
    std::cout << "========================================" << std::endl;
    std::cout << "[INFO] Notepad++ 模拟器运行完成" << std::endl;
    std::cout << "[INFO] 请检查以下内容以验证沙箱功能：" << std::endl;
    std::cout << "  1. 文件是否重定向到 {AppDir}\\VFS\\C\\Windows\\Temp\\test_notepadpp_simulator.txt" << std::endl;
    std::cout << "  2. 注册表是否重定向到 {AppDir}\\VFS\\registry.hive" << std::endl;
    std::cout << "  3. 子进程是否继承了 Hook" << std::endl;
    std::cout << "========================================" << std::endl;
    
    // 保持窗口打开（方便查看输出）
    std::cout << std::endl;
    std::cout << "按 Enter 键退出..." << std::endl;
    std::cin.get();
    
    return 0;
}

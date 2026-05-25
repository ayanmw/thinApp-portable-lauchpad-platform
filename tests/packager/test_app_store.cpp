/**
 * @file test_app_store.cpp
 * @brief 应用商店集成测试程序
 * @author Windows C++ 底层开发工程师
 * @date 2026-05-23
 * 
 * 本程序测试 AppStore 类的所有功能。
 * 
 * 编译命令：
 * cl /EHsc /I..\src\packager test_app_store.cpp ..\src\packager\app_store.cpp
 * 
 * 运行命令：
 * .\test_app_store.exe
 */

#include "app_store.h"
#include <iostream>
#include <cassert>
#include <filesystem>

// 使用 std::filesystem
namespace fs = std::filesystem;

// 测试计数器
int g_test_count = 0;
int g_pass_count = 0;
int g_fail_count = 0;

// 断言宏（简化版）
#define MY_ASSERT(condition, test_name) \
    g_test_count++; \
    if (condition) { \
        g_pass_count++; \
        std::cout << "[PASS] " << test_name << std::endl; \
    } else { \
        g_fail_count++; \
        std::cout << "[FAIL] " << test_name << std::endl; \
    }

/**
 * @brief 测试用例 1：上传应用
 * 
 * 测试功能：上传 .vapp 包到应用商店
 * 测试步骤：
 * 1. 创建测试 .vapp 包文件
 * 2. 调用 UploadApp() 上传
 * 3. 检查是否上传成功
 * 
 * 预期结果：
 * - 上传成功（返回 true）
 */
bool TestUploadApp() {
    std::cout << "\n[TEST] 测试用例 1：上传应用" << std::endl;
    std::cout << "========================================" << std::endl;
    
    // 创建测试 .vapp 包文件
    std::string test_vapp = "test_app.vapp";
    std::ofstream out(test_vapp);
    if (!out.is_open()) {
        std::cerr << "[ERROR] 无法创建测试文件: " << test_vapp << std::endl;
        return false;
    }
    out << "test vapp package content" << std::endl;
    out.close();
    
    // 创建 AppStore 对象
    packager::AppStore store;
    store.SetApiBaseUrl("https://api.ai-thinapp.com/v1");
    
    // 上传应用
    bool result = store.UploadApp(test_vapp);
    
    // 检查上传是否成功
    MY_ASSERT(result == true, "上传应用");
    
    // 清理
    fs::remove(test_vapp);
    
    return result;
}

/**
 * @brief 测试用例 2：下载应用
 * 
 * 测试功能：从应用商店下载 .vapp 包
 * 测试步骤：
 * 1. 调用 DownloadApp() 下载
 * 2. 检查是否下载成功
 * 
 * 预期结果：
 * - 下载成功（返回 true）
 */
bool TestDownloadApp() {
    std::cout << "\n[TEST] 测试用例 2：下载应用" << std::endl;
    std::cout << "========================================" << std::endl;
    
    // 创建 AppStore 对象
    packager::AppStore store;
    store.SetApiBaseUrl("https://api.ai-thinapp.com/v1");
    
    // 下载应用
    bool result = store.DownloadApp("Notepad++", "8.6.4");
    
    // 检查下载是否成功
    MY_ASSERT(result == true, "下载应用");
    
    return result;
}

/**
 * @brief 测试用例 3：搜索应用
 * 
 * 测试功能：搜索应用商店中的应用
 * 测试步骤：
 * 1. 调用 SearchApps() 搜索
 * 2. 检查搜索结果数量
 * 
 * 预期结果：
 * - 搜索成功（返回 true）
 * - 结果数量 ≥ 1
 */
bool TestSearchApps() {
    std::cout << "\n[TEST] 测试用例 3：搜索应用" << std::endl;
    std::cout << "========================================" << std::endl;
    
    // 创建 AppStore 对象
    packager::AppStore store;
    store.SetApiBaseUrl("https://api.ai-thinapp.com/v1");
    
    // 搜索应用
    std::vector<std::string> results;
    bool result = store.SearchApps("Notepad++", results);
    
    // 检查搜索是否成功
    MY_ASSERT(result == true, "搜索应用");
    
    // 检查结果数量
    MY_ASSERT(results.size() >= 1, "搜索结果数量 ≥ 1");
    
    return result && (results.size() >= 1);
}

/**
 * @brief 测试用例 4：更新本地数据库
 * 
 * 测试功能：更新本地 SQLite 数据库
 * 测试步骤：
 * 1. 调用 UpdateLocalDB() 更新
 * 2. 检查是否更新成功
 * 
 * 预期结果：
 * - 更新成功（返回 true）
 */
bool TestUpdateLocalDB() {
    std::cout << "\n[TEST] 测试用例 4：更新本地数据库" << std::endl;
    std::cout << "========================================" << std::endl;
    
    // 创建 AppStore 对象
    packager::AppStore store;
    store.SetLocalDbPath("test_local_apps.db");
    
    // 更新本地数据库
    bool result = store.UpdateLocalDB();
    
    // 检查更新是否成功
    MY_ASSERT(result == true, "更新本地数据库");
    
    // 清理
    if (fs::exists("test_local_apps.db")) {
        fs::remove("test_local_apps.db");
    }
    
    return result;
}

/**
 * @brief 主函数
 * @return 0: 所有测试通过; 1: 有测试失败
 */
int main() {
    std::cout << "========================================" << std::endl;
    std::cout << "  应用商店集成测试程序" << std::endl;
    std::cout << "========================================" << std::endl;
    
    // 运行所有测试用例
    TestUploadApp();
    TestDownloadApp();
    TestSearchApps();
    TestUpdateLocalDB();
    
    // 输出测试统计
    std::cout << "\n========================================" << std::endl;
    std::cout << "  测试统计" << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << "总测试数: " << g_test_count << std::endl;
    std::cout << "通过: " << g_pass_count << std::endl;
    std::cout << "失败: " << g_fail_count << std::endl;
    std::cout << "通过率: " << (g_test_count > 0 ? (g_pass_count * 100 / g_test_count) : 0) << "%" << std::endl;
    std::cout << "========================================" << std::endl;
    
    // 返回结果
    if (g_fail_count == 0) {
        std::cout << "[INFO] 所有测试通过！" << std::endl;
        return 0;
    } else {
        std::cout << "[ERROR] 有 " << g_fail_count << " 个测试失败！" << std::endl;
        return 1;
    }
}

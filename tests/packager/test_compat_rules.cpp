/**
 * @file test_compat_rules.cpp
 * @brief 兼容性规则库测试程序
 * @author Windows C++ 底层开发工程师
 * @date 2026-05-23
 * 
 * 本程序测试 CompatRules 类的所有功能。
 * 
 * 编译命令：
 * cl /EHsc /I..\\src\\packager test_compat_rules.cpp ..\\src\\packager\\compat_rules.cpp
 * 
 * 运行命令：
 * .\\test_compat_rules.exe
 */

#include "compat_rules.h"
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
 * @brief 测试用例 1：加载兼容性规则
 * 
 * 测试功能：从 YAML 文件加载兼容性规则
 * 测试步骤：
 * 1. 创建临时 YAML 规则文件
 * 2. 调用 LoadRules() 加载规则
 * 3. 检查规则数量是否正确
 * 
 * 预期结果：
 * - 成功加载 5 条规则
 */
bool TestLoadRules() {
    std::cout << "\n[TEST] 测试用例 1：加载兼容性规则" << std::endl;
    std::cout << "========================================" << std::endl;
    
    // 创建临时 YAML 规则文件
    std::string test_rules_file = "test_rules.yaml";
    std::ofstream out(test_rules_file);
    if (!out.is_open()) {
        std::cerr << "[ERROR] 无法创建测试规则文件: " << test_rules_file << std::endl;
        return false;
    }
    
    out << "file_rules:\n";
    out << "  - type: \"exclude\"\n";
    out << "    path: \"C:\\\\Windows\\\\System32\\\\*\"\n";
    out << "    description: \"排除系统文件\"\n";
    out << "  - type: \"include\"\n";
    out << "    path: \"C:\\\\Program Files\\\\*\\\\app.exe\"\n";
    out << "    description: \"包含应用主程序\"\n";
    out << "  - type: \"exclude\"\n";
    out << "    path: \"%APPDATA%\\\\*\\\\updates\\\\*\"\n";
    out << "    description: \"排除应用更新文件\"\n";
    out << "  - type: \"include\"\n";
    out << "    path: \"^C:\\\\Users\\\\[^\\\\]+\\\\AppData\\\\.*$\"\n";
    out << "    description: \"包含用户数据文件（正则）\"\n";
    out << "  - type: \"exclude\"\n";
    out << "    path: \"C:\\\\Temp\\\\*\"\n";
    out << "    description: \"排除临时文件\"\n";
    out.close();
    
    // 加载规则
    packager::CompatRules rules;
    bool result = rules.LoadRules(test_rules_file);
    
    // 检查加载是否成功
    MY_ASSERT(result == true, "加载规则文件");
    
    // 检查规则数量
    MY_ASSERT(rules.GetRuleCount() == 5, "规则数量 = 5");
    
    // 清理
    fs::remove(test_rules_file);
    
    return result && (rules.GetRuleCount() == 5);
}

/**
 * @brief 测试用例 2：前缀匹配
 * 
 * 测试功能：前缀匹配规则
 * 测试步骤：
 * 1. 创建前缀匹配规则（如 "C:\Windows\System32\*"）
 * 2. 调用 MatchRule() 匹配文件路径
 * 3. 检查是否匹配成功
 * 
 * 预期结果：
 * - "C:\Windows\System32\kernel32.dll" 匹配成功
 * - "C:\Windows\notepad.exe" 匹配失败
 */
bool TestPrefixMatch() {
    std::cout << "\n[TEST] 测试用例 2：前缀匹配" << std::endl;
    std::cout << "========================================" << std::endl;
    
    // 创建测试规则文件
    std::string test_rules_file = "test_prefix_rules.yaml";
    std::ofstream out(test_rules_file);
    out << "file_rules:\n";
    out << "  - type: \"exclude\"\n";
    out << "    path: \"C:\\\\Windows\\\\System32\\\\\"\n";
    out << "    description: \"排除系统文件\"\n";
    out.close();
    
    // 加载规则
    packager::CompatRules rules;
    rules.LoadRules(test_rules_file);
    
    // 测试前缀匹配
    bool result1 = rules.MatchRule("C:\\Windows\\System32\\kernel32.dll");
    MY_ASSERT(result1 == true, "前缀匹配: C:\\Windows\\System32\\kernel32.dll");
    
    bool result2 = rules.MatchRule("C:\\Windows\\notepad.exe");
    MY_ASSERT(result2 == false, "前缀不匹配: C:\\Windows\\notepad.exe");
    
    // 清理
    fs::remove(test_rules_file);
    
    return result1 && !result2;
}

/**
 * @brief 测试用例 3：通配符匹配
 * 
 * 测试功能：通配符匹配规则
 * 测试步骤：
 * 1. 创建通配符匹配规则（如 "C:\Program Files\*\app.exe"）
 * 2. 调用 MatchRule() 匹配文件路径
 * 3. 检查是否匹配成功
 * 
 * 预期结果：
 * - "C:\Program Files\App\app.exe" 匹配成功
 * - "C:\Program Files\App\readme.txt" 匹配失败
 */
bool TestWildcardMatch() {
    std::cout << "\n[TEST] 测试用例 3：通配符匹配" << std::endl;
    std::cout << "========================================" << std::endl;
    
    // 创建测试规则文件
    std::string test_rules_file = "test_wildcard_rules.yaml";
    std::ofstream out(test_rules_file);
    out << "file_rules:\n";
    out << "  - type: \"include\"\n";
    out << "    path: \"C:\\\\Program Files\\\\*\\\\app.exe\"\n";
    out << "    description: \"包含应用主程序\"\n";
    out.close();
    
    // 加载规则
    packager::CompatRules rules;
    rules.LoadRules(test_rules_file);
    
    // 测试通配符匹配
    bool result1 = rules.MatchRule("C:\\Program Files\\App\\app.exe");
    MY_ASSERT(result1 == true, "通配符匹配: C:\\Program Files\\App\\app.exe");
    
    bool result2 = rules.MatchRule("C:\\Program Files\\App\\readme.txt");
    MY_ASSERT(result2 == false, "通配符不匹配: C:\\Program Files\\App\\readme.txt");
    
    // 清理
    fs::remove(test_rules_file);
    
    return result1 && !result2;
}

/**
 * @brief 测试用例 4：正则匹配
 * 
 * 测试功能：正则表达式匹配规则
 * 测试步骤：
 * 1. 创建正则匹配规则（如 "^C:\\Users\\[^\\]+\AppData\\.*$"）
 * 2. 调用 MatchRule() 匹配文件路径
 * 3. 检查是否匹配成功
 * 
 * 预期结果：
 * - "C:\Users\John\AppData\Roaming\App\config.json" 匹配成功
 * - "C:\Users\John\Documents\file.txt" 匹配失败
 */
bool TestRegexMatch() {
    std::cout << "\n[TEST] 测试用例 4：正则匹配" << std::endl;
    std::cout << "========================================" << std::endl;
    
    // 创建测试规则文件
    std::string test_rules_file = "test_regex_rules.yaml";
    std::ofstream out(test_rules_file);
    out << "file_rules:\n";
    out << "  - type: \"include\"\n";
    out << "    path: \"^C:\\\\\\\\Users\\\\\\\\[^\\\\]+\\\\\\\\AppData\\\\\\\\.*$\"\n";
    out << "    description: \"包含用户数据文件（正则）\"\n";
    out.close();
    
    // 加载规则
    packager::CompatRules rules;
    rules.LoadRules(test_rules_file);
    
    // 测试正则匹配
    bool result1 = rules.MatchRule("C:\\Users\\John\\AppData\\Roaming\\App\\config.json");
    MY_ASSERT(result1 == true, "正则匹配: C:\\Users\\John\\AppData\\Roaming\\App\\config.json");
    
    bool result2 = rules.MatchRule("C:\\Users\\John\\Documents\\file.txt");
    MY_ASSERT(result2 == false, "正则不匹配: C:\\Users\\John\\Documents\\file.txt");
    
    // 清理
    fs::remove(test_rules_file);
    
    return result1 && !result2;
}

/**
 * @brief 测试用例 5：无匹配规则
 * 
 * 测试功能：无匹配规则时的处理
 * 测试步骤：
 * 1. 创建规则（仅匹配特定路径）
 * 2. 调用 MatchRule() 匹配不匹配的路径
 * 3. 检查是否返回 false
 * 
 * 预期结果：
 * - "C:\MyApp\app.exe" 不匹配任何规则，返回 false
 * - GetMatchedRules() 返回空列表
 */
bool TestNoMatch() {
    std::cout << "\n[TEST] 测试用例 5：无匹配规则" << std::endl;
    std::cout << "========================================" << std::endl;
    
    // 创建测试规则文件
    std::string test_rules_file = "test_nomatch_rules.yaml";
    std::ofstream out(test_rules_file);
    out << "file_rules:\n";
    out << "  - type: \"include\"\n";
    out << "    path: \"C:\\\\Program Files\\\\*\"\n";
    out << "    description: \"包含应用文件\"\n";
    out.close();
    
    // 加载规则
    packager::CompatRules rules;
    rules.LoadRules(test_rules_file);
    
    // 测试无匹配
    bool result1 = rules.MatchRule("C:\\MyApp\\app.exe");
    MY_ASSERT(result1 == false, "无匹配规则: C:\\MyApp\\app.exe");
    
    std::vector<packager::CompatRule> matched_rules = rules.GetMatchedRules("C:\\MyApp\\app.exe");
    MY_ASSERT(matched_rules.empty() == true, "匹配规则列表为空");
    
    // 清理
    fs::remove(test_rules_file);
    
    return !result1 && matched_rules.empty();
}

/**
 * @brief 主函数
 * @return 0: 所有测试通过; 1: 有测试失败
 */
int main() {
    std::cout << "========================================" << std::endl;
    std::cout << "  兼容性规则库测试程序" << std::endl;
    std::cout << "========================================" << std::endl;
    
    // 运行所有测试用例
    TestLoadRules();
    TestPrefixMatch();
    TestWildcardMatch();
    TestRegexMatch();
    TestNoMatch();
    
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

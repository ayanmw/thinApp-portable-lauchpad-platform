/**
 * @file test_capture_diagnosis.cpp
 * @brief 捕获失败诊断测试程序
 * @author Windows C++ 底层开发工程师
 * @date 2026-05-23
 * 
 * 本程序测试 AppCapture 类的捕获失败诊断功能。
 * 
 * 编译命令：
 * cl /EHsc /I..\src\packager test_capture_diagnosis.cpp ..\src\packager\app_capture.cpp
 * 
 * 运行命令：
 * .\test_capture_diagnosis.exe
 */

#include "app_capture.h"
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
 * @brief 测试用例 1：诊断注册表快照失败
 * 
 * 测试功能：模拟注册表快照失败，验证诊断报告正确
 * 测试步骤：
 * 1. 创建测试日志文件（包含注册表枚举失败错误）
 * 2. 调用 DiagnoseCaptureFailure() 诊断
 * 3. 检查诊断报告是否包含正确信息
 * 
 * 预期结果：
 * - 诊断报告包含 "E-REG-001" 或 "E-REG-002"
 * - 诊断报告包含 "权限不足" 或 "无法导出 Hive"
 */
bool TestDiagnoseRegistrySnapshotFailure() {
    std::cout << "\n[TEST] 测试用例 1：诊断注册表快照失败" << std::endl;
    std::cout << "========================================" << std::endl;
    
    // 创建测试日志文件
    std::string test_log = "test_log_registry.jsonl";
    std::ofstream out(test_log);
    if (!out.is_open()) {
        std::cerr << "[ERROR] 无法创建测试日志: " << test_log << std::endl;
        return false;
    }
    
    // 写入日志（包含注册表枚举失败错误）
    out << "{ \"timestamp\": \"2026-05-23T10:00:00.000Z\", \"level\": \"INFO\", \"module\": \"AppCapture\", \"message\": \"开始捕获\" }" << std::endl;
    out << "{ \"timestamp\": \"2026-05-23T10:00:01.000Z\", \"level\": \"ERROR\", \"module\": \"Registry\", \"message\": \"注册表枚举失败: AccessDenied\", \"key\": \"HKLM\\\\Software\\\\Microsoft\" }" << std::endl;
    out.close();
    
    // 创建 AppCapture 对象
    packager::AppCapture capture;
    
    // 诊断捕获失败
    std::string diagnosis_report;
    bool result = capture.DiagnoseCaptureFailure(test_log, diagnosis_report);
    
    // 检查诊断是否成功
    MY_ASSERT(result == true, "诊断注册表快照失败");
    
    // 检查诊断报告是否包含正确信息
    bool has_error_id = (diagnosis_report.find("E-REG") != std::string::npos);
    MY_ASSERT(has_error_id == true, "诊断报告包含错误 ID");
    
    // 清理
    fs::remove(test_log);
    
    return result && has_error_id;
}

/**
 * @brief 测试用例 2：诊断 .vapp 包导出失败
 * 
 * 测试功能：模拟 ZIP 压缩失败，验证诊断报告正确
 * 测试步骤：
 * 1. 创建测试日志文件（包含 ZIP 压缩失败错误）
 * 2. 调用 DiagnoseCaptureFailure() 诊断
 * 3. 检查诊断报告是否包含正确信息
 * 
 * 预期结果：
 * - 诊断报告包含 "E-EXPORT-001"
 * - 诊断报告包含 "磁盘空间不足"
 */
bool TestDiagnoseVAppExportFailure() {
    std::cout << "\n[TEST] 测试用例 2：诊断 .vapp 包导出失败" << std::endl;
    std::cout << "========================================" << std::endl;
    
    // 创建测试日志文件
    std::string test_log = "test_log_export.jsonl";
    std::ofstream out(test_log);
    if (!out.is_open()) {
        std::cerr << "[ERROR] 无法创建测试日志: " << test_log << std::endl;
        return false;
    }
    
    // 写入日志（包含 ZIP 压缩失败错误）
    out << "{ \"timestamp\": \"2026-05-23T10:00:00.000Z\", \"level\": \"INFO\", \"module\": \"AppCapture\", \"message\": \"开始导出\" }" << std::endl;
    out << "{ \"timestamp\": \"2026-05-23T10:00:01.000Z\", \"level\": \"ERROR\", \"module\": \"VAppExporter\", \"message\": \"ZIP 压缩失败: 磁盘空间不足\" }" << std::endl;
    out.close();
    
    // 创建 AppCapture 对象
    packager::AppCapture capture;
    
    // 诊断捕获失败
    std::string diagnosis_report;
    bool result = capture.DiagnoseCaptureFailure(test_log, diagnosis_report);
    
    // 检查诊断是否成功
    MY_ASSERT(result == true, "诊断 .vapp 包导出失败");
    
    // 检查诊断报告是否包含正确信息
    bool has_error_id = (diagnosis_report.find("E-EXPORT") != std::string::npos);
    MY_ASSERT(has_error_id == true, "诊断报告包含错误 ID");
    
    // 清理
    fs::remove(test_log);
    
    return result && has_error_id;
}

/**
 * @brief 测试用例 3：诊断 Hook DLL 注入失败
 * 
 * 测试功能：模拟 CreateRemoteThreadEx 失败，验证诊断报告正确
 * 测试步骤：
 * 1. 创建测试日志文件（包含 Hook DLL 注入失败错误）
 * 2. 调用 DiagnoseCaptureFailure() 诊断
 * 3. 检查诊断报告是否包含正确信息
 * 
 * 预期结果：
 * - 诊断报告包含 "E-HOOK-001" 或 "E-HOOK-002"
 * - 诊断报告包含 "进程保护" 或 "权限不足"
 */
bool TestDiagnoseHookDllInjectionFailure() {
    std::cout << "\n[TEST] 测试用例 3：诊断 Hook DLL 注入失败" << std::endl;
    std::cout << "========================================" << std::endl;
    
    // 创建测试日志文件
    std::string test_log = "test_log_hook.jsonl";
    std::ofstream out(test_log);
    if (!out.is_open()) {
        std::cerr << "[ERROR] 无法创建测试日志: " << test_log << std::endl;
        return false;
    }
    
    // 写入日志（包含 Hook DLL 注入失败错误）
    out << "{ \"timestamp\": \"2026-05-23T10:00:00.000Z\", \"level\": \"INFO\", \"module\": \"AppCapture\", \"message\": \"开始注入 Hook\" }" << std::endl;
    out << "{ \"timestamp\": \"2026-05-23T10:00:01.000Z\", \"level\": \"ERROR\", \"module\": \"Hook\", \"message\": \"Hook DLL 注入失败: app.exe (进程保护）\" }" << std::endl;
    out.close();
    
    // 创建 AppCapture 对象
    packager::AppCapture capture;
    
    // 诊断捕获失败
    std::string diagnosis_report;
    bool result = capture.DiagnoseCaptureFailure(test_log, diagnosis_report);
    
    // 检查诊断是否成功
    MY_ASSERT(result == true, "诊断 Hook DLL 注入失败");
    
    // 检查诊断报告是否包含正确信息
    bool has_error_id = (diagnosis_report.find("E-HOOK") != std::string::npos);
    MY_ASSERT(has_error_id == true, "诊断报告包含错误 ID");
    
    // 清理
    fs::remove(test_log);
    
    return result && has_error_id;
}

/**
 * @brief 测试用例 4：建议修复方案
 * 
 * 测试功能：根据诊断报告，验证修复建议正确
 * 测试步骤：
 * 1. 创建诊断报告（JSON 格式）
 * 2. 调用 SuggestFix() 建议修复方案
 * 3. 检查修复建议是否包含正确信息
 * 
 * 预期结果：
 * - 修复建议包含 "auto_fixable"
 * - 修复建议包含 "fix_action" 或 "manual_steps"
 */
bool TestSuggestFix() {
    std::cout << "\n[TEST] 测试用例 4：建议修复方案" << std::endl;
    std::cout << "========================================" << std::endl;
    
    // 创建诊断报告（JSON 格式）
    std::string diagnosis_report = "{";
    diagnosis_report += "  \"error_id\": \"E-FS-001\",";
    diagnosis_report += "  \"cause\": \"权限不足\",";
    diagnosis_report += "  \"suggestion\": \"以管理员身份运行\",";
    diagnosis_report += "  \"log\": { \"timestamp\": \"2026-05-23T10:00:00.000Z\", \"level\": \"ERROR\", \"module\": \"FileSystem\", \"message\": \"文件访问被拒绝\" }";
    diagnosis_report += "}";
    
    // 创建 AppCapture 对象
    packager::AppCapture capture;
    
    // 建议修复方案
    std::string fix_suggestion;
    bool result = capture.SuggestFix(diagnosis_report, fix_suggestion);
    
    // 检查建议是否成功
    MY_ASSERT(result == true, "建议修复方案");
    
    // 检查修复建议是否包含正确信息
    bool has_auto_fixable = (fix_suggestion.find("auto_fixable") != std::string::npos);
    MY_ASSERT(has_auto_fixable == true, "修复建议包含 auto_fixable");
    
    bool has_fix_action_or_manual_steps = 
        (fix_suggestion.find("fix_action") != std::string::npos) ||
        (fix_suggestion.find("manual_steps") != std::string::npos);
    MY_ASSERT(has_fix_action_or_manual_steps == true, "修复建议包含 fix_action 或 manual_steps");
    
    return result && has_auto_fixable && has_fix_action_or_manual_steps;
}

/**
 * @brief 主函数
 * @return 0: 所有测试通过; 1: 有测试失败
 */
int main() {
    std::cout << "========================================" << std::endl;
    std::cout << "  捕获失败诊断测试程序" << std::endl;
    std::cout << "========================================" << std::endl;
    
    // 运行所有测试用例
    TestDiagnoseRegistrySnapshotFailure();
    TestDiagnoseVAppExportFailure();
    TestDiagnoseHookDllInjectionFailure();
    TestSuggestFix();
    
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

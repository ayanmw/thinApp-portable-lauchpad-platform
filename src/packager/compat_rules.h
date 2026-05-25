/**
 * @file compat_rules.h
 * @brief 兼容性规则库头文件
 * @author Windows C++ 底层开发工程师
 * @date 2026-05-23
 * 
 * 本文件定义 CompatRule 结构体和 CompatRules 类，负责兼容性规则的
 * 加载、匹配和管理功能。
 * 
 * 兼容性规则用于记录每款应用的特殊处理方式（如"应用 X 需要排除 Y 文件"），
 * 提升捕获成功率。
 * 
 * 规则格式：YAML
 * 匹配模式：前缀匹配、通配符匹配、正则匹配
 */

#ifndef COMPAT_RULES_H
#define COMPAT_RULES_H

#include <string>
#include <vector>
#include <memory>

namespace packager {

// ============================================================================
// 结构体和枚举定义
// ============================================================================

/**
 * @brief 规则类型枚举
 * 
 * 表示兼容性规则的类型。
 */
enum class RuleType {
    Include,    ///< 包含（优先）
    Exclude      ///< 排除
};

/**
 * @brief 匹配模式枚举
 * 
 * 表示规则路径的匹配模式。
 */
enum class MatchMode {
    Prefix,      ///< 前缀匹配（如 "C:\Windows\System32\*"）
    Wildcard,    ///< 通配符匹配（如 "C:\Program Files\*\app.exe"）
    Regex        ///< 正则表达式匹配（如 "^C:\\Users\\[^\\]+\AppData\\.*$"）
};

/**
 * @brief 兼容性规则结构体
 * 
 * 表示一条兼容性规则（文件规则、注册表规则或进程规则）。
 */
struct CompatRule {
    RuleType rule_type_;      ///< 规则类型（Include / Exclude）
    MatchMode match_mode_;    ///< 匹配模式（Prefix / Wildcard / Regex）
    std::string pattern_;     ///< 匹配模式（文件路径、注册表路径或进程名）
    std::string description_;  ///< 规则描述
    
    /**
     * @brief 构造函数
     * @param rule_type 规则类型
     * @param match_mode 匹配模式
     * @param pattern 匹配模式
     * @param description 规则描述
     */
    CompatRule(RuleType rule_type = RuleType::Include,
                MatchMode match_mode = MatchMode::Prefix,
                const std::string& pattern = "",
                const std::string& description = "")
        : rule_type_(rule_type)
        , match_mode_(match_mode)
        , pattern_(pattern)
        , description_(description) {
    }
};

// ============================================================================
// CompatRules 类定义
// ============================================================================

/**
 * @brief 兼容性规则库类
 * 
 * 负责兼容性规则的加载、匹配和管理。
 * 
 * 使用示例：
 * @code
 * packager::CompatRules rules;
 * 
 * // 加载规则文件
 * if (!rules.LoadRules("rules.yaml")) {
 *     printf("加载规则失败\n");
 *     return;
 * }
 * 
 * // 匹配文件路径
 * std::string file_path = "C:\\Windows\\System32\\kernel32.dll";
 * if (rules.MatchRule(file_path)) {
 *     printf("文件匹配规则\n");
 * }
 * 
 * // 获取所有匹配的规则
 * std::vector<CompatRule> matched_rules = rules.GetMatchedRules(file_path);
 * printf("匹配到 %zu 条规则\n", matched_rules.size());
 * @endcode
 */
class CompatRules {
public:
    /**
     * @brief 构造函数
     */
    CompatRules();

    /**
     * @brief 析构函数
     */
    ~CompatRules();

    // ========================================================================
    // 公共接口
    // ========================================================================

    /**
     * @brief 从 YAML 文件加载兼容性规则
     * @param rules_file YAML 规则文件路径
     * @return 成功返回 true，失败返回 false
     * 
     * 功能说明：
     * 1. 使用 yaml-cpp 库解析 YAML 文件
     * 2. 提取文件规则、注册表规则、进程规则
     * 3. 将所有规则保存到 rules\_ 成员变量中
     * 
     * YAML 文件格式示例：
     * @code
     * # 文件规则
     * file_rules:
     *   - type: "exclude"
     *     path: "%APPDATA%\\Mozilla\\Firefox\\updates\\*"
     *     description: "排除 Firefox 自动更新文件"
     * 
     * # 注册表规则
     * registry_rules:
     *   - type: "exclude"
     *     path: "HKCU\\Software\\Mozilla\\Firefox\\Updates"
     *     description: "排除 Firefox 更新注册表项"
     * 
     * # 进程规则
     * process_rules:
     *   - type: "inject_hook"
     *     process_name: "firefox.exe"
     *     description: "对 firefox.exe 注入 Hook DLL"
     * @endcode
     */
    bool LoadRules(const std::string& rules_file);

    /**
     * @brief 匹配文件路径到兼容性规则
     * @param file_path 文件路径（如 "C:\\Windows\\System32\\kernel32.dll"）
     * @return 匹配返回 true，不匹配返回 false
     * 
     * 功能说明：
     * 1. 遍历所有规则（rules\_）
     * 2. 根据规则的 match_mode\_ 选择匹配算法
     * 3. 若任意规则匹配，返回 true
     * 
     * 匹配算法：
     * - 前缀匹配：file_path.starts_with(rule.pattern\_)
     * - 通配符匹配：将通配符模式转换为正则表达式，使用 std::regex 匹配
     * - 正则匹配：直接使用 std::regex 匹配
     */
    bool MatchRule(const std::string& file_path);

    /**
     * @brief 返回所有匹配的规则
     * @param file_path 文件路径（如 "C:\\Windows\\System32\\kernel32.dll"）
     * @return 匹配的规则列表（可能为空）
     * 
     * 功能说明：
     * 1. 遍历所有规则（rules\_）
     * 2. 根据规则的 match_mode\_ 选择匹配算法
     * 3. 返回所有匹配的规则
     */
    std::vector<CompatRule> GetMatchedRules(const std::string& file_path);

    /**
     * @brief 获取所有规则
     * @return 所有规则的常量引用
     */
    const std::vector<CompatRule>& GetAllRules() const { return rules_; }

    /**
     * @brief 清空所有规则
     */
    void ClearRules() { rules_.clear(); }

    /**
     * @brief 获取规则数量
     * @return 规则数量
     */
    size_t GetRuleCount() const { return rules_.size(); }

private:
    // ========================================================================
    // 私有成员变量
    // ========================================================================

    /**
     * @brief 规则列表
     * 
     * 存储所有加载的兼容性规则。
     */
    std::vector<CompatRule> rules_;

    // ========================================================================
    // 内部辅助方法
    // ========================================================================

    /**
     * @brief 展开环境变量
     * @param path 包含环境变量的路径（如 "%APPDATA%\\Mozilla"）
     * @return 展开后的路径（如 "C:\\Users\\user\\AppData\\Roaming\\Mozilla"）
     * 
     * 支持的环境变量：
     * - %APPDATA%
     * - %LOCALAPPDATA%
     * - %PROGRAMDATA%
     * - %TEMP%
     * - %PROGRAMFILES%
     * - %PROGRAMFILES(X86)%
     */
    std::string ExpandEnvVars(const std::string& path) const;

    /**
     * @brief 前缀匹配
     * @param rule_pattern 规则模式（如 "C:\\Windows\\System32\\"）
     * @param file_path 文件路径（如 "C:\\Windows\\System32\\kernel32.dll"）
     * @return 匹配返回 true，不匹配返回 false
     * 
     * 算法：
     * 1. 展开环境变量（rule_pattern 和 file_path）
     * 2. 检查 file_path 是否以 rule_pattern 开头
     */
    bool IsPrefixMatch(const std::string& rule_pattern,
                        const std::string& file_path) const;

    /**
     * @brief 通配符匹配
     * @param rule_pattern 规则模式（如 "C:\\Program Files\\*\\app.exe"）
     * @param file_path 文件路径（如 "C:\\Program Files\\App\\app.exe"）
     * @return 匹配返回 true，不匹配返回 false
     * 
     * 算法：
     * 1. 将通配符模式转换为正则表达式
     *    - * → .*
     *    - ? → .
     * 2. 使用 std::regex 匹配
     */
    bool IsWildcardMatch(const std::string& rule_pattern,
                         const std::string& file_path) const;

    /**
     * @brief 正则表达式匹配
     * @param regex_pattern 正则表达式模式（如 "^C:\\\\Users\\\\[^\\\\]+\\\\AppData\\\\.*$"）
     * @param file_path 文件路径（如 "C:\\Users\\John\\AppData\\Roaming\\App\\config.json"）
     * @return 匹配返回 true，不匹配返回 false
     * 
     * 算法：
     * 1. 使用 std::regex 匹配
     */
    bool IsRegexMatch(const std::string& regex_pattern,
                      const std::string& file_path) const;

    /**
     * @brief 将通配符模式转换为正则表达式
     * @param wildcard_pattern 通配符模式（如 "C:\\Program Files\\*\\app.exe"）
     * @return 正则表达式模式（如 "^C:\\Program Files\\.*\\app\\.exe$"）
     */
    std::string WildcardToRegex(const std::string& wildcard_pattern) const;

    /**
     * @brief 解析规则类型字符串
     * @param type_str 规则类型字符串（"include" 或 "exclude"）
     * @return 规则类型枚举
     */
    RuleType ParseRuleType(const std::string& type_str) const;

    /**
     * @brief 解析匹配模式（根据规则路径自动判断）
     * @param pattern 规则路径
     * @return 匹配模式枚举
     * 
     * 判断逻辑：
     * 1. 若包含正则表达式特殊字符（^, $, \, [], ()），则认为是正则匹配
     * 2. 若包含通配符（*, ?），则认为是通配符匹配
     * 3. 否则，认为是前缀匹配
     */
    MatchMode DetectMatchMode(const std::string& pattern) const;
};

} // namespace packager

#endif // COMPAT_RULES_H

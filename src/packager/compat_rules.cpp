/**
 * @file compat_rules.cpp
 * @brief 兼容性规则库实现文件
 * @author Windows C++ 底层开发工程师
 * @date 2026-05-23
 * 
 * 本文件实现 CompatRules 类，负责兼容性规则的加载、匹配和管理功能。
 * 使用 yaml-cpp 库解析 YAML 规则文件。
 */

#include "compat_rules.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <regex>
#include <Windows.h>

// 若已安装 yaml-cpp 库，取消注释以下行
// #include <yaml-cpp/yaml.h>

namespace packager {

// ============================================================================
// 构造函数和析构函数
// ============================================================================

CompatRules::CompatRules() {
    // 初始化成员变量（若有）
}

CompatRules::~CompatRules() {
    // 清理资源（若有）
}

// ============================================================================
// 公共接口实现
// ============================================================================

bool CompatRules::LoadRules(const std::string& rules_file) {
    // 检查规则文件是否存在
    if (!std::filesystem::exists(rules_file)) {
        std::cerr << "[ERROR] 规则文件不存在: " << rules_file << std::endl;
        return false;
    }

    std::cout << "[INFO] 开始加载兼容性规则: " << rules_file << std::endl;

    // TODO: 使用 yaml-cpp 库解析 YAML 文件
    // 示例代码（需要安装 yaml-cpp 库）：
    /*
    try {
        YAML::Node config = YAML::LoadFile(rules_file);
        
        // 解析文件规则
        if (config["file_rules"]) {
            for (const auto& rule_node : config["file_rules"]) {
                std::string type_str = rule_node["type"].as<std::string>();
                std::string path = rule_node["path"].as<std::string>();
                std::string description = rule_node["description"] ? 
                                       rule_node["description"].as<std::string>() : "";
                
                RuleType rule_type = ParseRuleType(type_str);
                MatchMode match_mode = DetectMatchMode(path);
                
                CompatRule rule(rule_type, match_mode, path, description);
                rules_.push_back(rule);
                
                std::cout << "[INFO] 已加载文件规则: " << path 
                          << " (" << (rule_type == RuleType::Include ? "include" : "exclude") 
                          << ")" << std::endl;
            }
        }
        
        // 解析注册表规则
        if (config["registry_rules"]) {
            for (const auto& rule_node : config["registry_rules"]) {
                // 同上...
            }
        }
        
        // 解析进程规则
        if (config["process_rules"]) {
            for (const auto& rule_node : config["process_rules"]) {
                // 同上...
            }
        }
        
    } catch (const YAML::Exception& e) {
        std::cerr << "[ERROR] 解析 YAML 文件失败: " << e.what() << std::endl;
        return false;
    }
    */

    // 简化实现：读取文件内容，解析简单的 YAML 格式（仅用于测试）
    std::ifstream file(rules_file);
    if (!file.is_open()) {
        std::cerr << "[ERROR] 无法打开规则文件: " << rules_file << std::endl;
        return false;
    }

    std::string line;
    int line_num = 0;
    bool in_file_rules = false;
    
    while (std::getline(file, line)) {
        line_num++;
        
        // 去除首尾空格
        line.erase(0, line.find_first_not_of(" \t"));
        line.erase(line.find_last_not_of(" \t") + 1);
        
        // 跳过空行和注释
        if (line.empty() || line[0] == '#') {
            continue;
        }
        
        // 检测文件规则开始
        if (line == "file_rules:") {
            in_file_rules = true;
            continue;
        }
        
        // 解析文件规则（简化格式）
        if (in_file_rules && line.find("- type:") != std::string::npos) {
            // 提取 type
            std::string type_str = "";
            size_t pos = line.find('"');
            if (pos != std::string::npos) {
                size_t end_pos = line.find('"', pos + 1);
                if (end_pos != std::string::npos) {
                    type_str = line.substr(pos + 1, end_pos - pos - 1);
                }
            }
            
            // 读取下一行（path）
            if (std::getline(file, line)) {
                line_num++;
                std::string path = "";
                pos = line.find('"');
                if (pos != std::string::npos) {
                    size_t end_pos = line.find('"', pos + 1);
                    if (end_pos != std::string::npos) {
                        path = line.substr(pos + 1, end_pos - pos - 1);
                    }
                }
                
                // 读取下一行（description）
                std::string description = "";
                if (std::getline(file, line)) {
                    line_num++;
                    pos = line.find('"');
                    if (pos != std::string::npos) {
                        size_t end_pos = line.find('"', pos + 1);
                        if (end_pos != std::string::npos) {
                            description = line.substr(pos + 1, end_pos - pos - 1);
                        }
                    }
                }
                
                // 创建规则
                if (!type_str.empty() && !path.empty()) {
                    RuleType rule_type = ParseRuleType(type_str);
                    MatchMode match_mode = DetectMatchMode(path);
                    
                    CompatRule rule(rule_type, match_mode, path, description);
                    rules_.push_back(rule);
                    
                    std::cout << "[INFO] 已加载文件规则: " << path 
                              << " (" << (rule_type == RuleType::Include ? "include" : "exclude") 
                              << ")" << std::endl;
                }
            }
        }
    }

    std::cout << "[INFO] 加载完成，共 " << rules_.size() << " 条规则" << std::endl;

    return true;
}

bool CompatRules::MatchRule(const std::string& file_path) {
    // 遍历所有规则
    for (const auto& rule : rules_) {
        bool matched = false;
        
        // 根据匹配模式选择算法
        switch (rule.match_mode_) {
            case MatchMode::Prefix:
                matched = IsPrefixMatch(rule.pattern_, file_path);
                break;
            case MatchMode::Wildcard:
                matched = IsWildcardMatch(rule.pattern_, file_path);
                break;
            case MatchMode::Regex:
                matched = IsRegexMatch(rule.pattern_, file_path);
                break;
        }
        
        if (matched) {
            std::cout << "[INFO] 匹配规则: " << rule.pattern_ << " -> " << file_path << std::endl;
            return true;
        }
    }
    
    return false;
}

std::vector<CompatRule> CompatRules::GetMatchedRules(const std::string& file_path) {
    std::vector<CompatRule> matched_rules;
    
    // 遍历所有规则
    for (const auto& rule : rules_) {
        bool matched = false;
        
        // 根据匹配模式选择算法
        switch (rule.match_mode_) {
            case MatchMode::Prefix:
                matched = IsPrefixMatch(rule.pattern_, file_path);
                break;
            case MatchMode::Wildcard:
                matched = IsWildcardMatch(rule.pattern_, file_path);
                break;
            case MatchMode::Regex:
                matched = IsRegexMatch(rule.pattern_, file_path);
                break;
        }
        
        if (matched) {
            matched_rules.push_back(rule);
        }
    }
    
    return matched_rules;
}

// ============================================================================
// 内部辅助方法实现
// ============================================================================

std::string CompatRules::ExpandEnvVars(const std::string& path) const {
    std::string expanded = path;
    
    // 替换环境变量
    struct EnvVar {
        const char* name;
        const char* default_val;
    } env_vars[] = {
        {"APPDATA", ""},
        {"LOCALAPPDATA", ""},
        {"PROGRAMDATA", ""},
        {"TEMP", ""},
        {"PROGRAMFILES", "C:\\Program Files"},
        {"PROGRAMFILES(X86)", "C:\\Program Files (x86)"}
    };
    
    for (const auto& env_var : env_vars) {
        std::string placeholder = "%" + std::string(env_var.name) + "%";
        size_t pos = expanded.find(placeholder);
        if (pos != std::string::npos) {
            // 获取环境变量值
            char buffer[32767];
            DWORD size = GetEnvironmentVariableA(env_var.name, buffer, sizeof(buffer));
            std::string value;
            if (size > 0 && size < sizeof(buffer)) {
                value = std::string(buffer, size);
            } else {
                value = env_var.default_val;
            }
            
            // 替换
            expanded.replace(pos, placeholder.length(), value);
        }
    }
    
    return expanded;
}

bool CompatRules::IsPrefixMatch(const std::string& rule_pattern,
                                  const std::string& file_path) const {
    // 展开环境变量
    std::string expanded_rule = ExpandEnvVars(rule_pattern);
    std::string expanded_path = ExpandEnvVars(file_path);
    
    // 检查前缀
    if (expanded_path.starts_with(expanded_rule)) {
        return true;
    }
    
    return false;
}

bool CompatRules::IsWildcardMatch(const std::string& rule_pattern,
                                   const std::string& file_path) const {
    // 将通配符模式转换为正则表达式
    std::string regex_pattern = WildcardToRegex(rule_pattern);
    
    // 展开环境变量
    std::string expanded_path = ExpandEnvVars(file_path);
    
    // 使用 std::regex 匹配
    try {
        std::regex re(regex_pattern);
        return std::regex_match(expanded_path, re);
    } catch (const std::regex_error& e) {
        std::cerr << "[ERROR] 正则表达式错误: " << e.what() << std::endl;
        return false;
    }
}

bool CompatRules::IsRegexMatch(const std::string& regex_pattern,
                                const std::string& file_path) const {
    // 展开环境变量
    std::string expanded_path = ExpandEnvVars(file_path);
    
    // 使用 std::regex 匹配
    try {
        std::regex re(regex_pattern);
        return std::regex_match(expanded_path, re);
    } catch (const std::regex_error& e) {
        std::cerr << "[ERROR] 正则表达式错误: " << e.what() << std::endl;
        return false;
    }
}

std::string CompatRules::WildcardToRegex(const std::string& wildcard_pattern) const {
    std::string regex_pattern = "^";
    
    for (char c : wildcard_pattern) {
        switch (c) {
            case '*':
                regex_pattern += ".*";
                break;
            case '?':
                regex_pattern += ".";
                break;
            case '.':
            case '\\':
            case '(':
            case ')':
            case '[':
            case ']':
            case '{':
            case '}':
            case '+':
            case '^':
            case '$':
            case '|':
                // 转义正则表达式特殊字符
                regex_pattern += "\\";
                regex_pattern += c;
                break;
            default:
                regex_pattern += c;
                break;
        }
    }
    
    regex_pattern += "$";
    return regex_pattern;
}

RuleType CompatRules::ParseRuleType(const std::string& type_str) const {
    if (type_str == "include") {
        return RuleType::Include;
    } else if (type_str == "exclude") {
        return RuleType::Exclude;
    } else {
        std::cerr << "[WARN] 未知规则类型: " << type_str << "，默认使用 Include" << std::endl;
        return RuleType::Include;
    }
}

MatchMode CompatRules::DetectMatchMode(const std::string& pattern) const {
    // 检查是否包含正则表达式特殊字符
    std::string regex_special_chars = "^$.*+?()[]{}|\\";
    for (char c : pattern) {
        if (regex_special_chars.find(c) != std::string::npos) {
            return MatchMode::Regex;
        }
    }
    
    // 检查是否包含通配符
    if (pattern.find('*') != std::string::npos || 
        pattern.find('?') != std::string::npos) {
        return MatchMode::Wildcard;
    }
    
    // 否则，认为是前缀匹配
    return MatchMode::Prefix;
}

} // namespace packager

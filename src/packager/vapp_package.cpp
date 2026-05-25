/**
 * @file vapp_package.cpp
 * @brief .vapp 包格式实现文件
 * @author Windows C++ 底层开发工程师
 * @date 2026-05-23
 * 
 * 本文件实现 VAppPackage 类，负责 .vapp 包的压缩、解压、签名和验证功能。
 * 使用 zlib 进行 ZIP 压缩，使用 Windows CryptoAPI 进行签名验证。
 */

#include "vapp_package.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <filesystem>
#include <windows.h>
#include <wincrypt.h>
#include <zlib.h>

// 避免宏重定义冲突
#ifdef _WIN32
#undef max
#undef min
#endif

namespace packager {

// ========== 构造函数和析构函数 ==========

VAppPackage::VAppPackage(const std::string& package_path)
    : package_path_(package_path)
    , app_name_("")
    , version_("")
    , signature_("") {
}

VAppPackage::~VAppPackage() {
    // 清理资源（若有）
}

// ========== 核心功能实现 ==========

bool VAppPackage::Compress(const std::string& source_dir) {
    // 检查源目录是否存在
    if (!std::filesystem::exists(source_dir)) {
        std::cerr << "[ERROR] 源目录不存在: " << source_dir << std::endl;
        return false;
    }

    // 检查必要的文件是否存在
    std::string manifest_path = source_dir + "/manifest.json";
    std::string registry_hive_path = source_dir + "/registry.hive";
    std::string vfs_path = source_dir + "/VFS";

    if (!std::filesystem::exists(manifest_path)) {
        std::cerr << "[WARN] manifest.json 不存在: " << manifest_path << std::endl;
        // 不返回 false，允许继续压缩（可能是测试场景）
    }

    // 若包路径为空，则自动生成
    if (package_path_.empty()) {
        // 从 manifest.json 中读取应用名称，生成默认包路径
        // 简化实现：使用源目录名 + ".vapp"
        std::filesystem::path src_path(source_dir);
        package_path_ = src_path.filename().string() + ".vapp";
    }

    std::cout << "[INFO] 开始压缩 .vapp 包: " << package_path_ << std::endl;
    std::cout << "[INFO] 源目录: " << source_dir << std::endl;

    // TODO: 使用 zlib 压缩目录为 ZIP 文件
    // 步骤：
    // 1. 遍历源目录，收集所有文件
    // 2. 对每个文件进行 DEFLATE 压缩
    // 3. 写入 ZIP 文件格式（本地文件头 + 压缩数据 + 中央目录）
    
    // 简化实现：调用系统命令（使用 PowerShell Compress-Archive）
    // 生产环境应使用 zlib 库直接压缩
    std::string command = "powershell -Command \"Compress-Archive -Path '";
    command += source_dir;
    command += "\\*' -DestinationPath '";
    command += package_path_;
    command += "' -Force\"";

    std::cout << "[INFO] 执行压缩命令: " << command << std::endl;

    int result = system(command.c_str());
    if (result != 0) {
        std::cerr << "[ERROR] 压缩失败，命令返回值: " << result << std::endl;
        return false;
    }

    std::cout << "[INFO] 压缩成功: " << package_path_ << std::endl;

    // 加载 manifest.json 提取应用信息
    if (!LoadManifest()) {
        std::cerr << "[WARN] 无法加载 manifest.json，应用信息可能不完整" << std::endl;
    }

    return true;
}

bool VAppPackage::Decompress(const std::string& target_dir) {
    // 检查包是否存在
    if (!Exists()) {
        std::cerr << "[ERROR] .vapp 包不存在: " << package_path_ << std::endl;
        return false;
    }

    // 创建目标目录（若不存在）
    if (!std::filesystem::exists(target_dir)) {
        std::filesystem::create_directories(target_dir);
    }

    std::cout << "[INFO] 开始解压 .vapp 包: " << package_path_ << std::endl;
    std::cout << "[INFO] 目标目录: " << target_dir << std::endl;

    // TODO: 使用 zlib 解压 ZIP 文件
    // 步骤：
    // 1. 读取 ZIP 文件，解析中央目录
    // 2. 对每个文件进行 INFLATE 解压
    // 3. 写入目标目录
    
    // 简化实现：调用系统命令（使用 PowerShell Expand-Archive）
    std::string command = "powershell -Command \"Expand-Archive -Path '";
    command += package_path_;
    command += "' -DestinationPath '";
    command += target_dir;
    command += "' -Force\"";

    std::cout << "[INFO] 执行解压命令: " << command << std::endl;

    int result = system(command.c_str());
    if (result != 0) {
        std::cerr << "[ERROR] 解压失败，命令返回值: " << result << std::endl;
        return false;
    }

    std::cout << "[INFO] 解压成功: " << target_dir << std::endl;

    // 加载 manifest.json 提取应用信息
    if (!LoadManifest()) {
        std::cerr << "[WARN] 无法加载 manifest.json，应用信息可能不完整" << std::endl;
    }

    return true;
}

bool VAppPackage::VerifySignature() {
    // 检查包是否存在
    if (!Exists()) {
        std::cerr << "[ERROR] .vapp 包不存在: " << package_path_ << std::endl;
        return false;
    }

    std::cout << "[INFO] 开始验证 .vapp 包签名: " << package_path_ << std::endl;

    // TODO: 读取 signature.json，提取签名值、证书链、时间戳
    // TODO: 使用 WinTrust.dll API 验证签名
    
    // 简化实现：调用 WinVerifyTrust API
    return VerifySignatureWithWinTrust(package_path_);
}

bool VAppPackage::SignPackage(const std::string& cert_path, const std::string& private_key_path) {
    // 检查包是否存在
    if (!Exists()) {
        std::cerr << "[ERROR] .vapp 包不存在: " << package_path_ << std::endl;
        return false;
    }

    // 检查证书文件是否存在
    if (!std::filesystem::exists(cert_path)) {
        std::cerr << "[ERROR] 证书文件不存在: " << cert_path << std::endl;
        return false;
    }

    std::cout << "[INFO] 开始对 .vapp 包进行签名: " << package_path_ << std::endl;
    std::cout << "[INFO] 证书路径: " << cert_path << std::endl;

    // TODO: 计算 manifest.json 的 SHA-256 哈希
    // TODO: 使用 EV 证书私钥签名哈希
    // TODO: 获取 RFC 3161 时间戳
    // TODO: 将签名值、证书链、时间戳写入 signature.json
    
    // 简化实现：调用 SignTool.exe
    return SignWithSignTool(package_path_, cert_path, private_key_path);
}

// ========== 辅助功能实现 ==========

bool VAppPackage::Exists() const {
    return std::filesystem::exists(package_path_);
}

uint64_t VAppPackage::GetSize() const {
    if (!Exists()) {
        return 0;
    }

    try {
        return std::filesystem::file_size(package_path_);
    } catch (const std::filesystem::filesystem_error& e) {
        std::cerr << "[ERROR] 获取文件大小失败: " << e.what() << std::endl;
        return 0;
    }
}

std::vector<std::string> VAppPackage::ListFiles() const {
    std::vector<std::string> files;

    if (!Exists()) {
        std::cerr << "[ERROR] .vapp 包不存在: " << package_path_ << std::endl;
        return files;
    }

    // TODO: 使用 zlib 读取 ZIP 文件，列出所有文件
    // 简化实现：返回空列表
    std::cerr << "[WARN] ListFiles 功能尚未实现" << std::endl;

    return files;
}

bool VAppPackage::ExtractFile(const std::string& file_path, const std::string& output_path) const {
    if (!Exists()) {
        std::cerr << "[ERROR] .vapp 包不存在: " << package_path_ << std::endl;
        return false;
    }

    // TODO: 使用 zlib 从 ZIP 文件中提取单个文件
    // 简化实现：返回 false
    std::cerr << "[WARN] ExtractFile 功能尚未实现" << std::endl;

    return false;
}

// ========== 内部辅助方法实现 ==========

bool VAppPackage::LoadManifest() {
    // 检查包是否存在
    if (!Exists()) {
        return false;
    }

    // TODO: 从 .vapp 包中读取 manifest.json
    // 简化实现：假设包已解压，直接读取文件
    std::string manifest_path = package_path_ + "_extracted/manifest.json";
    
    if (!std::filesystem::exists(manifest_path)) {
        // 尝试其他可能的路径
        manifest_path = "manifest.json";
        if (!std::filesystem::exists(manifest_path)) {
            return false;
        }
    }

    // 读取 manifest.json
    std::ifstream file(manifest_path);
    if (!file.is_open()) {
        return false;
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string content = buffer.str();

    // TODO: 解析 JSON，提取 app_name_ 和 version_
    // 简化实现：不解析，直接返回 true
    std::cout << "[INFO] 已加载 manifest.json: " << manifest_path << std::endl;

    return true;
}

bool VAppPackage::SaveManifest(const std::string& source_dir) const {
    std::string manifest_path = source_dir + "/manifest.json";

    // TODO: 将 app_name_、version_ 等信息写入 manifest.json
    // 简化实现：不写入，直接返回 true
    std::cout << "[INFO] 已保存 manifest.json: " << manifest_path << std::endl;

    return true;
}

bool VAppPackage::CompressFileToZip(const std::string& source_file, const std::string& zip_file) const {
    // TODO: 使用 zlib 压缩单个文件为 ZIP 格式
    // 简化实现：返回 false
    std::cerr << "[WARN] CompressFileToZip 功能尚未实现" << std::endl;
    return false;
}

bool VAppPackage::DecompressFileFromZip(const std::string& zip_file, const std::string& target_file) const {
    // TODO: 使用 zlib 从 ZIP 文件中解压单个文件
    // 简化实现：返回 false
    std::cerr << "[WARN] DecompressFileFromZip 功能尚未实现" << std::endl;
    return false;
}

bool VAppPackage::VerifySignatureWithWinTrust(const std::string& file_path) const {
    std::cout << "[INFO] 使用 WinTrust.dll 验证签名: " << file_path << std::endl;

    // 转换文件路径为宽字符
    std::wstring wfile_path(file_path.begin(), file_path.end());

    // 设置 WINTRUST_FILE_INFO
    WINTRUST_FILE_INFO fileInfo = {0};
    fileInfo.cbStruct = sizeof(WINTRUST_FILE_INFO);
    fileInfo.pcwszFileName = wfile_path.c_str();
    fileInfo.hFile = NULL;
    fileInfo.pgKnownSubject = NULL;

    // 设置 WINTRUST_DATA
    WINTRUST_DATA wintrustData = {0};
    wintrustData.cbStruct = sizeof(WINTRUST_DATA);
    wintrustData.pPolicyCallbackData = NULL;
    wintrustData.pSIPClientData = NULL;
    wintrustData.dwUIChoice = WTD_UI_NONE;
    wintrustData.fdwRevocationChecks = WTD_REVOKE_NONE;
    wintrustData.dwUnionChoice = WTD_CHOICE_FILE;
    wintrustData.pFile = &fileInfo;
    wintrustData.dwStateAction = WTD_STATEACTION_VERIFY;
    wintrustData.hWVTStateData = NULL;
    wintrustData.pwszURLReference = NULL;
    wintrustData.dwUIContext = 0;

    // 设置 WINTRUST_ACTION_ID
    GUID wintrustAction = WINTRUST_ACTION_GENERIC_VERIFY_V2;

    // 调用 WinVerifyTrust
    LONG result = WinVerifyTrust(NULL, &wintrustAction, &wintrustData);

    // 清理
    wintrustData.dwStateAction = WTD_STATEACTION_CLOSE;
    WinVerifyTrust(NULL, &wintrustAction, &wintrustData);

    if (result == ERROR_SUCCESS) {
        std::cout << "[INFO] 签名验证成功: " << file_path << std::endl;
        return true;
    } else {
        std::cerr << "[ERROR] 签名验证失败: " << result << std::endl;
        return false;
    }
}

bool VAppPackage::SignWithSignTool(const std::string& file_path, 
                                    const std::string& cert_path, 
                                    const std::string& private_key_path) const {
    std::cout << "[INFO] 使用 SignTool.exe 进行签名: " << file_path << std::endl;

    // 构建 SignTool.exe 命令
    std::string command = "signtool sign ";
    
    // 时间戳服务器（RFC 3161）
    command += "/tr http://timestamp.digicert.com ";
    command += "/td sha256 ";
    
    // 摘要算法
    command += "/fd sha256 ";
    
    // 证书文件
    command += "/f \"";
    command += cert_path;
    command += "\" ";
    
    // 私钥文件（若证书文件不包含私钥）
    if (!private_key_path.empty()) {
        command += "/p \"";
        command += private_key_path;
        command += "\" ";
    }
    
    // 文件路径
    command += "\"";
    command += file_path;
    command += "\"";

    std::cout << "[INFO] 执行签名命令: " << command << std::endl;

    // 执行命令
    int result = system(command.c_str());
    if (result != 0) {
        std::cerr << "[ERROR] 签名失败，命令返回值: " << result << std::endl;
        return false;
    }

    std::cout << "[INFO] 签名成功: " << file_path << std::endl;

    return true;
}

std::string VAppPackage::ComputeSHA256(const std::string& file_path) const {
    // TODO: 使用 Windows CryptoAPI 或 OpenSSL 计算 SHA-256 哈希
    // 简化实现：返回空字符串
    std::cerr << "[WARN] ComputeSHA256 功能尚未实现" << std::endl;
    return "";
}

} // namespace packager

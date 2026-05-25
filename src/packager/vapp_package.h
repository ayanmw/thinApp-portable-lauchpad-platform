/**
 * @file vapp_package.h
 * @brief .vapp 包格式实现头文件
 * @author Windows C++ 底层开发工程师
 * @date 2026-05-23
 * 
 * 本文件定义 VAppPackage 类，负责 .vapp 包的压缩、解压、签名和验证功能。
 * .vapp 包基于 ZIP 格式（使用 zlib 压缩），包含应用的所有文件和元数据。
 */

#ifndef VAPP_PACKAGE_H
#define VAPP_PACKAGE_H

#include <string>
#include <vector>
#include <memory>

namespace packager {

/**
 * @brief .vapp 包类
 * 
 * 负责 .vapp 包的创建、压缩、解压、签名和验证。
 * .vapp 包实际上是 ZIP 文件（仅改了后缀），包含：
 * - manifest.json：应用元数据
 * - registry.hive：虚拟注册表 hive
 * - VFS\：虚拟文件系统目录
 * - signature.json：签名信息（可选）
 * - rules.yaml：兼容性规则（可选）
 */
class VAppPackage {
public:
    /**
     * @brief 构造函数
     * @param package_path .vapp 包路径
     */
    explicit VAppPackage(const std::string& package_path = "");

    /**
     * @brief 析构函数
     */
    ~VAppPackage();

    // ========== 包信息获取/设置 ==========

    /**
     * @brief 获取包路径
     * @return 包路径
     */
    std::string GetPackagePath() const { return package_path_; }

    /**
     * @brief 设置包路径
     * @param package_path 包路径
     */
    void SetPackagePath(const std::string& package_path) { package_path_ = package_path; }

    /**
     * @brief 获取应用名称
     * @return 应用名称
     */
    std::string GetAppName() const { return app_name_; }

    /**
     * @brief 设置应用名称
     * @param app_name 应用名称
     */
    void SetAppName(const std::string& app_name) { app_name_ = app_name; }

    /**
     * @brief 获取应用版本
     * @return 应用版本
     */
    std::string GetVersion() const { return version_; }

    /**
     * @brief 设置应用版本
     * @param version 应用版本
     */
    void SetVersion(const std::string& version) { version_ = version; }

    /**
     * @brief 获取签名信息
     * @return 签名信息（JSON 字符串）
     */
    std::string GetSignature() const { return signature_; }

    /**
     * @brief 设置签名信息
     * @param signature 签名信息（JSON 字符串）
     */
    void SetSignature(const std::string& signature) { signature_ = signature; }

    // ========== 核心功能 ==========

    /**
     * @brief 压缩目录为 .vapp 包
     * @param source_dir 源目录（包含 apps.json、VFS\、registry.hive 等）
     * @return 成功返回 true，失败返回 false
     * 
     * 功能说明：
     * 1. 读取源目录下的 apps.json（应用元数据）
     * 2. 压缩 VFS\ 目录（虚拟文件系统）
     * 3. 压缩 registry.hive（虚拟注册表）
     * 4. 将所有文件打包为 ZIP 格式，保存为 .vapp 文件
     * 
     * 使用 zlib 库进行 ZIP 压缩。
     */
    bool Compress(const std::string& source_dir);

    /**
     * @brief 解压 .vapp 包到目标目录
     * @param target_dir 目标目录
     * @return 成功返回 true，失败返回 false
     * 
     * 功能说明：
     * 1. 读取 .vapp 包（实际上是 ZIP 文件）
     * 2. 解压所有文件到目标目录
     * 3. 恢复 apps.json、VFS\、registry.hive 等文件
     * 
     * 使用 zlib 库进行 ZIP 解压。
     */
    bool Decompress(const std::string& target_dir);

    /**
     * @brief 验证 .vapp 包的数字签名
     * @return 签名有效返回 true，无效返回 false
     * 
     * 功能说明：
     * 1. 读取包中的 signature.json 文件
     * 2. 提取签名值、证书链、时间戳
     * 3. 使用 WinTrust.dll API 验证签名
     * 4. 检查证书链是否可信（信任根证书）
     * 5. 验证时间戳（证书是否过期）
     * 
     * 使用 Windows CryptoAPI（WinTrust.dll）。
     */
    bool VerifySignature();

    /**
     * @brief 对 .vapp 包进行 EV 代码签名
     * @param cert_path 证书路径（.pfx 或 .p12 文件）
     * @param private_key_path 私钥路径（可选，若证书文件已包含私钥）
     * @return 成功返回 true，失败返回 false
     * 
     * 功能说明：
     * 1. 计算 manifest.json 的 SHA-256 哈希
     * 2. 使用 EV 证书私钥签名哈希（RSA-2048 或 ECC-256）
     * 3. 获取 RFC 3161 时间戳（支持长期验证）
     * 4. 将签名值、证书链、时间戳写入 signature.json
     * 
     * 使用 SignTool.exe 进行签名（需提前申请 EV 证书）。
     * 命令示例：signtool sign /tr http://timestamp.digicert.com /td sha256 /fd sha256 /a package.vapp
     */
    bool SignPackage(const std::string& cert_path, const std::string& private_key_path = "");

    // ========== 辅助功能 ==========

    /**
     * @brief 检查 .vapp 包是否存在
     * @return 存在返回 true，不存在返回 false
     */
    bool Exists() const;

    /**
     * @brief 获取 .vapp 包大小（字节）
     * @return 包大小（字节），失败返回 0
     */
    uint64_t GetSize() const;

    /**
     * @brief 列出 .vapp 包中的所有文件
     * @return 文件列表
     */
    std::vector<std::string> ListFiles() const;

    /**
     * @brief 提取单个文件从 .vapp 包中
     * @param file_path 文件在包中的路径（如 "manifest.json"）
     * @param output_path 输出路径
     * @return 成功返回 true，失败返回 false
     */
    bool ExtractFile(const std::string& file_path, const std::string& output_path) const;

private:
    // ========== 成员变量 ==========
    
    std::string package_path_;   ///< .vapp 包路径
    std::string app_name_;       ///< 应用名称（从 manifest.json 读取）
    std::string version_;        ///< 应用版本（从 manifest.json 读取）
    std::string signature_;      ///< 签名信息（JSON 字符串）

    // ========== 内部辅助方法 ==========

    /**
     * @brief 读取 manifest.json 并提取应用信息
     * @return 成功返回 true，失败返回 false
     */
    bool LoadManifest();

    /**
     * @brief 保存 manifest.json（使用当前应用信息）
     * @param source_dir 源目录
     * @return 成功返回 true，失败返回 false
     */
    bool SaveManifest(const std::string& source_dir) const;

    /**
     * @brief 使用 zlib 压缩文件为 ZIP 格式
     * @param source_file 源文件
     * @param zip_file ZIP 文件
     * @return 成功返回 true，失败返回 false
     */
    bool CompressFileToZip(const std::string& source_file, const std::string& zip_file) const;

    /**
     * @brief 使用 zlib 解压 ZIP 格式文件
     * @param zip_file ZIP 文件
     * @param target_file 目标文件
     * @return 成功返回 true，失败返回 false
     */
    bool DecompressFileFromZip(const std::string& zip_file, const std::string& target_file) const;

    /**
     * @brief 调用 WinTrust.dll API 验证签名
     * @param file_path 文件路径
     * @return 签名有效返回 true，无效返回 false
     */
    bool VerifySignatureWithWinTrust(const std::string& file_path) const;

    /**
     * @brief 调用 SignTool.exe 进行签名
     * @param file_path 文件路径
     * @param cert_path 证书路径
     * @param private_key_path 私钥路径
     * @return 成功返回 true，失败返回 false
     */
    bool SignWithSignTool(const std::string& file_path, 
                          const std::string& cert_path, 
                          const std::string& private_key_path) const;

    /**
     * @brief 计算文件的 SHA-256 哈希
     * @param file_path 文件路径
     * @return 哈希值（十六进制字符串），失败返回空字符串
     */
    std::string ComputeSHA256(const std::string& file_path) const;
};

} // namespace packager

#endif // VAPP_PACKAGE_H

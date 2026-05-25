/**
 * @file app_store.h
 * @brief 应用商店集成头文件
 * @author Windows C++ 底层开发工程师
 * @date 2026-05-23
 * 
 * 本文件定义 AppStore 类，负责应用商店的 API 调用和本地数据库管理。
 * 
 * 功能：
 * - 上传 .vapp 包到应用商店（POST /api/v1/apps/upload）
 * - 从应用商店下载 .vapp 包（GET /api/v1/apps/download）
 * - 搜索应用商店中的应用（GET /api/v1/apps/search）
 * - 更新本地 SQLite 数据库（缓存应用元数据）
 * 
 * 使用技术：
 * - WinHTTP API：调用 RESTful API
 * - SQLite3 库：操作本地数据库
 */

#ifndef APP_STORE_H
#define APP_STORE_H

#include <string>
#include <vector>
#include <memory>

namespace packager {

// ============================================================================
// 应用元数据结构体
// ============================================================================

/**
 * @brief 应用元数据结构体
 * 
 * 表示应用商店中的应用元数据。
 */
struct AppMetadata {
    std::string id;              ///< 应用 ID（如 "app_123456"）
    std::string name;            ///< 应用名称（如 "Notepad++"）
    std::string version;         ///< 应用版本（如 "8.6.4"）
    std::string publisher;       ///< 发布者（如 "Notepad++ Team"）
    std::string description;      ///< 应用描述
    std::string icon_url;        ///< 图标 URL
    std::string download_url;    ///< 下载 URL
    uint64_t file_size;          ///< 文件大小（字节）
    std::string sha256;         ///< SHA-256 哈希
    std::string signature;       ///< 签名信息（JSON 字符串）
    int download_count;          ///< 下载量
    double rating;               ///< 评分（0.0-5.0）
    int reviews;                 ///< 评论数
    std::string category;        ///< 分类（office/dev/design/game/...）
    std::string created_at;      ///< 创建时间（ISO 8601）
    std::string updated_at;      ///< 更新时间（ISO 8601）
};

// ============================================================================
// AppStore 类定义
// ============================================================================

/**
 * @brief 应用商店类
 * 
 * 负责应用商店的 API 调用和本地数据库管理。
 * 
 * 使用示例：
 * @code
 * packager::AppStore store;
 * 
 * // 设置 API 基础 URL
 * store.SetApiBaseUrl("https://api.ai-thinapp.com/v1");
 * 
 * // 设置本地数据库路径
 * store.SetLocalDbPath("local_apps.db");
 * 
 * // 上传应用
 * if (!store.UploadApp("notepad++.vapp")) {
 *     printf("上传失败\n");
 *     return;
 * }
 * 
 * // 下载应用
 * if (!store.DownloadApp("Notepad++", "8.6.4")) {
 *     printf("下载失败\n");
 *     return;
 * }
 * 
 * // 搜索应用
 * std::vector<std::string> results;
 * if (store.SearchApps("Notepad", results)) {
 *     printf("找到 %zu 个结果\n", results.size());
 * }
 * 
 * // 更新本地数据库
 * if (!store.UpdateLocalDB()) {
 *     printf("更新本地数据库失败\n");
 *     return;
 * }
 * @endcode
 */
class AppStore {
public:
    /**
     * @brief 构造函数
     * @param api_base_url API 基础 URL（如 "https://api.ai-thinapp.com/v1"）
     * @param local_db_path 本地数据库路径（如 "local_apps.db"）
     */
    explicit AppStore(const std::string& api_base_url = "",
                      const std::string& local_db_path = "");

    /**
     * @brief 析构函数
     */
    ~AppStore();

    // ========================================================================
    // 配置方法
    // ========================================================================

    /**
     * @brief 获取 API 基础 URL
     * @return API 基础 URL
     */
    std::string GetApiBaseUrl() const { return api_base_url_; }

    /**
     * @brief 设置 API 基础 URL
     * @param api_base_url API 基础 URL（如 "https://api.ai-thinapp.com/v1"）
     */
    void SetApiBaseUrl(const std::string& api_base_url) { api_base_url_ = api_base_url; }

    /**
     * @brief 获取本地数据库路径
     * @return 本地数据库路径
     */
    std::string GetLocalDbPath() const { return local_db_path_; }

    /**
     * @brief 设置本地数据库路径
     * @param local_db_path 本地数据库路径（如 "local_apps.db"）
     */
    void SetLocalDbPath(const std::string& local_db_path) { local_db_path_ = local_db_path; }

    // ========================================================================
    // 核心功能
    // ========================================================================

    /**
     * @brief 上传 .vapp 包到应用商店
     * @param vapp_path .vapp 包路径
     * @return 成功返回 true，失败返回 false
     * 
     * 功能说明：
     * 1. 读取 .vapp 包文件
     * 2. 调用 RESTful API：`POST /api/v1/apps/upload`
     * 3. 发送 .vapp 包文件（multipart/form-data）
     * 4. 解析响应（成功/失败）
     * 
     * API 请求格式：
     * POST /api/v1/apps/upload
     * Content-Type: multipart/form-data
     * 
     * Body:
     * - file: .vapp 包文件
     * - metadata: 应用元数据（JSON 字符串）
     * 
     * API 响应格式：
     * ```json
     * {
     *   "code": 0,
     *   "message": "success",
     *   "data": {
     *     "app_id": "app_123456"
     *   }
     * }
     * ```
     * 
     * 使用 WinHTTP API 发送 HTTP 请求。
     */
    bool UploadApp(const std::string& vapp_path);

    /**
     * @brief 从应用商店下载 .vapp 包
     * @param app_name 应用名称
     * @param version 应用版本
     * @return 成功返回 true，失败返回 false
     * 
     * 功能说明：
     * 1. 调用 RESTful API：`GET /api/v1/apps/download?name={app_name}&version={version}`
     * 2. 解析响应（重定向到 CDN 下载地址）
     * 3. 从 CDN 下载 .vapp 包
     * 4. 保存到本地
     * 
     * API 请求格式：
     * GET /api/v1/apps/download?name=Notepad++&version=8.6.4
     * 
     * API 响应格式：
     * HTTP/1.1 302 Found
     * Location: https://cdn.ai-thinapp.com/vapp/notepad++-8.6.4.vapp
     * 
     * 使用 WinHTTP API 发送 HTTP 请求。
     */
    bool DownloadApp(const std::string& app_name, const std::string& version);

    /**
     * @brief 搜索应用商店中的应用
     * @param keyword 搜索关键词
     * @param results 搜索结果（输出参数）
     * @return 成功返回 true，失败返回 false
     * 
     * 功能说明：
     * 1. 调用 RESTful API：`GET /api/v1/apps/search?q={keyword}`
     * 2. 解析响应（JSON 格式）
     * 3. 提取应用列表，保存到 results 中
     * 
     * API 请求格式：
     * GET /api/v1/apps/search?q=notepad
     * 
     * API 响应格式：
     * ```json
     * {
     *   "code": 0,
     *   "message": "success",
     *   "data": {
     *     "total": 5,
     *     "apps": [
     *       {
     *         "id": "app_123456",
     *         "name": "Notepad++",
     *         "version": "8.6.4",
     *         ...
     *       }
     *     ]
     *   }
     * }
     * ```
     * 
     * 使用 WinHTTP API 发送 HTTP 请求。
     */
    bool SearchApps(const std::string& keyword, std::vector<std::string>& results);

    /**
     * @brief 更新本地 SQLite 数据库
     * @return 成功返回 true，失败返回 false
     * 
     * 功能说明：
     * 1. 调用 RESTful API：`GET /api/v1/apps`（获取所有应用元数据）
     * 2. 解析响应（JSON 格式）
     * 3. 更新本地 SQLite 数据库（删除旧数据，插入新数据）
     * 
     * 本地数据库表结构：
     * ```sql
     * CREATE TABLE apps (
     *     id TEXT PRIMARY KEY,
     *     name TEXT NOT NULL,
     *     version TEXT NOT NULL,
     *     publisher TEXT,
     *     description TEXT,
     *     icon_url TEXT,
     *     download_url TEXT NOT NULL,
     *     file_size INTEGER NOT NULL,
     *     sha256 TEXT NOT NULL,
     *     download_count INTEGER DEFAULT 0,
     *     rating REAL DEFAULT 0.0,
     *     category TEXT,
     *     created_at TEXT NOT NULL,
     *     updated_at TEXT NOT NULL
     * );
     * ```
     * 
     * 使用 SQLite3 库操作本地数据库。
     */
    bool UpdateLocalDB();

    // ========================================================================
    // 辅助功能
    // ========================================================================

    /**
     * @brief 获取应用元数据
     * @param app_id 应用 ID
     * @param metadata 应用元数据（输出参数）
     * @return 成功返回 true，失败返回 false
     * 
     * 功能说明：
     * 1. 先从本地数据库查询
     * 2. 若本地数据库不存在，则从应用商店 API 查询
     * 3. 返回应用元数据
     */
    bool GetAppMetadata(const std::string& app_id, AppMetadata& metadata);

    /**
     * @brief 删除应用（从本地数据库和应用商店）
     * @param app_id 应用 ID
     * @return 成功返回 true，失败返回 false
     * 
     * 功能说明：
     * 1. 从本地数据库删除
     * 2. 调用 RESTful API：`DELETE /api/v1/apps/{app_id}`
     */
    bool DeleteApp(const std::string& app_id);

private:
    // ========================================================================
    // 私有成员变量
    // ========================================================================

    /**
     * @brief API 基础 URL
     * 
     * 默认值："https://api.ai-thinapp.com/v1"
     */
    std::string api_base_url_;

    /**
     * @brief 本地数据库路径
     * 
     * 默认值："local_apps.db"
     */
    std::string local_db_path_;

    // ========================================================================
    // 内部辅助方法
    // ========================================================================

    /**
     * @brief 使用 WinHTTP API 发送 HTTP 请求
     * @param url 请求 URL
     * @param method HTTP 方法（"GET"、"POST"、"DELETE" 等）
     * @param body 请求体（可选）
     * @param response 响应内容（输出参数）
     * @return 成功返回 true，失败返回 false
     */
    bool SendHttpRequest(const std::string& url,
                        const std::string& method,
                        const std::string& body,
                        std::string& response) const;

    /**
     * @brief 解析 JSON 响应
     * @param json_str JSON 字符串
     * @param result 解析结果（输出参数）
     * @return 成功返回 true，失败返回 false
     * 
     * 解析 JSON 格式：
     * ```json
     * {
     *   "code": 0,
     *   "message": "success",
     *   "data": {
     *     ...
     *   }
     * }
     * ```
     */
    bool ParseJsonResponse(const std::string& json_str,
                           std::string& result) const;

    /**
     * @brief 初始化本地数据库
     * @return 成功返回 true，失败返回 false
     * 
     * 功能说明：
     * 1. 检查本地数据库文件是否存在
     * 2. 若不存在，则创建数据库和表
     * 3. 若已存在，则检查表结构是否完整
     */
    bool InitLocalDB();

    /**
     * @brief 执行 SQL 语句（不返回结果）
     * @param sql SQL 语句
     * @return 成功返回 true，失败返回 false
     */
    bool ExecuteSQL(const std::string& sql) const;

    /**
     * @brief 查询 SQL 语句（返回结果）
     * @param sql SQL 语句
     * @param results 查询结果（输出参数）
     * @return 成功返回 true，失败返回 false
     */
    bool QuerySQL(const std::string& sql,
                   std::vector<std::string>& results) const;
};

} // namespace packager

#endif // APP_STORE_H

/**
 * @file hook_engine.h
 * @brief Hook 引擎模块头文件
 * 
 * 基于 MinHook 实现 API Hook，用于拦截文件系统、注册表和进程创建相关 API。
 * 实现应用沙箱的文件系统和注册表虚拟化。
 * 
 * @author AI ThinApp Team
 * @date 2026-05-23
 */

// ============================================================================
// 捕获事件类型和捕获记录结构（全局命名空间，供 C 函数使用）
// ============================================================================

/**
 * @brief 捕获事件类型（全局命名空间，C 函数需要）
 */
typedef enum _CAPTURE_EVENT_TYPE {
    CAPTURE_EVENT_FILE_CREATE,       // 文件创建
    CAPTURE_EVENT_FILE_WRITE,        // 文件写入
    CAPTURE_EVENT_FILE_READ,         // 文件读取
    CAPTURE_EVENT_FILE_DELETE,       // 文件删除
    CAPTURE_EVENT_REG_CREATE_KEY,    // 注册表键创建
    CAPTURE_EVENT_REG_SET_VALUE,     // 注册表值设置
    CAPTURE_EVENT_REG_DELETE_KEY,   // 注册表键删除
    CAPTURE_EVENT_REG_DELETE_VALUE   // 注册表值删除
} CAPTURE_EVENT_TYPE;

/**
 * @brief 捕获记录结构（全局命名空间，C 函数需要）
 */
typedef struct _CAPTURE_RECORD {
    CAPTURE_EVENT_TYPE eventType;    // 事件类型
    WCHAR path[MAX_PATH];            // 文件/注册表路径
    BYTE data[1024];                // 数据（可选，如文件内容、注册表值）
    size_t dataSize;                 // 数据大小
    FILETIME timestamp;              // 时间戳
    DWORD processId;                 // 进程 ID
    WCHAR processName[MAX_PATH];     // 进程名称
} CAPTURE_RECORD;

#ifndef HOOK_ENGINE_H
#define HOOK_ENGINE_H

#include <Windows.h>
#include <stdint.h>
#include <atomic>
#include <thread>
#include <vector>  // 添加缺少的 vector 头文件
#include "vfs/vfs_cache.h"
#include "vfs/path_redirect.h"

// 注意：extern "C" 只能用于 C 函数声明，不能包含 C++ 的命名空间和类
// 所以将 extern "C" 块移动到文件末尾的 C 函数声明处

namespace ThinApp {
namespace Engine {
namespace Hook {


/**
 * @brief Hook 引擎类（单例模式）
 * 
 * 封装 Hook 引擎的所有功能，包括初始化、安装/卸载 Hook、
 * Hook 覆盖检测和自动重装。
 */
class HookEngine {
public:
    /**
     * @brief 获取单例实例
     */
    static HookEngine& GetInstance();
    
    /**
     * @brief 初始化 Hook 引擎
     */
    BOOL Initialize();
    
    /**
     * @brief 安装所有预定义的 Hook
     */
    BOOL InstallHooks();
    
    /**
     * @brief 卸载所有 Hook
     */
    BOOL UninstallHooks();
    
    /**
     * @brief 清理 Hook 引擎
     */
    BOOL Shutdown();
    
    /**
     * @brief 检查 Hook 引擎是否已初始化
     */
    BOOL IsInitialized() const;
    
    /**
     * @brief 检查 Hook 覆盖情况（定时检测 + 事件驱动检测）
     * 
     * 多检测点：
     * 1. 定时检测：每 1 秒检查一次
     * 2. 事件驱动检测：在 Hook 函数入口处调用
     */
    void CheckHookCoverage();
    
    /**
     * @brief 重新安装被覆盖的 Hook（指数退避）
     * 
     * 自动重装策略：
     * - 第1次重试：等待1秒
     * - 第2次重试：等待2秒
     * - 第3次重试：等待4秒
     * - 第4次重试：等待8秒
     * - 最多重试5次，若仍失败则记录错误并停止重试
     */
    void ReloadHook();
    
    /**
     * @brief 启动 Hook 覆盖检测线程
     */
    BOOL StartHookCheck();
    
    /**
     * @brief 停止 Hook 覆盖检测线程
     */
    BOOL StopHookCheck();
    
    /**
     * @brief 启用捕获模式
     */
    BOOL EnableCaptureMode();
    
    /**
     * @brief 禁用捕获模式
     */
    BOOL DisableCaptureMode();
    
    /**
     * @brief 检查捕获模式是否已启用
     */
    BOOL IsCaptureModeEnabled() const;
    
    /**
     * @brief 清除捕获记录
     */
    BOOL ClearCaptureRecords();
    
    /**
     * @brief 获取捕获记录数量
     */
    size_t GetCaptureRecordCount() const;
    
    /**
     * @brief 获取捕获记录
     */
    size_t GetCaptureRecords(CAPTURE_RECORD* records, size_t maxRecords);
    
private:
    /**
     * @brief 私有构造函数（单例模式）
     */
    HookEngine();
    
    /**
     * @brief 私有析构函数
     */
    ~HookEngine();
    
    /**
     * @brief 禁用拷贝构造函数
     */
    HookEngine(const HookEngine&) = delete;
    
    /**
     * @brief 禁用赋值运算符
     */
    HookEngine& operator=(const HookEngine&) = delete;
    
    // 注意：HookCheckThreadProc 已改为使用 lambda 创建线程
    // 线程实现见 HookCheckThreadProcImpl()
    
    /**
     * @brief 记录捕获事件（线程安全）
     */
    void RecordCaptureEvent(CAPTURE_EVENT_TYPE eventType,
                            const WCHAR* path,
                            const BYTE* data,
                            size_t dataSize);
    
private:
    // ========== 成员变量 ==========
    
    /** @brief 是否已初始化 */
    BOOL initialized_;
    
    /** @brief Hook 覆盖检测线程 */
    std::thread* check_thread_;
    
    /** @brief 停止检测标志（原子变量，线程安全） */
    std::atomic<bool> stop_check_;
    
    /** @brief 连续失败次数（用于指数退避） */
    int failed_count_;
    
    /** @brief 最大重试次数 */
    static const int MAX_RETRY_COUNT = 5;
    
    /** @brief 最大退避延迟（秒） */
    static const int MAX_BACKOFF_DELAY = 60;
    
    /** @brief 捕获模式是否已启用 */
    BOOL capture_mode_enabled_;
    
    /** @brief 捕获记录列表 */
    std::vector<CAPTURE_RECORD> capture_records_;
    
    /** @brief 捕获记录锁（临界区，mutable 允许在 const 函数中修改） */
    mutable CRITICAL_SECTION capture_records_lock_;
    
    /** @brief VFS 缓存实例 */
    std::unique_ptr<ThinApp::Engine::VFS::VfsCache> vfs_cache_;
    
    /** @brief 路径重定向实例 */
    std::unique_ptr<ThinApp::Engine::VFS::PathRedirect> path_redirect_;
    
    /** @brief 原 CreateFileW 函数指针 */
    HANDLE (WINAPI *original_create_file_w_)(
        LPCWSTR, DWORD, DWORD, LPSECURITY_ATTRIBUTES, DWORD, DWORD, HANDLE);
    
    /** @brief 原 RegCreateKeyW 函数指针 */
    LSTATUS (WINAPI *original_reg_create_key_w_)(HKEY, LPCWSTR, PHKEY);
    
    /** @brief 原 RegSetValueExW 函数指针 */
    LSTATUS (WINAPI *original_reg_set_value_ex_w_)(HKEY, LPCWSTR, DWORD, const BYTE*, DWORD);
    
    /** @brief 原 CreateProcessW 函数指针 */
    BOOL (WINAPI *original_create_process_w_)(
        LPCWSTR, LPWSTR, LPSECURITY_ATTRIBUTES, LPSECURITY_ATTRIBUTES,
        BOOL, DWORD, LPVOID, LPCWSTR, LPSTARTUPINFOW, LPPROCESS_INFORMATION);
};

} // namespace Hook
} // namespace Engine
} // namespace ThinApp

// ============================================================================
// C 接口函数（使用 extern "C" 包裹）
// ============================================================================

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief 初始化 Hook 引擎
 * 
 * 初始化 MinHook 库，准备 Hook 环境。
 * 
 * @return TRUE: 初始化成功; FALSE: 初始化失败
 */
BOOL HookEngine_Initialize(void);

/**
 * @brief 安装所有预定义的 Hook
 * 
 * 安装文件系统、注册表和进程创建相关的 API Hook。
 * 
 * @return TRUE: 安装成功; FALSE: 安装失败
 */
BOOL HookEngine_InstallHooks(void);

/**
 * @brief 卸载所有 Hook
 * 
 * 卸载所有已安装的 Hook，恢复原始 API 函数。
 * 
 * @return TRUE: 卸载成功; FALSE: 卸载失败
 */
BOOL HookEngine_UninstallHooks(void);

/**
 * @brief 清理 Hook 引擎
 * 
 * 卸载所有 Hook 并清理 MinHook 库。
 * 
 * @return TRUE: 清理成功; FALSE: 清理失败
 */
BOOL HookEngine_Shutdown(void);

/**
 * @brief 检查 Hook 引擎是否已初始化
 * 
 * @return TRUE: 已初始化; FALSE: 未初始化
 */
BOOL HookEngine_IsInitialized(void);

// ============================================================================
// 捕获模式 API（用于应用捕获工具）
// ============================================================================

/**
 * @brief 启用捕获模式
 * 
 * 在 Hook 回调中记录所有文件/注册表操作，用于应用捕获工具。
 * 
 * @return TRUE: 成功; FALSE: 失败
 * 
 * @note 调用此函数后，所有 Hook 回调会记录操作到全局列表。
 *       记录的操作可以通过 HookEngine_GetCaptureRecords() 获取。
 */
BOOL HookEngine_EnableCaptureMode(void);

/**
 * @brief 禁用捕获模式
 * 
 * 停止记录文件/注册表操作。
 * 
 * @return TRUE: 成功; FALSE: 失败
 * 
 * @note 调用此函数后，Hook 回调不再记录操作。
 */
BOOL HookEngine_DisableCaptureMode(void);

/**
 * @brief 检查捕获模式是否已启用
 * 
 * @return TRUE: 已启用; FALSE: 未启用
 */
BOOL HookEngine_IsCaptureModeEnabled(void);

/**
 * @brief 清除捕获记录
 * 
 * 清除所有已记录的捕获操作。
 * 
 * @return TRUE: 成功; FALSE: 失败
 * 
 * @note 在启用捕获模式前，建议先清除之前的记录。
 */
BOOL HookEngine_ClearCaptureRecords(void);

/**
 * @brief 获取捕获记录数量
 * 
 * @return 捕获记录数量
 */
size_t HookEngine_GetCaptureRecordCount(void);

/**
 * @brief 获取捕获记录
 * 
 * @param[out] records 捕获记录数组（调用者分配内存）
 * @param[in] maxRecords 最大记录数量
 * @return 实际获取的记录数量
 * 
 * @note 调用此函数前，先调用 HookEngine_GetCaptureRecordCount() 获取需要的数组大小。
 */
size_t HookEngine_GetCaptureRecords(CAPTURE_RECORD* records, size_t maxRecords);

#ifdef __cplusplus
}
#endif

#endif  // HOOK_ENGINE_H

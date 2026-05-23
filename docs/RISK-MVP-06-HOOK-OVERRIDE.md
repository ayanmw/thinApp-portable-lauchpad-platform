# 技术风险详述 - R-MVP-06（Hook 覆盖检测失效）

## 风险描述

Hook 引擎安装的 Hook 可能被其他安全软件或应用程序覆盖，导致：
- Hook 失效（无法重定向文件/注册表操作）
- 应用便携化失败（数据写入真实系统）
- 用户数据泄露（配置保存到真实注册表）

## 影响范围

- **影响用户**：使用安全软件（如 360、火绒、卡巴斯基）的用户
- **影响功能**：Hook 引擎（所有 Hook 点）
- **影响指标**：Hook 覆盖率、应用便携化成功率

## 概率评估

- **技术难度**：高（需要对抗其他 Hook 的覆盖）
- **类似项目参考**：Sandboxie Plus 需要定期检测 Hook 是否被覆盖
- **概率**：**高**（60-80% 概率在 MVP 阶段遇到 Hook 覆盖问题）

## 缓释措施（按优先级排序）

### P0（必须实现，MVP Phase 1 完成）

**措施 1：实现定时检测（每 5 秒）**
- 创建一个后台线程，每 5 秒检查所有 Hook 点
- 检测方法：比较函数前 16 字节是否为预期的 Jump 指令
- 预期覆盖率：95%

**措施 2：实现自动重装**
- 检测到 Hook 被覆盖后，立即重新安装 Hook
- 重装失败超过阈值（3 次）→ 通知用户
- 预期恢复率：90%

### P1（建议实现，MVP Phase 2 完成）

**措施 3：实现异常捕获**
- 在 Hook 函数中添加异常处理（SEH）
- 若 Hook 函数执行失败，立即重新安装 Hook
- 预期捕获率：80%

**措施 4：实现 Hook 链管理**
- 不覆盖其他 Hook，而是插入到 Hook 链中
- 使用 `CallNextHookEx` 调用下一个 Hook
- 预期兼容性：95%

### P2（可选实现，MVP Phase 3 完成）

**措施 5：实现内核 Hook（V2 阶段）**
- 使用 Minifilter 驱动拦截系统调用
- 内核态 Hook 更难被覆盖
- 预期覆盖率：99.9%

## 应急预案（若缓释措施失效）

1. **降级方案**：提供"兼容模式"（禁用与其他 Hook 冲突的功能）
2. **回滚方案**：若 Hook 覆盖问题无法解决，提示用户关闭其他安全软件
3. **用户沟通**：在文档中明确说明兼容的安全软件列表

## 负责人与截止日期

- **负责人**：Dev Lead（高级 C++ 工程师）
- **P0 措施截止日期**：2026-07-01（MVP Phase 1 结束前）
- **P1 措施截止日期**：2026-07-22（MVP Phase 2 结束前）
- **P2 措施截止日期**：V2 阶段

## 监控指标

- Hook 覆盖率（目标：> 95%）
- Hook 重装次数（目标：< 5 次/小时）
- 用户投诉的 Hook 失效问题（目标：< 3 个/月）

若任意指标连续 3 次测试不达标，触发应急预案。

## 测试计划

| 测试场景 | 测试方法 | 验收标准 |
|----------|----------|----------|
| 无其他 Hook | 安装应用，检查 Hook 覆盖率 | 覆盖率 = 100% |
| 有 360 安全卫士 | 安装 360，检查 Hook 覆盖率 | 覆盖率 ≥ 95% |
| 有火绒安全软件 | 安装火绒，检查 Hook 覆盖率 | 覆盖率 ≥ 95% |
| 有卡巴斯基 | 安装卡巴斯基，检查 Hook 覆盖率 | 覆盖率 ≥ 90% |
| Hook 被覆盖后自动重装 | 手动覆盖 Hook，检查是否自动重装 | 重装成功，应用正常便携化 |

## Hook 覆盖检测实现示例

```cpp
// 检测 Hook 是否被覆盖
bool IsHookOverridden(LPCWSTR functionName, LPCWSTR moduleName, BYTE* expectedBytes) {
    HMODULE hModule = GetModuleHandleW(moduleName);
    if (!hModule) return true; // 模块未加载，视为被覆盖
    
    FARPROC functionAddress = GetProcAddress(hModule, functionName);
    if (!functionAddress) return true; // 函数未找到，视为被覆盖
    
    // 读取函数前 16 字节
    BYTE actualBytes[16];
    SIZE_T bytesRead;
    if (!ReadProcessMemory(GetCurrentProcess(), functionAddress, actualBytes, 16, &bytesRead)) {
        return true; // 读取失败，视为被覆盖
    }
    
    // 比较前 16 字节
    return memcmp(actualBytes, expectedBytes, 16) != 0;
}

// 定时检测线程
DWORD WINAPI HookMonitorThread(LPVOID lpParam) {
    while (g_monitoring) {
        // 检查所有 Hook 点
        for (auto& hook : g_hooks) {
            if (IsHookOverridden(hook.functionName, hook.moduleName, hook.expectedBytes)) {
                // Hook 被覆盖，重新安装
                ReinstallHook(hook);
            }
        }
        
        // 等待 5 秒
        Sleep(5000);
    }
    return 0;
}
```

## 参考文献

- MinHook 官方文档：https://github.com/TsudaKageyu/minhook
- Microsoft Detours 官方文档：https://github.com/microsoft/Detours
- Windows Hook 机制：https://docs.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-sethook

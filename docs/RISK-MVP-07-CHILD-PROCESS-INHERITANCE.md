# 技术风险详述 - R-MVP-07（子进程继承失败）

## 风险描述

Hook 引擎需要通过 DLL 注入让子进程也加载 Hook DLL，但可能失败：
- `CreateProcess` 创建的子进程未加载 Hook DLL
- 多级子进程（如 Chrome 的多进程架构）未继承 Hook
- 某些应用使用 `CreateProcessInternalW` 创建进程（绕过标准 Hook）

## 影响范围

- **影响用户**：使用多进程应用的用户（如 Chrome、Firefox、VSCode）
- **影响功能**：子进程 Hook 继承
- **影响指标**：子进程 Hook 覆盖率、应用便携化成功率

## 概率评估

- **技术难度**：高（需要 Hook 多个进程创建函数）
- **类似项目参考**：Sandboxie Plus 需要特殊处理 Chrome 的多进程架构
- **概率**：**中**（40-60% 概率在 MVP 阶段子进程继承失败）

## 缓释措施（按优先级排序）

### P0（必须实现，MVP Phase 1 完成）

**措施 1：完善 `CreateProcessInternalW` Hook**
- Hook `kernel32.dll!CreateProcessW` 和 `kernel32.dll!CreateProcessInternalW`
- 在进程创建时注入 Hook DLL（使用远程线程注入）
- 预期覆盖率：90%

**措施 2：支持 5 层子进程继承**
- 递归注入：父进程 → 子进程 → 孙进程 → ...
- 每层都注入 Hook DLL
- 预期覆盖率：95%

### P1（建议实现，MVP Phase 2 完成）

**措施 3：多种注入方式**
- 远程线程注入（标准方式）
- `AppInit_DLLs` 注册表注入（备用方式）
- `SetWindowsHookEx` 注入（备用方式）
- 预期覆盖率：98%

**措施 4：注入失败重试**
- 注入失败后重试 3 次
- 记录失败应用，建立兼容性规则库
- 预期覆盖率：99%

### P2（可选实现，MVP Phase 3 完成）

**措施 5：内核态进程创建拦截（V2 阶段）**
- 使用 PsSetCreateProcessNotifyRoutine 回调
- 在进程创建时强制注入 Hook DLL
- 预期覆盖率：99.9%

## 应急预案（若缓释措施失效）

1. **降级方案**：提供"单进程模式"（仅 Hook 主进程）
2. **回滚方案**：若子进程继承问题无法解决，提示用户"部分功能可能无法便携化"
3. **用户沟通**：在文档中明确说明多进程应用的兼容性限制

## 负责人与截止日期

- **负责人**：Dev Lead（高级 C++ 工程师）
- **P0 措施截止日期**：2026-07-01（MVP Phase 1 结束前）
- **P1 措施截止日期**：2026-07-22（MVP Phase 2 结束前）
- **P2 措施截止日期**：V2 阶段

## 监控指标

- 子进程 Hook 覆盖率（目标：> 95%）
- 多级子进程继承成功率（目标：100% 5 层继承）
- 用户投诉的子进程继承失败问题（目标：< 3 个/月）

若任意指标连续 3 次测试不达标，触发应急预案。

## 测试计划

| 测试应用 | 测试场景 | 验收标准 |
|----------|----------|----------|
| Notepad++ | 主进程 + 插件进程 | 所有进程都受 Hook 约束 |
| 7-Zip | 主进程 + 右键菜单进程 | 所有进程都受 Hook 约束 |
| Firefox | 主进程 + 多个内容进程 | 所有进程都受 Hook 约束（约 5-10 个进程） |
| Chrome | 主进程 + 多个渲染进程 | 所有进程都受 Hook 约束（约 10-20 个进程） |
| VSCode | 主进程 + 终端进程 + 扩展进程 | 所有进程都受 Hook 约束（约 5-10 个进程） |

## 子进程注入实现示例

```cpp
// Hook CreateProcessInternalW
BOOL WINAPI Hook_CreateProcessInternalW(
    LPCWSTR lpApplicationName,
    LPWSTR lpCommandLine,
    LPSECURITY_ATTRIBUTES lpProcessAttributes,
    LPSECURITY_ATTRIBUTES lpThreadAttributes,
    BOOL bInheritHandles,
    DWORD dwCreationFlags,
    LPVOID lpEnvironment,
    LPCWSTR lpCurrentDirectory,
    LPSTARTUPINFOW lpStartupInfo,
    LPPROCESS_INFORMATION lpProcessInformation,
    DWORD dwFlags
) {
    // 调用原始函数创建进程
    BOOL result = Orig_CreateProcessInternalW(
        lpApplicationName,
        lpCommandLine,
        lpProcessAttributes,
        lpThreadAttributes,
        bInheritHandles,
        dwCreationFlags,
        lpEnvironment,
        lpCurrentDirectory,
        lpStartupInfo,
        lpProcessInformation,
        dwFlags
    );
    
    if (result && lpProcessInformation) {
        // 注入 Hook DLL 到子进程
        InjectDll(lpProcessInformation->hProcess, L"HookEngine.dll");
    }
    
    return result;
}

// 注入 DLL 到目标进程
bool InjectDll(HANDLE hProcess, LPCWSTR dllPath) {
    // 1. 在目标进程中分配内存
    SIZE_T dllPathSize = (wcslen(dllPath) + 1) * sizeof(WCHAR);
    LPVOID pRemoteMem = VirtualAllocEx(hProcess, NULL, dllPathSize, MEM_COMMIT, PAGE_READWRITE);
    if (!pRemoteMem) return false;
    
    // 2. 写入 DLL 路径到目标进程内存
    SIZE_T bytesWritten;
    WriteProcessMemory(hProcess, pRemoteMem, dllPath, dllPathSize, &bytesWritten);
    
    // 3. 获取 LoadLibraryW 地址
    HMODULE hKernel32 = GetModuleHandleW(L"kernel32.dll");
    FARPROC pLoadLibrary = GetProcAddress(hKernel32, "LoadLibraryW");
    
    // 4. 创建远程线程执行 LoadLibraryW
    HANDLE hRemoteThread = CreateRemoteThread(
        hProcess,
        NULL,
        0,
        (LPTHREAD_START_ROUTINE)pLoadLibrary,
        pRemoteMem,
        0,
        NULL
    );
    
    if (!hRemoteThread) {
        VirtualFreeEx(hProcess, pRemoteMem, 0, MEM_RELEASE);
        return false;
    }
    
    // 5. 等待远程线程结束
    WaitForSingleObject(hRemoteThread, INFINITE);
    
    // 6. 清理
    CloseHandle(hRemoteThread);
    VirtualFreeEx(hProcess, pRemoteMem, 0, MEM_RELEASE);
    
    return true;
}
```

## 参考文献

- Microsoft Docs: CreateProcessInternalW：https://docs.microsoft.com/en-us/windows/win32/api/processthreadsapi/nf-processthreadsapi-createprocessw
- DLL 注入技术：https://en.wikipedia.org/wiki/DLL_injection
- Chrome 多进程架构：https://www.chromium.org/developers/design-documents/multi-process-architecture/

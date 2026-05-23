# 测试环境检查清单

**项目**: AI ThinApp Portable Launchpad Platform  
**阶段**: POC Week 4  
**日期**: 2026-05-23  
**负责人**: Test Environment Prep Engineer  

---

## 1. 本机测试环境（Win10 22H2）

### 1.1 开发工具

- [ ] **Visual Studio 2022** 已安装（C++ 桌面开发 workload）
  - 版本: _______________
  - 确认方法: 打开 Visual Studio Installer，检查 "使用 C++ 的桌面开发"
  - 备注: _______________

- [ ] **CMake 3.20+** 已安装
  - 版本: _______________
  - 确认方法: 命令行执行 `cmake --version`
  - 备注: _______________

- [ ] **Git** 已安装
  - 版本: _______________
  - 确认方法: 命令行执行 `git --version`
  - 备注: _______________

### 1.2 第三方库

- [ ] **MinHook 1.3.3** 已下载并解压到 `third_party\minhook\`
  - 路径: `third_party\minhook\` (存在)
  - 版本: _______________
  - 确认方法: 检查 `third_party\minhook\include\MinHook.h` 是否存在
  - 下载地址: <https://github.com/TsudaKageyu/minhook/releases>
  - 备注: _______________

### 1.3 Hook DLL 编译

- [ ] **Hook DLL 已编译** (`build\Release\hook_engine.dll`)
  - 路径: `build\Release\hook_engine.dll` (存在)
  - 架构: x64 / x86 (请圈选)
  - 确认方法: 检查文件是否存在，使用 Dependency Walker 检查导出函数
  - 编译命令: 
    ```powershell
    mkdir build
    cd build
    cmake ..
    cmake --build . --config Release
    ```
  - 备注: _______________

### 1.4 基准应用

- [ ] **Notepad++** 已安装
  - 版本: _______________
  - 安装路径: _______________
  - 确认方法: 启动 Notepad++，检查 "?"
  - 备注: _______________

- [ ] **7-Zip** 已安装
  - 版本: _______________
  - 安装路径: _______________
  - 确认方法: 启动 7-Zip File Manager，检查 Help -> About
  - 备注: _______________

- [ ] **Firefox Portable** 已安装
  - 版本: _______________
  - 安装路径: _______________
  - 确认方法: 启动 Firefox Portable，检查 About Firefox
  - 下载地址: <https://portableapps.com/apps/internet/firefox_portable>
  - 备注: _______________

### 1.5 应用目录结构

- [ ] 应用目录已准备好 (`D:\MyApps\`)
  - 目录结构:
    ```
    D:\MyApps\
    ├── Notepad++\
    │   └── notepad++.exe
    ├── 7-Zip\
    │   └── 7zFM.exe
    └── FirefoxPortable\
        └── FirefoxPortable.exe
    ```
  - 确认方法: 检查上述目录和文件是否存在
  - 备注: _______________

### 1.6 调试工具

- [ ] **Process Monitor** 已安装（监控文件/注册表操作）
  - 版本: _______________
  - 安装路径: _______________
  - 下载地址: <https://learn.microsoft.com/en-us/sysinternals/downloads/procmon>
  - 确认方法: 启动 Process Monitor，能够捕获文件和注册表事件
  - 备注: _______________

- [ ] **x64dbg / x32dbg** 已安装（调试 Hook 引擎）
  - 版本: _______________
  - 安装路径: _______________
  - 下载地址: <https://x64dbg.com/>
  - 确认方法: 启动 x64dbg，能够附加到进程
  - 备注: _______________

- [ ] **Process Explorer** 已安装（查看进程模块）
  - 版本: _______________
  - 安装路径: _______________
  - 下载地址: <https://learn.microsoft.com/en-us/sysinternals/downloads/process-explorer>
  - 确认方法: 启动 Process Explorer，查看进程的 DLL 模块
  - 备注: _______________

### 1.7 测试脚本

- [ ] **下载脚本** 可用 (`tools\ci\download-fixtures.ps1`)
  - 路径: `tools\ci\download-fixtures.ps1` (存在)
  - 确认方法: 执行脚本，能够下载 Notepad++ 和 7-Zip 安装包
  - 备注: _______________

- [ ] **测试执行脚本** 可用 (`tools\ci\run-benchmark-tests.ps1`)
  - 路径: `tools\ci\run-benchmark-tests.ps1` (存在)
  - 确认方法: 执行脚本，能够启动应用并检测 Hook 注入
  - 备注: _______________

---

## 2. 虚拟机测试环境（Win10 22H2）

### 2.1 虚拟机软件

- [ ] **VMware Workstation** 已安装
  - 版本: _______________
  - 确认方法: 启动 VMware Workstation，检查 Help -> About
  - 备注: _______________

- [ ] **VirtualBox** 已安装（可选，作为 VMware 的替代方案）
  - 版本: _______________
  - 确认方法: 启动 VirtualBox，检查 Help -> About VirtualBox
  - 备注: _______________

### 2.2 虚拟机创建

- [ ] **虚拟机已创建** (VMware Workstation / VirtualBox)
  - 虚拟机名称: _______________
  - 虚拟机路径: _______________
  - 确认方法: 在 VMware/VirtualBox 中能够看到虚拟机
  - 备注: _______________

- [ ] **Windows 10 22H2** 已安装
  - 版本: Windows 10 22H2 (OS Build 19045.xxx)
  - 确认方法: 右键 "此电脑" -> 属性，查看 Windows 规格
  - 备注: _______________

- [ ] **VMware Tools / VirtualBox Guest Additions** 已安装
  - 版本: _______________
  - 确认方法: 检查 "程序和功能" 中是否有 VMware Tools / VirtualBox Guest Additions
  - 备注: _______________

### 2.3 杀毒软件

- [ ] **Windows Defender** 已启用（默认启用）
  - 版本: _______________
  - 确认方法: 打开 Windows Security，检查 Virus & threat protection
  - 备注: _______________

- [ ] **360 安全卫士** 已安装（可选）
  - 版本: _______________
  - 确认方法: 启动 360 安全卫士
  - 下载地址: <https://www.360.cn/>
  - 备注: _______________

- [ ] **腾讯电脑管家** 已安装（可选）
  - 版本: _______________
  - 确认方法: 启动腾讯电脑管家
  - 下载地址: <https://guanjia.qq.com/>
  - 备注: _______________

- [ ] **火绒安全** 已安装（可选）
  - 版本: _______________
  - 确认方法: 启动火绒安全
  - 下载地址: <https://www.huorong.cn/>
  - 备注: _______________

### 2.4 测试文件

- [ ] **测试脚本已复制到虚拟机**
  - 文件列表:
    - [ ] `tools\ci\run-benchmark-tests.ps1`
    - [ ] `tools\ci\download-fixtures.ps1`
  - 确认方法: 在虚拟机中检查上述文件是否存在
  - 备注: _______________

- [ ] **应用目录已复制到虚拟机** (`C:\PortableApps\`)
  - 目录结构:
    ```
    C:\PortableApps\
    ├── Notepad++\
    │   └── notepad++.exe
    ├── 7-Zip\
    │   └── 7zFM.exe
    └── FirefoxPortable\
        └── FirefoxPortable.exe
    ```
  - 确认方法: 在虚拟机中检查上述目录和文件是否存在
  - 备注: _______________

- [ ] **Hook DLL 已复制到虚拟机**
  - 路径: `C:\PortableApps\hook_engine.dll`
  - 确认方法: 在虚拟机中检查文件是否存在
  - 备注: _______________

### 2.5 虚拟机快照

- [ ] **虚拟机快照已创建** (可选，但强烈建议)
  - 快照名称: "Clean Install - Before POC Testing"
  - 创建时间: _______________
  - 确认方法: 在 VMware/VirtualBox 中检查快照管理器
  - 备注: 用于测试失败后快速恢复环境

---

## 3. U 盘测试环境

### 3.1 U 盘准备

- [ ] **U 盘已格式化** (NTFS / exFAT)
  - U 盘型号: _______________
  - 容量: _______________
  - 文件系统: NTFS / exFAT (请圈选)
  - 确认方法: 右键 U 盘 -> 属性，查看文件系统
  - 备注: exFAT 兼容性更好（支持 macOS 和 Linux），但 NTFS 性能更好

- [ ] **应用目录已复制到 U 盘**
  - 目录结构:
    ```
    E:\PortableApps\  (E: 为 U 盘盘符)
    ├── Notepad++\
    │   └── notepad++.exe
    ├── 7-Zip\
    │   └── 7zFM.exe
    └── FirefoxPortable\
        └── FirefoxPortable.exe
    ```
  - 确认方法: 在文件资源管理器中检查 U 盘中的目录和文件
  - 备注: _______________

- [ ] **Hook DLL 已复制到 U 盘**
  - 路径: `E:\PortableApps\hook_engine.dll` (E: 为 U 盘盘符)
  - 确认方法: 在文件资源管理器中检查文件是否存在
  - 备注: _______________

### 3.2 便携性测试

- [ ] **在多台电脑上测试过** (验证便携性)
  - 测试电脑列表:
    - [ ] 电脑 1: _______________ (操作系统: _______________)
    - [ ] 电脑 2: _______________ (操作系统: _______________)
    - [ ] 电脑 3: _______________ (操作系统: _______________)
  - 确认方法: 在每台电脑上启动应用，验证功能正常
  - 测试结果:
    - 电脑 1: ✅ 通过 / ❌ 失败
    - 电脑 2: ✅ 通过 / ❌ 失败
    - 电脑 3: ✅ 通过 / ❌ 失败
  - 备注: _______________

---

## 4. 网络环境

### 4.1 网络连接

- [ ] **本机能够访问互联网**
  - 确认方法: 浏览器访问 <https://www.google.com/>
  - 备注: _______________

- [ ] **虚拟机能够访问互联网**
  - 确认方法: 在虚拟机中浏览器访问 <https://www.google.com/>
  - 网络模式: NAT / Bridged (请圈选)
  - 备注: _______________

### 4.2 防火墙

- [ ] **本机防火墙已配置** (允许测试程序通过)
  - 确认方法: Windows Security -> Firewall & network protection -> Allow an app through firewall
  - 备注: _______________

- [ ] **虚拟机防火墙已配置** (允许测试程序通过)
  - 确认方法: 同本机
  - 备注: _______________

---

## 5. 文档和报告

### 5.1 测试计划

- [ ] **POC 计划** 已阅读 (`docs\POC-PLAN.md`)
  - 确认方法: 打开 `docs\POC-PLAN.md`，了解测试范围和验收标准
  - 备注: _______________

- [ ] **基准应用验证方案** 已阅读 (`docs\BENCHMARK-TEST-PLAN.md`)
  - 确认方法: 打开 `docs\BENCHMARK-TEST-PLAN.md`，了解测试步骤
  - 备注: _______________

- [ ] **便携性验证方案** 已阅读 (`docs\PORTABILITY-TEST-PLAN.md`)
  - 确认方法: 打开 `docs\PORTABILITY-TEST-PLAN.md`，了解测试步骤
  - 备注: _______________

- [ ] **杀软兼容性测试计划** 已阅读 (`docs\ANTIVIRUS-TEST-PLAN.md`)
  - 确认方法: 打开 `docs\ANTIVIRUS-TEST-PLAN.md`，了解测试步骤
  - 备注: _______________

### 5.2 测试报告

- [ ] **测试环境准备报告** 已创建 (`docs\TEST-ENV-PREP-REPORT.md`)
  - 路径: `docs\TEST-ENV-PREP-REPORT.md` (存在)
  - 确认方法: 打开文件，检查内容完整
  - 备注: _______________

- [ ] **阻塞项记录** 已创建 (如果有阻塞项) (`docs\WEEK4-BLOCKERS.md`)
  - 路径: `docs\WEEK4-BLOCKERS.md` (存在)
  - 确认方法: 打开文件，检查阻塞项已记录
  - 备注: _______________

---

## 6. 签名和日期

**检查人**: _______________  
**日期**: _______________  
**审核人**: _______________  
**日期**: _______________  

---

## 附录：快速检查命令

### A. 检查 Visual Studio 2022

```powershell
# 检查 Visual Studio 2022 是否安装
Test-Path "C:\Program Files\Microsoft Visual Studio\2022\Community"
Test-Path "C:\Program Files\Microsoft Visual Studio\2022\Professional"
Test-Path "C:\Program Files\Microsoft Visual Studio\2022\Enterprise"
```

### B. 检查 CMake 版本

```powershell
cmake --version
```

### C. 检查 MinHook

```powershell
# 检查 MinHook 是否存在
Test-Path "third_party\minhook\include\MinHook.h"
```

### D. 检查 Hook DLL

```powershell
# 检查 Hook DLL 是否存在
Test-Path "build\Release\hook_engine.dll"
```

### E. 检查基准应用

```powershell
# 检查 Notepad++
Test-Path "D:\MyApps\Notepad++\notepad++.exe"

# 检查 7-Zip
Test-Path "D:\MyApps\7-Zip\7zFM.exe"

# 检查 Firefox Portable
Test-Path "D:\MyApps\FirefoxPortable\FirefoxPortable.exe"
```

### F. 检查调试工具

```powershell
# 检查 Process Monitor
Test-Path "C:\Sysinternals\Procmon.exe"

# 检查 x64dbg
Test-Path "C:\Program Files\x64dbg\x64dbg.exe"

# 检查 Process Explorer
Test-Path "C:\Sysinternals\procexp.exe"
```

---

**文档版本**: 1.0  
**最后更新**: 2026-05-23  
**作者**: Test Environment Prep Engineer

# 应用捕获工具技术调研文档 (AppCapture Research)

## 1. 目标

应用捕获工具（AppCapture）是 Packager 模块的核心组件，用于监控应用安装过程，捕获所有文件/注册表操作，生成 .vapp 包。

### 1.1 核心目标

- **监控应用安装过程**：实时捕获应用安装时的所有文件和注册表操作
- **捕获所有文件操作**：创建/修改/删除
- **捕获所有注册表操作**：创建/修改/删除
- **生成 .vapp 包**：包含捕获的文件和注册表 hive

### 1.2 成功标准

- 能完整捕获典型应用（如 7-Zip、Notepad++）的安装过程
- 捕获的文件和注册表差异准确率 ≥ 95%
- 生成的 .vapp 包能被 Launchpad 正确识别和加载
- POC 阶段可在 1 周内完成原型开发和演示

---

## 2. 技术方案对比

### 2.1 方案总览

| 方案 | 原理 | 优点 | 缺点 | 推荐？ |
|------|------|------|------|--------|
| A. 文件系统过滤驱动（Minifilter） | 内核层拦截文件操作 | 最彻底，无法绕过 | 开发难度大，需要 WHQL 认证 | 否（V2 阶段） |
| B. API Hook（同 Hook 引擎） | 用户态拦截文件 API | 复用 Hook 引擎，开发快 | 应用可能绕过（直接 Syscall） | **是（POC）** |
| C. 系统还原点（System Restore） | 创建还原点 → 安装 → 对比还原点 | 无需 Hook，兼容性好 | 只能捕获文件，无法捕获注册表 | 否 |
| D. 虚拟机快照对比 | 虚拟机快照 → 安装 → 对比快照 | 最彻底，100% 捕获 | 需要虚拟机，用户操作复杂 | 否（用户体验差） |
| E. 文件系统迷你过滤器（Minifilter 用户态 API） | 使用 FilterConnectCommunicationPort 通信 | 内核+用户态结合 | 开发难度中 | 备选 |

### 2.2 详细方案分析

#### 方案 A：文件系统过滤驱动（Minifilter）

**原理**：
- 在内核层挂载文件系统过滤驱动（Minifilter）
- 拦截所有文件系统 I/O 请求（IRP_MJ_CREATE, IRP_MJ_WRITE, IRP_MJ_READ 等）
- 在 Pre/Post 回调中记录文件操作

**优点**：
- **最彻底**：所有文件操作都无法绕过（包括直接 Syscall）
- **性能高**：内核层处理，无需用户态-内核态切换
- **兼容性好**：不依赖具体 API，对所有文件系统操作生效

**缺点**：
- **开发难度大**：需要熟悉 Windows 内核驱动开发（WDM/WDF）
- **需要 WHQL 认证**：否则 Windows 会显示"未签名的驱动程序"警告
- **调试困难**：内核调试需要双机调试（WinDbg + 虚拟机）
- **稳定性风险**：驱动崩溃会导致 BSOD（蓝屏）

**结论**：**不推荐用于 POC 阶段**，但可作为 V2 阶段的技术方案。

---

#### 方案 B：API Hook（同 Hook 引擎）★ 推荐

**原理**：
- 在用户态 Hook 文件系统相关 API（NtCreateFile, NtWriteFile, NtReadFile 等）
- 复用已有的 Hook 引擎（MinHook 框架）
- 在 Hook 回调中记录文件操作

**优点**：
- **开发快**：复用 Hook 引擎代码，无需重新开发
- **兼容性好**：与现有架构无缝集成
- **调试方便**：用户态调试，可使用 Visual Studio 调试器
- **风险低**：不会导致系统崩溃（最多应用崩溃）

**缺点**：
- **可能被绕过**：应用若直接调用 Syscall（绕过 ntdll.dll），则无法捕获
- **Hook 覆盖问题**：其他注入工具可能覆盖我们的 Hook
- **性能损耗**：每次文件操作都需要经过 Hook 函数（但影响较小）

**结论**：**推荐用于 POC 阶段**。虽然可能被绕过，但在 POC 阶段可接受（在文档中说明限制）。

---

#### 方案 C：系统还原点（System Restore）

**原理**：
- 在安装前创建系统还原点
- 安装完成后，对比还原点快照，找出变化的文件
- 使用 `srutil.dll` 或 `WMI` 接口操作还原点

**优点**：
- **无需 Hook**：不依赖 API Hook，兼容性好
- **系统原生支持**：Windows 自带功能，无需额外驱动或框架

**缺点**：
- **只能捕获文件**：无法捕获注册表操作（系统还原点不包含注册表）
- **性能差**：创建/对比还原点非常慢（可能需要几分钟）
- **需要管理员权限**：创建还原点需要管理员权限
- **不准确**：还原点可能包含其他应用的修改（不够精确）

**结论**：**不推荐**，因为无法捕获注册表操作，而注册表是应用安装的重要组成部分。

---

#### 方案 D：虚拟机快照对比

**原理**：
- 在虚拟机中创建快照（安装前）
- 在虚拟机中安装应用
- 对比快照，找出变化的文件和注册表项
- 使用虚拟机提供的快照对比 API（如 VMware VIX，VirtualBox SDK）

**优点**：
- **最彻底**：100% 捕获（虚拟机监控所有 I/O 操作）
- **隔离性好**：不会影响真实系统
- **兼容性好**：不依赖具体 API 或驱动

**缺点**：
- **需要虚拟机**：用户需要安装 VMware/VirtualBox（操作复杂）
- **性能差**：虚拟机性能远低于真实系统
- **用户体验差**：普通用户不会使用虚拟机
- **自动化难**：快照对比需要手动操作或复杂的脚本

**结论**：**不推荐用于面向用户的产品**，但可作为内部测试工具（验证捕获结果的准确性）。

---

#### 方案 E：文件系统迷你过滤器（Minifilter 用户态 API）

**原理**：
- 使用 Minifilter 驱动（内核层），但通过 `FilterConnectCommunicationPort` 与用户态通信
- 在内核层拦截文件操作，但将记录发送到用户态处理
- 结合了内核层的彻底性和用户态的灵活性

**优点**：
- **较彻底**：大部分文件操作无法绕过
- **灵活性高**：用户态处理捕获逻辑，易于调试和修改
- **性能较好**：内核层只负责拦截和转发，用户态负责记录

**缺点**：
- **开发难度中**：需要熟悉 Minifilter 驱动开发和用户态-内核态通信
- **需要驱动签名**：虽然不需要 WHQL 认证，但仍需要代码签名证书
- **稳定性风险**：驱动崩溃仍会导致 BSOD（但概率低于纯内核方案）

**结论**：**可作为备选方案**（如果 API Hook 无法满足需求）。

---

### 2.3 推荐方案：方案 B（API Hook）

**推荐理由**：

1. **POC 阶段快速验证**：复用 Hook 引擎代码，1 周内可完成原型开发
2. **与现有架构无缝集成**：Hook 引擎已支持文件系统/注册表 API 拦截
3. **开发风险低**：用户态开发，调试方便，不会导致系统崩溃
4. **可接受的限制**：POC 阶段允许"应用绕过 Hook"的情况（在文档中说明限制）

**限制说明（POC 阶段文档记录）**：

- 若应用直接调用 Syscall（绕过 ntdll.dll），则无法捕获
- 典型应用（如 7-Zip、Notepad++）通常使用标准 API，不会直接调用 Syscall
- 若 POC 验证通过，V2 阶段可切换到 Minifilter 驱动方案（彻底解决绕过问题）

---

## 3. 捕获流程设计

### 3.1 总体流程

```
1. 用户点击"开始捕获"
   ↓
2. 创建前快照（文件系统 + 注册表）
   ↓
3. 启动安装程序（用户选择 exe/msi）
   ↓
4. Hook 安装程序的所有文件/注册表操作
   ↓
5. 安装完成后，用户点击"停止捕获"
   ↓
6. 创建后快照（文件系统 + 注册表）
   ↓
7. 对比前后快照，生成差异列表
   ↓
8. 将差异文件打包到 {AppDir}\VFS\
   ↓
9. 将差异注册表导出到 {AppDir}\VFS\registry.hive
   ↓
10. 生成 .vapp 包（ZIP 格式，包含 VFS\ 目录 + 元数据.json）
```

### 3.2 详细流程

#### 步骤 1：开始捕获

**用户操作**：点击"开始捕获"按钮

**系统处理**：
1. 检查是否已有捕获在进行中（若有，提示用户先停止）
2. 创建临时目录 `{TempDir}\AppCapture_{Timestamp}`
3. 记录开始时间 `StartTime`

**输出**：状态栏显示"捕获已启动，请安装应用..."

---

#### 步骤 2：创建前快照

**目标**：记录安装前的文件和注册表状态，用于后续对比

**文件系统快照**：
1. 遍历关键目录：
   - `C:\Program Files\`
   - `C:\Program Files (x86)\`
   - `C:\Windows\`
   - `C:\Users\*\AppData\Roaming\`
   - `C:\Users\*\AppData\Local\`
   - `C:\Users\*\AppData\LocalLow\`
   - `C:\ProgramData\`
   - `C:\Temp\`
2. 记录每个文件的：
   - 完整路径
   - 文件大小
   - 创建时间
   - 修改时间
   - 文件哈希（SHA-256，可选，性能损耗大）
3. 保存为 `{TempDir}\BeforeSnapshot\files.json`

**注册表快照**：
1. 导出关键注册表键：
   - `HKLM\Software\`
   - `HKCU\Software\`
   - `HKCR\`（文件关联）
   - `HKU\`（用户配置）
2. 使用 `RegSaveKey` API 导出为 hive 文件：
   - `{TempDir}\BeforeSnapshot\HKLM_Software.hive`
   - `{TempDir}\BeforeSnapshot\HKCU_Software.hive`
3. 解析 hive 文件，提取所有键和值的列表，保存为 `{TempDir}\BeforeSnapshot\registry.json`

**性能优化**：
- 文件系统快照：只记录文件元数据，不计算哈希（POC 阶段）
- 注册表快照：使用 `RegSaveKey` 导出为二进制 hive，后续对比时再解析（性能优于解析为 JSON）

**输出**：前快照保存在 `{TempDir}\BeforeSnapshot\`

---

#### 步骤 3：启动安装程序

**用户操作**：选择安装程序（exe/msi），点击"启动"

**系统处理**：
1. 验证文件是否存在、是否是有效 PE 文件
2. 若是一 .msi 文件，调用 `msiexec.exe /i <path>`
3. 若是 exe 文件，直接调用 `CreateProcess`
4. 注入 Hook DLL 到安装程序进程（参考 `process_inherit.cpp`）
5. 开始记录文件/注册表操作

**Hook 回调处理**：
- 每次文件操作（创建/写入/删除）触发 Hook 回调
- 每次注册表操作（创建键/设置值/删除键）触发 Hook 回调
- 将操作记录到 `{TempDir}\Operations\` 目录（按时间戳命名）

**输出**：安装程序启动，Hook 开始记录操作

---

#### 步骤 4：Hook 安装程序的所有文件/注册表操作

**文件操作记录**：

| 操作类型 | API | 记录内容 |
|---------|-----|---------|
| 创建文件 | `NtCreateFile` | 路径、DesiredAccess、CreateDisposition、时间戳 |
| 写入文件 | `NtWriteFile` | 路径、写入偏移量、写入字节数、时间戳 |
| 读取文件 | `NtReadFile` | 路径、读取偏移量、读取字节数、时间戳 |
| 删除文件 | `NtCreateFile`（DeleteOnClose） | 路径、时间戳 |
| 修改文件属性 | `NtSetInformationFile` | 路径、FileInformationClass、时间戳 |

**注册表操作记录**：

| 操作类型 | API | 记录内容 |
|---------|-----|---------|
| 创建键 | `NtCreateKey` | 路径、DesiredAccess、CreateOptions、时间戳 |
| 设置值 | `NtSetValueKey` | 路径、值名称、值类型、值数据、时间戳 |
| 删除键 | `NtDeleteKey` | 路径、时间戳 |
| 删除值 | `NtDeleteValueKey` | 路径、值名称、时间戳 |

**记录格式**（JSON）：

```json
{
  "operations": [
    {
      "timestamp": "2026-05-23T17:00:00.000Z",
      "type": "FileCreated",
      "path": "C:\\Program Files\\7-Zip\\7z.exe",
      "process": "setup.exe",
      "pid": 12345
    },
    {
      "timestamp": "2026-05-23T17:00:01.000Z",
      "type": "RegValueSet",
      "path": "HKLM\\Software\\7-Zip\\Path",
      "valueName": "Path",
      "valueType": "REG_SZ",
      "valueData": "C:\\Program Files\\7-Zip\\",
      "process": "setup.exe",
      "pid": 12345
    }
  ]
}
```

**输出**：所有操作记录在 `{TempDir}\Operations\`

---

#### 步骤 5：停止捕获

**用户操作**：安装完成后，点击"停止捕获"按钮

**系统处理**：
1. 停止 Hook 记录（不再记录新的操作）
2. 提示用户是否要"创建后快照"
3. 若用户确认，执行步骤 6

**输出**：状态栏显示"捕获已停止，正在创建后快照..."

---

#### 步骤 6：创建后快照

**与步骤 2 相同**，但保存为 `{TempDir}\AfterSnapshot\`

**输出**：后快照保存在 `{TempDir}\AfterSnapshot\`

---

#### 步骤 7：对比前后快照，生成差异列表

**文件系统差异对比**：

| 差异类型 | 检测方式 | 处理 |
|---------|---------|------|
| 新增文件 | 后快照有，前快照无 | 复制到 `{AppDir}\VFS\` |
| 修改文件 | 后快照有，前快照有，但大小/时间/哈希不同 | 复制到 `{AppDir}\VFS\`（覆盖） |
| 删除文件 | 前快照有，后快照无 | 记录到差异列表（但 POC 阶段不处理，因为安装程序通常不会删除文件） |

**注册表差异对比**：

| 差异类型 | 检测方式 | 处理 |
|---------|---------|------|
| 新增键 | 后快照有，前快照无 | 添加到 `{AppDir}\VFS\registry.hive` |
| 修改值 | 后快照有，前快照有，但值数据不同 | 更新到 `{AppDir}\VFS\registry.hive` |
| 删除键 | 前快照有，后快照无 | 记录到差异列表（但 POC 阶段不处理） |

**差异列表格式**（JSON）：

```json
{
  "addedFiles": [
    "C:\\Program Files\\7-Zip\\7z.exe",
    "C:\\Program Files\\7-Zip\\7z.dll"
  ],
  "modifiedFiles": [],
  "deletedFiles": [],
  "addedRegKeys": [
    "HKLM\\Software\\7-Zip"
  ],
  "modifiedRegValues": [
    {
      "path": "HKLM\\Software\\7-Zip\\Path",
      "valueName": "Path",
      "valueType": "REG_SZ",
      "valueData": "C:\\Program Files\\7-Zip\\"
    }
  ],
  "deletedRegKeys": []
}
```

**输出**：差异列表保存在 `{TempDir}\Diff\diff.json`

---

#### 步骤 8：将差异文件打包到 {AppDir}\VFS\

**目标**：将新增/修改的文件复制到虚拟文件系统目录

**路径映射规则**（参考 `ARCHITECTURE.md`）：

| 真实路径 | 虚拟路径 |
|---------|---------|
| `C:\Program Files\7-Zip\7z.exe` | `{AppDir}\VFS\C\Program Files\7-Zip\7z.exe` |
| `C:\Windows\System32\xxx.dll` | `{AppDir}\VFS\C\Windows\System32\xxx.dll` |
| `%APPDATA%\xxx` | `{AppDir}\VFS\AppData\Roaming\xxx` |

**实现步骤**：
1. 读取 `diff.json` 中的 `addedFiles` 和 `modifiedFiles`
2. 对每个文件，计算虚拟路径
3. 创建目标目录（若不存在）
4. 复制文件到虚拟路径
5. 记录复制结果（成功/失败）

**输出**：虚拟文件系统目录 `{AppDir}\VFS\` 包含捕获的文件

---

#### 步骤 9：将差异注册表导出到 {AppDir}\VFS\registry.hive

**目标**：将新增/修改的注册表键和值导出到虚拟 hive

**实现步骤**：
1. 读取 `diff.json` 中的 `addedRegKeys` 和 `modifiedRegValues`
2. 创建新的 hive 文件 `{AppDir}\VFS\registry.hive`
3. 将差异注册表键和值写入 hive
4. 使用 `RegSaveKey` API 保存 hive

**输出**：虚拟注册表 hive `{AppDir}\VFS\registry.hive`

---

#### 步骤 10：生成 .vapp 包

**目标**：将 VFS 目录和元数据打包为 .vapp 包（ZIP 格式）

**.vapp 包结构**：

```
app.vapp (ZIP 文件)
├── VFS\                          # 虚拟文件系统目录
│   ├── C\
│   │   ├── Program Files\
│   │   │   └── 7-Zip\
│   │   │       ├── 7z.exe
│   │   │       └── 7z.dll
│   │   └── Windows\
│   │       └── System32\
│   │           └── xxx.dll
│   ├── AppData\
│   │   └── Roaming\
│   │       └── 7-Zip\
│   │           └── config.ini
│   └── registry.hive             # 虚拟注册表 hive
├── manifest.json                  # 元数据
└── icon.ico                      # 应用图标（可选）
```

**manifest.json 格式**：

```json
{
  "name": "7-Zip",
  "version": "23.01",
  "publisher": "Igor Pavlov",
  "description": "7-Zip 文件压缩工具",
  "icon": "icon.ico",
  "mainExe": "7z.exe",
  "installDate": "2026-05-23T17:00:00.000Z",
  "captureToolVersion": "1.0.0",
  "dependencies": []
}
```

**实现步骤**：
1. 创建 `{TempDir}\VApp\` 目录
2. 复制 `{AppDir}\VFS\` 到 `{TempDir}\VApp\VFS\`
3. 生成 `manifest.json`
4. 使用 `zip` 命令或 `zlib` 库压缩为 .vapp 文件
5. 移动到最终位置（如 `{OutputDir}\7-Zip.vapp`）

**输出**：.vapp 包文件

---

## 4. 技术难点

### 4.1 安装程序可能重启系统

**问题**：某些应用安装完成后需要重启系统（如驱动程序、系统组件）

**影响**：重启后捕获工具无法继续记录（进程已退出）

**解决方案**（POC 阶段）：
- **不支持重启场景**：在文档中说明限制，建议用户先安装需要的驱动，再捕获应用
- **中断/恢复捕获**（V2 阶段）：在系统重启前保存捕获状态，重启后恢复捕获

**实现思路**（V2 阶段）：
1. 在捕获开始时，注册系统重启回调（`RegisterApplicationRestart`）
2. 系统重启前，保存捕获状态到文件（`{TempDir}\State\state.json`）
3. 系统重启后，自动恢复捕获（读取状态文件，继续记录）

---

### 4.2 安装程序可能有多个子进程

**问题**：某些安装程序会创建多个子进程（如 `msiexec.exe` 会创建多个子进程执行安装逻辑）

**影响**：若只 Hook 父进程，子进程的文件/注册表操作无法捕获

**解决方案**：Hook 继承（参考 `process_inherit.cpp`）

**实现思路**：
1. Hook `CreateProcessInternalW` API
2. 在 Hook 回调中，创建挂起的子进程（`CREATE_SUSPENDED`）
3. 注入 Hook DLL 到子进程（远程线程注入）
4. 恢复子进程执行
5. 子进程的 Hook DLL 在 `DllMain` 中自动安装 Hook

**验证标准**：能捕获 2 层子进程的操作（父进程 → 子进程 → 孙进程）

---

### 4.3 注册表快照对比性能

**问题**：注册表键数量庞大（HKLM\Software 可能有数万个键），对比性能差

**影响**：创建快照和对比快照可能需要几分钟，用户体验差

**解决方案**：
- **优化快照创建**：使用 `RegSaveKey` 导出为二进制 hive，而非逐个枚举键
- **优化对比算法**：使用哈希表（Hash Map）存储注册表键和值，对比时间复杂度 O(n)

**实现思路**：
1. 快照创建：调用 `RegSaveKey` 导出 hive 文件（快，因为是企业级 API）
2. 快照对比：加载前后 hive 文件，使用 Windows API 枚举键和值，构建哈希表
3. 对比哈希表，找出差异

**性能基准**（参考）：
- 创建快照：~5 秒（HKLM\Software + HKCU\Software）
- 对比快照：~10 秒

---

### 4.4 大文件捕获性能

**问题**：某些应用包含大文件（如游戏、IDE），捕获和复制耗时较长

**影响**：用户体验差，可能误认为程序卡死

**解决方案**（POC 阶段）：
- **增量存储**：只存储文件的差异部分（类似 rsync 算法）
- **异步复制**：在后台线程复制文件，主线程显示进度条

**实现思路**（增量存储）：
1. 计算文件的哈希值（SHA-256）
2. 检查全局缓存（`{GlobalCache}\Files\{Hash}`）是否已存在该文件
3. 若存在，创建硬链接（Hard Link）或符号链接（Symbolic Link）
4. 若不存在，复制文件到缓存，再创建硬链接

**优点**：
- 节省磁盘空间（多个应用共享同一个文件）
- 加快捕获速度（若文件已存在，无需复制）

**输出**：大文件捕获性能可接受（< 1 分钟 for 1 GB 文件）

---

### 4.5 Hook 被其他软件覆盖

**问题**：某些安全软件（如 360、火绒）会注入自己的 DLL 到目标进程，可能覆盖我们的 Hook

**影响**：Hook 失效，无法捕获文件/注册表操作

**解决方案**（参考 `hook_engine.cpp` 中的 Hook 覆盖检测）：
- 定期检查 Hook 是否被覆盖（每 5 秒）
- 若被覆盖，自动重装 Hook
- 重装失败超过阈值，通知用户

**实现思路**：
1. 在 Hook 安装完成后，启动一个后台线程
2. 线程每 5 秒检查一次 Hook 状态（调用 `MH_Status`）
3. 若 Hook 被覆盖，调用 `MH_EnableHook` 重新启用
4. 若重装失败超过 3 次，写入日志并通知用户

---

### 4.6 应用直接调用 Syscall（绕过 ntdll.dll）

**问题**：某些应用为了提高性能或反调试，可能直接调用 Syscall（绕过 ntdll.dll 的 API 封装）

**影响**：我们的 Hook 无法捕获这些操作（因为 Hook 的是 ntdll.dll 的导出函数）

**解决方案**（POC 阶段）：
- **记录限制**：在文档中说明，若应用直接调用 Syscall，则无法捕获
- **检测 Syscall**：在 Hook 回调中，检查调用栈是否来自 ntdll.dll（若不是，则可能是直接 Syscall）

**实现思路**（检测 Syscall）：
1. 在 Hook 回调中，调用 `CaptureStackBackTrace` 获取调用栈
2. 遍历调用栈，检查每个返回地址是否属于 ntdll.dll
3. 若不属于，则记录"可能的直接 Syscall"警告

**V2 阶段解决方案**：
- 使用 Minifilter 驱动（内核层拦截，无法绕过）

---

## 5. 原型实现计划

### 5.1 文件结构

```
D:\anmw_work_proj\ai-thinApp-portable-lauchpad-platform\
├── docs\
│   ├── APP-CAPTURE-RESEARCH.md      # 本文档
│   └── APP-CAPTURE-PROTOTYPE-REPORT.md  # 原型报告
├── src\
│   └── packager\
│       ├── app_capture.h             # 应用捕获器头文件
│       └── app_capture.cpp           # 应用捕获器实现
└── tests\
    └── packager\
        └── test_app_capture.cpp      # 测试程序
```

### 5.2 类设计

**参考任务描述中的 `AppCapture` 类**：

```cpp
namespace packager {

// 捕获事件类型
enum class CaptureEvent {
    FileCreated,
    FileModified,
    FileDeleted,
    RegKeyCreated,
    RegValueSet,
    RegKeyDeleted
};

// 捕获事件
struct CaptureRecord {
    CaptureEvent type;       // 事件类型
    std::string path;        // 文件/注册表路径
    std::string data;        // 数据（文件内容/注册表值）
    FILETIME timestamp;      // 时间戳
};

// 捕获快照
struct CaptureSnapshot {
    std::vector<std::string> files;   // 文件列表
    std::vector<std::string> regKeys; // 注册表键列表
};

// 应用捕获器
class AppCapture {
public:
    AppCapture();
    ~AppCapture();

    // 开始捕获（创建前快照）
    bool Start();

    // 停止捕获（创建后快照，对比差异）
    bool Stop();

    // 获取捕获记录
    const std::vector<CaptureRecord>& GetRecords() const;

    // 导出 .vapp 包
    bool ExportVApp(const std::string& outputPath);

    // 回调函数（进度通知）
    std::function<void(int progress, const std::string& status)> OnProgress;

private:
    // 创建快照
    bool CreateSnapshot(CaptureSnapshot& snapshot);

    // 对比快照
    bool CompareSnapshots(const CaptureSnapshot& before,
                          const CaptureSnapshot& after,
                          std::vector<CaptureRecord>& records);

    // 快照数据
    CaptureSnapshot snapshotBefore_;
    CaptureSnapshot snapshotAfter_;
    std::vector<CaptureRecord> records_;

    // 临时目录
    std::string tempDir_;
};

}  // namespace packager
```

### 5.3 测试计划

**测试用例**：

| 测试用例 ID | 测试内容 | 预期结果 |
|------------|---------|---------|
| TC-001 | 测试 Start/Stop | 能正常开始和停止捕获 |
| TC-002 | 测试快照创建（文件系统） | 能正确枚举关键目录的文件 |
| TC-003 | 测试快照创建（注册表） | 能正确导出 HKLM\Software 和 HKCU\Software |
| TC-004 | 测试快照对比（文件新增） | 能检测出新增的文件 |
| TC-005 | 测试快照对比（文件修改） | 能检测出修改的文件 |
| TC-006 | 测试快照对比（文件删除） | 能检测出删除的文件 |
| TC-007 | 测试快照对比（注册表新增） | 能检测出新增的注册表键 |
| TC-008 | 测试快照对比（注册表修改） | 能检测出修改的注册表值 |
| TC-009 | 测试 .vapp 包导出 | 生成的 ZIP 文件可解压，内容正确 |
| TC-010 | 测试 Hook 集成 | Hook 回调能正确记录文件/注册表操作 |

---

## 6. 后续优化方向（V2 阶段）

### 6.1 使用 Minifilter 驱动

- **目标**：解决"应用直接调用 Syscall 绕过 Hook"的问题
- **实现**：开发 Minifilter 驱动，在内核层拦截所有文件操作
- **优点**：无法绕过，捕获最彻底
- **缺点**：开发难度大，需要 WHQL 认证

### 6.2 支持系统重启场景

- **目标**：支持需要重启系统的应用安装（如驱动程序）
- **实现**：在系统重启前保存捕获状态，重启后恢复捕获
- **优点**：扩大应用捕获的适用范围
- **缺点**：实现复杂，需要处理状态序列化和恢复

### 6.3 增量存储优化

- **目标**：优化大文件捕获性能
- **实现**：使用 rsync 算法，只存储文件的差异部分
- **优点**：节省磁盘空间，加快捕获速度
- **缺点**：实现复杂，需要计算文件哈希和差异

### 6.4 注册表快照对比优化

- **目标**：优化注册表快照对比性能
- **实现**：使用哈希表存储注册表键和值，对比时间复杂度 O(n)
- **优点**：加快对比速度，提升用户体验
- **缺点**：需要额外的内存存储哈希表

---

## 7. 修订历史

| 版本 | 日期 | 作者 | 变更说明 |
|------|------|------|----------|
| 0.1 | 2026-05-23 | Windows 系统编程工程师 | 初版，完成技术调研 |

---

## 附录 A：参考资料

### A.1 MinHook 文档

- 官网：https://github.com/TsudaKageyu/minhook
- 文档：https://github.com/TsudaKageyu/minhook/wiki

### A.2 Windows API 参考

- NtCreateFile：https://learn.microsoft.com/en-us/windows-hardware/drivers/ddi/ntifs/nf-ntifs-ntcreatefile
- NtCreateKey：https://learn.microsoft.com/en-us/windows-hardware/drivers/ddi/wdm/nf-wdm-ntcreatekey
- RegSaveKey：https://learn.microsoft.com/en-us/windows/win32/api/winreg/nf-winreg-regsavekeyw

### A.3 文件系统过滤驱动

- Minifilter 入门：https://learn.microsoft.com/en-us/windows-hardware/drivers/ifs/getting-started-with-filter-driver-development
- FilterConnectCommunicationPort：https://learn.microsoft.com/en-us/windows-hardware/drivers/ddi/fltkernel/nf-fltkernel-filterconnectcommunicationport

---

## 附录 B：术语表

| 术语 | 说明 |
|------|------|
| API Hook | 应用程序接口钩子，用于拦截和修改 API 函数的行为 |
| MinHook | 轻量级的 API Hook 框架，支持 x86/x64 |
| Minifilter | Windows 文件系统过滤驱动框架 |
| Syscall | 系统调用，应用程序与操作系统内核交互的接口 |
| Hive | 注册表数据存储文件（二进制格式） |
| CoW | Copy-on-Write，写时复制，一种虚拟化技术 |
| VFS | Virtual File System，虚拟文件系统 |
| VReg | Virtual Registry，虚拟注册表 |
| .vapp | 虚拟应用包，包含应用的所有文件和注册表配置 |


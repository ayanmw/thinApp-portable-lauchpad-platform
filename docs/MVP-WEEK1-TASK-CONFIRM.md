# MVP Week 1 任务理解确认

> 本文档确认对 AI ThinApp Portable Launchpad Platform 项目 MVP Week 1 开发任务的理解。
> 版本：1.0 | 日期：2026-05-23 | 负责人：Windows C++ 底层开发工程师

---

## 1. 任务概览

Week 1 的核心目标是 **实现 VFS 缓存和路径重定向算法优化**，提升 Hook 引擎的文件重定向性能。

### 1.1 开发任务（Day 1-5）

| 天数 | 任务模块 | 具体任务 | 产出文件 |
|------|----------|----------|----------|
| Day 1-3 | VFS 缓存实现 | 实现 LRU + TTL 缓存策略 | `src/engine/vfs/vfs_cache.h`<br>`src/engine/vfs/vfs_cache.cpp` |
| Day 1-3 | Hook 引擎集成 | 在文件重定向逻辑中集成 VfsCache | `src/engine/hook/hook_engine.cpp` (更新) |
| Day 1-3 | 单元测试 | 编写 VFS 缓存测试用例 | `tests/engine/test_vfs_cache.cpp` |
| Day 4-5 | 路径重定向优化 | 实现 Trie 树 + 缓存优化 | `src/engine/vfs/path_redirect.h`<br>`src/engine/vfs/path_redirect.cpp` |
| Day 4-5 | Hook 引擎集成 | 用 PathRedirect 替换原有逻辑 | `src/engine/hook/hook_engine.cpp` (更新) |
| Day 4-5 | 单元测试 | 编写路径重定向测试用例 | `tests/engine/test_path_redirect.cpp` |

---

## 2. 技术方案理解

### 2.1 VFS 缓存设计（LRU + TTL）

#### 2.1.1 数据结构
- **哈希表（unordered_map）**：键为原始路径（宽字符串），值为双向链表节点指针，支持 O(1) 查找
- **双向链表**：按访问时间排序，最近访问的节点在链表头部，最久未访问的在尾部
- **LRU 淘汰**：当缓存满时，淘汰链表尾部节点

#### 2.1.2 缓存策略
- **LRU 容量**：最大缓存 1000 个路径映射
- **TTL**：60 秒（超时自动失效）
- **大小限制**：最大 100MB（防止内存占用过高）

#### 2.1.3 线程安全
- 使用 **SRWLOCK** 实现读写锁（支持多线程并发访问）
- 读操作：`AcquireSRWLockShared()`（共享锁，不阻塞其他读操作）
- 写操作：`AcquireSRWLockExclusive()`（排他锁，阻塞其他读写操作）

#### 2.1.4 缓存失效策略
- **TTL 到期**：缓存项创建/更新超过 60 秒自动失效
- **手动失效**：应用更新时主动清理缓存（Invalidate/Clear）

### 2.2 路径重定向算法优化（Trie 树 + 缓存）

#### 2.2.1 Trie 树数据结构
- **节点结构**：每个节点代表一个路径字符，根节点代表空路径
- **查询复杂度**：O(k)（k 为路径长度），与规则数量无关
- **适用场景**：路径前缀匹配（如 `C:\Program Files\` → `{AppDir}\VFS\C\Program Files\`）

#### 2.2.2 默认重定向规则
- `C:\Windows\*` → `{AppDir}\VFS\C\Windows\*`
- `C:\Program Files\*` → `{AppDir}\VFS\C\Program Files\*`
- `C:\ProgramData\*` → `{AppDir}\VFS\C\ProgramData\*`

#### 2.2.3 缓存优化
- 将 Trie 树查询结果缓存到最近 100 条重定向结果中
- 避免重复 Trie 树查询，提升性能

#### 2.2.4 线程安全
- 使用 **`std::shared_mutex`** 实现读写锁（C++17 标准库）
- 读操作：`std::shared_lock<std::shared_mutex>`
- 写操作：`std::unique_lock<std::shared_mutex>`

---

## 3. 性能目标

### 3.1 VFS 缓存性能目标

| 指标 | 目标值 | 测试方法 |
|------|--------|----------|
| 缓存命中率 | ≥ 80% | 80% 请求访问 20% 热点文件 |
| 重定向延迟（缓存命中） | < 1ms | 测量 10000 次缓存命中场景的平均延迟 |
| 内存占用 | < 50MB | 1000 个缓存项 |
| 线程安全 | 无崩溃/死锁 | 10 线程并发读写缓存 |

### 3.2 路径重定向性能目标

| 指标 | 目标值 | 测试方法 |
|------|--------|----------|
| 路径重定向延迟（P50） | < 0.1ms | 测量 10000 次重定向的 P50 延迟 |
| Trie 树匹配正确率 | = 100% | 输入已知路径，验证输出正确性 |
| 缓存命中率 | ≥ 50% | 50% 请求访问 10 条热点规则 |
| 内存占用 | < 10MB | 1000 条规则 |
| 线程安全 | 无崩溃/死锁 | 10 线程并发重定向 |

---

## 4. 验收标准

### 4.1 功能验收

- ✅ VFS 缓存实现完整（LRU + TTL + 线程安全）
- ✅ 路径重定向优化实现完整（Trie 树 + 缓存）
- ✅ Hook 引擎集成缓存和路径优化
- ✅ 所有测试用例通过（5 + 4 = 9 个测试用例）

### 4.2 性能验收

- ✅ 缓存命中率 ≥ 80%（VFS 缓存）
- ✅ 重定向延迟 < 1ms（VFS 缓存命中时）
- ✅ 路径重定向延迟 < 0.1ms（P50）
- ✅ 缓存命中率 ≥ 50%（路径重定向缓存）

### 4.3 代码质量验收

- ✅ 单元测试覆盖率 ≥ 80%
- ✅ 所有文件使用 UTF-8 编码（无 BOM）
- ✅ 代码注释用中文
- ✅ 无编译器警告（/W4 级别）

---

## 5. 开发计划

### 5.1 Day 1-3：VFS 缓存实现

#### Day 1：设计 VFS 缓存数据结构
- [x] 读取设计文档（已完成）
- [ ] 创建 `src/engine/vfs/vfs_cache.h`
- [ ] 定义 `VfsCache` 类
- [ ] 定义缓存条目结构（`path`、`data`、`timestamp`、`ttl`、`access_count`）
- [ ] 定义 LRU 链表节点结构
- [ ] 定义哈希表（unordered_map）

#### Day 2：实现 VFS 缓存核心逻辑
- [ ] 创建 `src/engine/vfs/vfs_cache.cpp`
- [ ] 实现 `Get()` 方法（查缓存 → 检查 TTL → 更新 LRU + access_count）
- [ ] 实现 `Put()` 方法（插入缓存 → 检查容量 → LRU 淘汰）
- [ ] 实现 `Invalidate()` 方法（失效指定路径缓存）
- [ ] 实现 `Clear()` 方法（清空缓存）
- [ ] 使用 SRWLOCK 实现读写锁

#### Day 3：集成 VFS 缓存到 Hook 引擎
- [ ] 修改 `src/engine/hook/hook_engine.cpp`
- [ ] 在文件重定向逻辑中集成 `VfsCache`
- [ ] 重定向前先查缓存（命中则直接返回）
- [ ] 重定向完成后写入缓存（未命中）

### 5.2 Day 4-5：路径重定向算法优化

#### Day 4：实现 Trie 树 + 缓存
- [ ] 创建 `src/engine/vfs/path_redirect.h`
- [ ] 定义 `PathRedirect` 类
- [ ] 定义 Trie 树节点结构（字符 + 子节点 map + 是否为终点 + 重定向路径）
- [ ] 定义缓存结构（最近 100 条重定向结果）
- [ ] 创建 `src/engine/vfs/path_redirect.cpp`
- [ ] 实现 `AddRule()` 方法（添加重定向规则到 Trie 树）
- [ ] 实现 `Redirect()` 方法（Trie 树匹配 + 缓存查询）
- [ ] 实现 `InitDefaultRules()` 方法（初始化默认规则）
- [ ] 使用 `std::shared_mutex` 实现读写锁

#### Day 5：集成路径重定向到 Hook 引擎 + 测试
- [ ] 修改 `src/engine/hook/hook_engine.cpp`
- [ ] 用 `PathRedirect::Redirect()` 替换原有的字符串替换逻辑
- [ ] 在 DLL 初始化时调用 `InitDefaultRules()`
- [ ] 编写测试程序 `tests/engine/test_vfs_cache.cpp`（5 个测试用例）
- [ ] 编写测试程序 `tests/engine/test_path_redirect.cpp`（4 个测试用例）

---

## 6. 风险与依赖

### 6.1 依赖
- 依赖 `docs/MVP-HOOK-ENGINE-DESIGN.md`（已读取）
- 依赖 MinHook 库（版本 ≥ 1.3.3）
- 依赖 Windows SDK（版本 ≥ 10.0.19041.0）

### 6.2 风险
| 风险描述 | 影响程度 | 缓释措施 |
|----------|----------|----------|
| VFS 缓存锁管理复杂，可能导致死锁 | 高 | 使用 SRWLOCK，读操作不阻塞，写操作阻塞；避免锁嵌套 |
| Trie 树内存占用过高 | 中 | 限制 Trie 树节点数量，定期清理不常用节点 |
| 性能不达标（缓存命中率 < 80%） | 中 | 优化缓存策略，调整 LRU 容量和 TTL |
| 编译器不支持 C++17 | 低 | 使用 C++14 替代（需修改 `std::shared_mutex` 为 SRWLOCK） |

### 6.3 Blocker 记录
若存在技术难点（如高性能并发锁），立即记录到 `docs/MVP-BLOCKERS.md` 并上报 PM。

---

## 7. 下一步行动

1. **立即开始**：创建 `src/engine/vfs/vfs_cache.h` 和 `src/engine/vfs/vfs_cache.cpp`
2. **优先保证核心逻辑正确**：先实现基本功能，再优化性能
3. **每完成一个文件就写入到对应路径**：便于版本控制和代码审查
4. **完成后输出总结报告**：创建 `docs/MVP-WEEK1-COMPLETE-REPORT.md`

---

## 8. 确认签名

- **负责人**：Windows C++ 底层开发工程师
- **日期**：2026-05-23
- **状态**：✅ 任务理解确认完成，开始执行

---

## 附录：设计文档关键要点

### A.1 VFS 缓存设计要点
- 缓存策略：LRU + TTL
- LRU 容量：1000 个路径映射
- TTL：60 秒
- 大小限制：100MB

### A.2 路径重定向算法优化要点
- 使用 Trie 树存储路径重定向规则
- 查询复杂度：O(k)（k 为路径长度）
- 支持 1000+ 路径重定向规则

### A.3 性能目标要点
- VFS 缓存命中率 ≥ 80%
- 路径重定向延迟 < 0.1ms（P50）
- 单元测试覆盖率 ≥ 80%

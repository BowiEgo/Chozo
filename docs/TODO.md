# Chozo 引擎待办清单（P0 / P1）

> **用途**：跟踪代码评审中定级为 P0、P1 的问题。
> **来源**：2026-09-28 全量评审（完整构建 + `-Wall -Wextra` 全 TU 扫描 + 运行验证 + 格式检查）。
> **范围**：仅 P0/P1 缺陷；P2 级工程化问题见 `docs/P2-PLAN.md`。
> **维护约定**：修完一项后把它移到「已完成」表并附上 commit 与验证方式；新增问题请注明文件:行号与复现方式。

## 严重度定义

| 级别 | 定义 | 处理时机 |
|---|---|---|
| **P0** | 可复现的正确性缺陷：内存安全（UB/UAF/泄漏）、逻辑错误、功能未生效 | 立即修，阻塞其他工作 |
| **P1** | 不立刻崩溃，但会导致资源泄漏、数据竞争、UB 或严重维护风险 | 本迭代内修完 |
| **P2** | 工程化 / 可维护性问题（CI、测试、死代码、文档、构建） | 见 `docs/P2-PLAN.md` |

## 一、已完成

| 编号 | 问题 | 提交 | 验证 |
|---|---|---|---|
| P0-1 | `Handle()` 默认构造时 `m_Obj` 未初始化（-O0/-O2 行为不一致） | `40f25d7` | 新增 `HandleTest`；复现程序输出 `0x0` |
| P0-2 | `TransformComponent::SetTransformParams` 克隆后立刻 `Destroy()`（use-after-free） | `5077e41` | 改为原地更新/深拷贝；组件默认构造即持有有效参数 |
| P0-3 | `EntityRegistry::Destroy/Get` 调用不存在的 `Handle::get()`；`Create`/`ForEach` 无法实例化 | `1cca7f5` | 新增 `EntityRegistryTest` |
| P0-4 | `SceneObj::Update` 中变换系统从未运行（世界矩阵恒为单位阵） | `8b8b627` | 启用并加固失效实体/环状层级 |
| P0-5 | 网格重传泄漏上一份 VkBuffer/VMA 分配 | `4294d2d` | 退出日志中 buffer 均被释放 |
| P0-6 | `TypeRegister::IsLightType` 返回 Mesh 掩码 | `b1dc1b6` | `TypeRegistryTest` 回归用例 |
| P0-7 | `CZ_DEBUGBREAK()` 在 Debug/Release 均为空实现 | `6fd3387` | Debug 下 SIGTRAP，Release 下无操作 |
| P1-1 | 所有权模型缺失（`Handle` 可拷贝 + 手工 `Destroy`） | `0c96f2b`,`07d3bb2` | 见 `docs/ownership.md`；退出 `No active allocations.` |
| P1-10 | ImGui 纹理描述符集合引用了已销毁的 image view（`VUID-vkCmdDrawIndexed-None-08114`） | `bbcbd21` | 确定性复现 9 次 → 0 次 |

## 二、待处理（P1）

### P1-12 引擎核心库在每个镜像中重复链接（单例分裂）

**位置**：`CMakeLists.txt`（`CZCoreLibs` / `CZRuntimeLibs` 由静态库聚合）、`Source/Editor/CMakeLists.txt`、`Source/Backend/Vulkan/CMakeLists.txt`

**现象与影响**

- 静态模块被同时链接进可执行文件与各 dylib，于是 `Logger`、`TypeRegister`、`CameraManager`、`AssetRegistry`、`JobSystem`、`Application` 等单例在进程内各存在一份，跨镜像调用会看到不同状态。
- 已修一部分：`CZMemory` 改为共享库（`07d3bb2`）。此前内存统计与泄漏追踪每镜像一份，跨镜像释放还会触发 `HeapFree underflow`。
- 仍存在：编辑器 dylib 里的 `Application::Get()` / `Logger::Get()` 与可执行文件里的不是同一实例（控制台面板只看得到 dylib 侧日志；`Editor::GetImGuiRenderer()` 依赖 exe 导出符号才不会出错）。

**建议方案**

- 把引擎核心编译为**一个共享库**（如 `libCZCore.dylib`：Core + Runtime + RHI 的公共部分），可执行文件、Editor、Vulkan 后端统一链接它；或
- 显式采用「exe 导出符号 + dylib 不重复链接核心静态库」的模型（导出列表 / `-Wl,-undefined,dynamic_lookup`），并用测试锁定。
- 完成前不要在 dylib 边界两侧依赖任何单例状态。

**验收标准**

- 进程内每个单例只有一个实例（可用打印地址的测试验证）。
- 编辑器控制台能看到后端与引擎的全部日志。

**预估**：1–2 天（以构建结构调整为主）。

---

---

### P1-2 DescriptorSet 缓存：淘汰与并发保护

**位置**：`Source/Runtime/RHI/Device.cpp`（`GetOrCreateDescriptorSet`）、`Include/Runtime/RHI/Device.hpp`（`m_DescriptorSetCache`）

**状态**：键构造已修（`resize` → `reserve`，随 `07d3bb2`）；淘汰与并发保护仍待处理。

**剩余问题**

- 缓存条目只记录 `LastFrame`，没有基于它的淘汰逻辑 → 无上限增长。
- 三份缓存（SetLayout / Sampler / DescriptorSet）均无锁；`AssetRegistry::LoadAssetAsync` 会在 Job 线程创建资源 → 数据竞争。

**建议方案**

- 实现基于 `LastFrame` 的 LRU 淘汰（帧号超过 `MaxFramesInFlight` 阈值即释放条目）。
- 给缓存加锁，或在 API 上标注 main-thread-only 并加断言；与 P1-6 的线程契约一起定。
- 补一个键构造 / 命中行为的单元测试。

**验收标准**

- 单测覆盖键构造与命中/未命中行为。
- 连续创建大量不同 DescriptorSet 后缓存条目数有上限。
- TSan 下并发加载资源无竞争报告。

**预估**：0.5–1 天。

---

---

### P1-3 RHI 命令缓冲：参数校验顺序错误、空函数指针调用

**位置**：`Source/Backend/Vulkan/VulkanCommandBufferObj.cpp:53-64`、`:72`、`:119-126`、`:137-144`

**现象与影响**

- `SetPolygonMode` 无条件调用 `vkCmdSetPolygonModeEXT`，但该函数指针只在 `VK_EXT_extended_dynamic_state_3` 可用时才由 `VulkanDeviceObj::LoadDynamicState3Functions()` 加载；`BindPipeline`（`:72`）同样无条件调用 → 扩展缺失时是空指针调用（崩溃）。
- `BindVertexBuffer`/`BindIndexBuffer` 先 `vkBufferObj->GetVKBuffer()` 再 `if (!vkBufferObj)`，判空形同虚设 → 传入无效句柄时不是报错而是 UB。

**建议方案**

- 把判空/有效性检查移动到任何解引用之前；`As<>()` 结果先校验。
- `DynamicState3Functions` 暴露 `HasXxx()` 查询，缺失时走静态管线状态或明确报错（Fatal）而不是静默调用空指针。

**验收标准**

- 在模拟"扩展不可用"（注入空指针）时不崩溃且日志可读。
- clang-tidy 的 `bugprone-*` 无相关告警。

**预估**：0.5 天。

---

---

### P1-4 Vulkan 队列族：创建与实际使用不一致

**位置**：`Source/Backend/Vulkan/VulkanDeviceObj.cpp:274-277`、`:365`、`:388-399`

**现象与影响**

- `uniqueQueueFamilies` 计算后从未使用；`VkDeviceCreateInfo` 只创建了 Graphics 队列族。
- 随后对 Present/Compute 直接 `vkGetDeviceQueue(indices.Present.value(), ...)`，在 graphics/present/compute 分属不同族的硬件上属于非法用法（取用未创建的队列），校验层报错甚至驱动异常。
- 三处 `std::optional::value()` 均无 `has_value()` 保护。

**建议方案**

- 依据 `uniqueQueueFamilies` 生成 `VkDeviceQueueCreateInfo` 数组（记录每个族要创建的队列数与优先级）。
- 用"族 index → 队列"的映射取队列；`FindQueueFamilies` 不完整时启动即失败并给出可读错误。

**验收标准**

- 在独立 compute/present 队列的设备（或 mock）上无校验层错误。
- 队列族不完整时 `Engine::Init` 返回失败而不是继续初始化。

**预估**：1 天。

---

---

### P1-5 `Buffer` / `SafeBuffer` 内存语义不安全

**位置**：`Include/Core/Memory/Buffer.hpp:18-52`、`:66-77`、`:95-120`

**现象与影响**

- `Buffer(const void* data, size)` 允许包装外部内存，而 `Allocate()` / `Release()` 无条件 `delete[] (Byte*)Data` → 释放非自有内存。
- `SafeBuffer` 未禁用拷贝：默认拷贝构造+析构会对同一指针 `delete[]` → 双重释放。
- `ReadBytes` 分配 `new Byte[size]` 后把裸指针交给调用方，必然泄漏（且绕过内存统计）。
- 全部使用 `new[]/delete[]`，不经过 `HEAP_MALLOC`，无法被内存系统统计与追踪。

**建议方案**

- 删除"包装外部内存"的构造函数，或拆成独立的非拥有类型（如 `BufferView`）。
- `SafeBuffer` 显式 `= delete` 拷贝并实现移动，或直接改用 `std::vector<Byte>`。
- `ReadBytes` 返回 `SafeBuffer` / `std::vector<Byte>`。
- 统一走 `HEAP_MALLOC` 以便纳入内存统计。

**验收标准**

- ASan 下无 double-free / leak；新增单测覆盖拷贝、移动、`Release` 与空 buffer。

**预估**：0.5 天。

---

---

### P1-6 线程安全缺乏统一策略

**位置**

- `Source/Core/FileSystem/VFS.cpp:7-23`（静态 map 无锁；`Resolve` 会在 Job 线程被调用，`:44` 的日志还漏了 `{}` 占位符导致路径丢失）
- `Source/Core/TypeRegistry/TypeRegistry.cpp:34-37`（`GetTypeInfo` 释放锁后返回 `std::vector` 元素指针，`RegisterType` 触发扩容即悬垂）
- `Include/Runtime/RenderCore/AssetRegistry.hpp`（策略与派生类各持一把 mutex，`Clear()` 与 Job 线程加载可并发）
- `Source/Runtime/RHI/Device.cpp`（三份缓存无锁）
- `Source/Core/Log/Logger.cpp`（`RemoveSink` 与并发写日志）
- `Source/Runtime/RenderCore/ShaderCompiler.cpp`（全局单例，异步编译路径）

**现象与影响**：数据竞争、悬垂指针、容器并发读写崩溃；当前没有任何文档说明"哪些对象属于哪个线程"。

**建议方案**

1. 先写线程契约（主线程独占 / 可跨线程 / 加锁可跨线程），落到头文件注释与断言。
2. `VFS`：改用读写锁，或在启动阶段冻结挂载表。
3. `TypeRegistry`：`GetTypeInfo` 返回拷贝，或把存储换成地址稳定的容器（`std::deque` / `vector<unique_ptr>`）。
4. `AssetRegistry`：合并为单一 mutex 并明确锁顺序。
5. `Device` 缓存：加锁或标注 main-thread-only（配合 P1-2）。
6. `Logger`：使用 spdlog 线程安全路径，或先复制 sinks 再修改。

**验收标准**

- TSan 跑"编辑器启动 + 异步资源加载 + 退出"无报告。
- 线程契约写入 `docs/` 并在相关头文件中标注。

**预估**：1–2 天。

---

---

### P1-7 JobSystem：可移植性与实现质量

**位置**：`Source/Core/JobSystem/JobSystem.cpp:110-125`、`:116`、`:193-200`、`:218-221`

**现象与影响**

- `:116` 无条件调用 macOS 专有的 `pthread_threadid_np`（且结果未使用）→ Linux 编译失败，与 CMake 声称的跨平台支持不符。
- `WaitAll` 用 `notify_one` + 轮询空队列自旋，浪费 CPU。
- 队列满时在提交线程同步执行任务（代码已注明 TODO），会让主线程被长任务卡住。
- Job 执行路径无异常保护：`job.OnExecute` 抛异常会直接终止进程；`std::promise::set_value` 亦然。
- `JobSystem::Get()` 在未初始化时返回空句柄，调用方（如 `AssetRegistry::LoadAssetAsync`）无检查。

**建议方案**

- 用 `std::this_thread::get_id()` 替换 `pthread_threadid_np`，或按平台条件编译。
- `WaitAll` 改为基于计数 + 条件变量的等待，去掉轮询。
- 为 `OnExecute` 包 try/catch 并把异常转为失败结果；
- `Get()` 增加有效性检查，或在未初始化时直接断言。

**验收标准**

- Linux（或 CI 容器）编译通过；压力测试下空闲 CPU 占用接近 0。
- 单测覆盖"队列满"与"任务抛异常"两条路径。

**预估**：1 天。

---

---

### P1-8 `Result<T,E>` 实现缺陷与错误处理风格不统一

**位置**：`Include/Core/Header/Result.hpp`；风格混用示例：`CZ_CORE_ASSERT`（多处）、`Result`（`VulkanDeviceObj::Create*`）、`throw std::runtime_error`（`Source/Backend/Vulkan/VulkanGraphicsContextObj.cpp`）、只打日志（`VulkanDeviceObj::CreateDescriptorPool`）

**现象与影响**

- 手写 union 版 `Result`：无拷贝赋值；错误态下 `value()` 返回未构造的 union 成员引用 → UB；无 `value_or`/`map`/`and_then` 等配套接口。
- 已确认的阻塞点：union 成员的默认构造被删除，因此 `Result` 无法承载 move-only 类型（如 `Scope<T>`）。RHI 工厂的 `Result<..., VkResult>` 已在 `07d3bb2` 中移除，改为返回 `Scope`（失败返回空）。
- 错误处理风格混用，调用方无法判断"某个失败需要检查还是已经被吞掉"。

**建议方案**

- 迁移到 `std::expected`（C++23，可用 `tl::expected` 过渡），或补全 Rule of Five 并提供安全访问接口。
- 统一策略：可恢复错误 → `Result` 冒泡到 `Application::Startup`；不可恢复 → Fatal 日志 + 停机；初始化失败不得静默忽略（当前 `Application::Startup` 忽略 `Engine::Init` 返回值）。

**验收标准**

- 错误态访问不再产生 UB（编译期或运行期明确失败）。
- 至少一条初始化失败路径被测试覆盖（`Startup` 返回 false 并给出可读错误）。

**预估**：0.5–1 天。

---

---

### P1-9 断言开关写反（Debug 关闭 / Release 打开）

**位置**：`Include/Core/Header/Assert.hpp:15`

**现象与影响**

- 当前 `#ifndef CZ_DEBUG` 分支才是"启用断言"的实现（注释却写着 `Stripped in Release builds`），即 **Debug 构建断言被剥离，Release 构建反而生效**。
- 配合已在 `6fd3387` 恢复为真实中断的 `CZ_DEBUGBREAK()`，Release 构建会在断言失败时 `__builtin_trap()` 直接终止，而 Debug 构建毫无保护 —— 与预期完全相反。

**建议方案**

- 条件改为 `#ifdef CZ_DEBUG`；Release 分支保留空实现，并消除未使用变量告警（例如 `#define CZ_CORE_ASSERT(...) ((void)0)`）。
- 明确 `CZ_DEBUGBREAK()` 在 Release 下保持空操作（当前已是）。

**验收标准**

- Debug 下故意破坏不变式（例如越界传入 `Buffer::Write`）会立即中断。
- Release 下同一场景只产生日志，不中断。

**预估**：0.5 天（含筛选会立即触发的既有断言）。

---

---

### P1-11 缺少帧延迟删除（deferred deletion）机制

**位置**：`Source/Runtime/RenderCore/Mesh.cpp:25-35`（P0-5 修复时用 `WaitIdle()` 保证安全）、`Source/Runtime/RenderCore/Viewport.cpp:40`（同一策略，`:44` 已留有 `device->EnqueueCleanup(...)` 的注释草稿）、其余运行时资源重建路径

**现象与影响**

- 任何运行时资源重建都要整卡停等 GPU（`vkDeviceWaitIdle`）：编辑器拖拽网格参数、缩放视口时明显掉帧。
- 各处手工 `WaitIdle` 容易漏写，漏写即"销毁在用资源"（校验层报错 / GPU 读写已释放内存）。

**建议方案**

- 在 RHI 层提供帧退休队列：`DeviceObj::Retire(resource, frameIndex)` + 每帧 `CollectGarbage(当前帧 - MaxFramesInFlight)`。
- `GraphicsBuffer` / `Image` / `FrameBuffer` / `Sampler` / `Pipeline` 的 `Destroy()` 统一走该队列（或提供 `DeferredDestroy`）。
- 完成后移除 `Mesh::Upload`、`Viewport::Resize` 中的 `WaitIdle()`。

**验收标准**

- 连续拖拽网格参数无 GPU 停顿（用 Tracy GPU 区段对比修复前后）。
- 移除上述 `WaitIdle()` 后校验层无 "destroying in-use object" 类错误。

**预估**：1–2 天。

---


## 三、回归基线

```bash
# 核心改动（快，无需 Vulkan SDK）
cmake --preset core-debug && cmake --build --preset core-debug && ctest --preset core-debug
cmake --preset core-release && cmake --build --preset core-release && ctest --preset core-release
cmake --preset core-asan && cmake --build --preset core-asan && ctest --preset core-asan

# 完整引擎（需要 Vulkan SDK）
cmake --preset full-release && cmake --build --preset full-release

# 运行验证：Debug + 校验层跑 60s，检查日志与退出报告
./build/dist/Debug/Chozo.app/Contents/MacOS/Launch

# 门禁脚本（与 CI 一致）
.github/scripts/check-format.sh
.github/scripts/check-warnings.sh <build.log> 4
```

CI 见 `.github/workflows/ci.yml`：`format` / `core`（Debug+Release 矩阵）/ `full-macos` / `sanitizers`。

## 附录 A：`dev-0.1.x` 引入的行为变化与注意事项

| 变化 | 说明 |
|---|---|
| `CZ_DEBUGBREAK()` 在 Debug 下真正中断 | 泄漏报告、堆下溢、Fatal 日志现在会 `__builtin_trap()`；退出时已无残留分配，不会再中断 |
| `Handle` 默认构造为 `nullptr` | 之前是未初始化值；依赖"默认句柄非空"的代码（如有）会暴露出来 |
| 移除 `Handle(const TObject*)` 重载 | 该重载永远无法编译（const 指针赋给非 const 成员），全仓库无调用方 |
| `EntityRegistry::ForEach` → `Begin()` / `End()` | 原实现无法实例化；若后续需要遍历请用新接口 |
| `TransformComponent` 改为值语义 | 不再分配 params 对象；`SetTransformParams` 只做字段拷贝 |
| 网格重新上传会 `WaitIdle()` | 见 P1-11，属过渡方案 |
| `Handle<T>` 不再有 `Destroy()` | 释放只能经由拥有者（`Scope` 或容器）；`ViewOf()` / `ViewAs<T>()` 从拥有者派生视图 |
| `CZMemory` 变为共享库 | 内存统计与泄漏追踪现在是进程级唯一的；bundle 的 Frameworks 目录会多出 `libCZMemory.dylib` |
| `TransformSystem` 开始运行 | `WorldMatrix` 现在会真正被计算；此前恒为单位阵 |

---

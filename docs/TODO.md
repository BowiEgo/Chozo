# Chozo 引擎待办清单（P0 / P1）

> **用途**：跟踪代码评审中定级为 P0、P1 的问题，作为后续迭代的施工清单。
> **来源**：2026-09-28 全量评审（完整构建 + `-Wall -Wextra` 全 TU 扫描 + 运行验证 + 格式检查）。
> **范围**：仅收录 **P0（确定性缺陷）** 与 **P1（设计 / 资源管理风险）**。P2 级工程化问题（CI、测试覆盖、死代码、文档）见附录 B，不在本清单内。
> **状态**：P0 共 7 项，已在分支 `dev-0.1.x` 修复并验证；P1 共 11 项，待处理。
> **维护约定**：修完一项后，把它从第二章移到第一章表格，并附上 commit 与验证方式；新增问题请标出文件:行号与复现方式。

## 严重度定义

| 级别 | 定义 | 处理时机 |
|---|---|---|
| **P0** | 可复现的正确性缺陷：内存安全（UB/UAF/泄漏）、逻辑错误、功能未生效 | 立即修，阻塞其他工作 |
| **P1** | 不立刻崩溃，但会导致资源泄漏、数据竞争、UB 或严重维护风险 | 本迭代内修完 |
| **P2** | 工程化 / 可维护性问题（CI、测试、死代码、文档、构建整洁度） | 见附录 B |

---

## 一、P0：已修复（分支 `dev-0.1.x`）

分支基于 `refactor@934d034`，共 7 个提交，每个 P0 一个提交。

| 编号 | 问题 | 影响 | 提交 | 验证方式 |
|---|---|---|---|---|
| P0-1 | `Handle()` 默认构造时 `m_Obj` 未初始化 | 默认句柄持有不确定指针，`operator bool`/`IsValid`/`operator==` 读垃圾值；实测同一份代码 `-O0` 返回 1、`-O2` 返回 0 | `40f25d7` | 新增 `Source/Core/Header/Tests/HandleTest.cpp`；最小复现程序输出 `m_Obj = 0x0` |
| P0-2 | `TransformComponent::SetTransformParams` 克隆后立刻 `Destroy()` | 组件持有的 `Params` 立即悬垂；每次编辑器改 Transform（属性面板 → SyncBridge → `Scene::SetTransform`）后读写即 use-after-free | `5077e41` | 改为原地更新自有权重对象；默认构造保证 params 有效；新增显式 `TransformParamsObj::operator=` |
| P0-3 | `EntityRegistry::Destroy/Get` 调用不存在的 `Handle::get()`；`Create`/`ForEach` 无法实例化 | 注册表 API 一旦被调用即编译失败 | `1cca7f5` | 新增 `Source/Core/Header/Tests/EntityRegistryTest.cpp`（create/get/destroy + 空句柄） |
| P0-4 | `SceneObj::Update` 中 `TransformSystem::Update()` 被注释，系统从未运行 | `WorldMatrix` 恒为单位阵，渲染提交的 `ModelMatrix` 不随节点变化 | `8b8b627` | 启用更新并加固：跳过已销毁/缺组件的实体，层级遍历加上限防环 |
| P0-5 | `MeshObj::Upload` 覆盖旧 `VertexBuffer`/`IndexBuffer` 句柄 | 每次按参数重建网格泄漏一对 VkBuffer + VMA 分配 | `4294d2d` | 运行验证：退出日志中两个 buffer 均被正确销毁 |
| P0-6 | `TypeRegister::IsLightType` 返回 `m_MeshMask` | 光源类型查询恒等于 Mesh 掩码 | `b1dc1b6` | 代码审查（当前无调用方，防回归） |
| P0-7 | `CZ_DEBUGBREAK()` 在 Debug/Release 均为空实现 | 断言失败、堆下溢、Fatal 日志、泄漏报告都无法中断到调试器 | `6fd3387` | 独立验证：Debug 下触发 SIGTRAP（exit 133），Release 下为空操作 |

**回归基线（当前值）**

```bash
cmake -S . -B build && cmake --build build -j8   # 0 error；警告集合与修复前一致（12 条，均为既有问题）
./build/Source/Test/CZTest                        # 13 test cases / 75 assertions, all pass
./build/dist/Debug/Chozo.app/Contents/MacOS/Launch  # 正常启动、渲染、退出
```

---

## 二、P1：待处理

### 建议处理顺序

| 顺序 | 编号 | 理由 |
|---|---|---|
| 1 | P1-9 断言开关写反 | 改动最小，但决定后续所有调试手段是否有效 |
| 2 | P1-2 DescriptorSet 缓存 | 与 P1-10 的校验层报错高度相关，先修可缩小排查面 |
| 3 | P1-10 校验层 descriptor 报错 | 定位类任务，修完才能信任渲染日志 |
| 4 | P1-3 命令缓冲判空/空函数指针 | 崩溃隐患，改动小 |
| 5 | P1-1 所有权模型 | 工作量最大，是 P1-11、资源泄漏类问题的根因 |
| 6 | P1-11 帧延迟删除 | 依赖 P1-1 的生命周期决策 |
| 7 | P1-5 Buffer 内存语义 | 独立小改动，消除双重释放风险 |
| 8 | P1-6 线程安全策略 | 需要先定线程契约，工作量中等 |
| 9 | P1-4 Vulkan 队列族 | 影响特定硬件，改动力度中等 |
| 10 | P1-7 JobSystem | 影响跨平台构建与 CPU 占用 |
| 11 | P1-8 Result / 错误处理统一 | 影响面广，放在其他项之后统一收口 |

---

### P1-1 所有权模型缺失（`Handle` 可拷贝 + 手工 `Destroy`）

**位置**

- `Include/Core/Header/Handle.hpp`（句柄可自由拷贝，`Destroy()` 手工调用）
- `Include/Runtime/RenderCore/AssetRegistry.hpp:35`（`ResourceStoragePolicy::Deallocate` 是空实现）
- `Source/Runtime/RenderCore/Camera/CameraManager.cpp:11-19`（`Shutdown()` 直接 `Delete(camera.Camera)`）
- `Source/Runtime/RenderCore/Renderer.cpp:24`（`static Pipeline testPipeline` 全局对象）
- `Include/Runtime/RenderCore/Components/TransformComponent.hpp:15`、`Include/Runtime/RenderCore/Components/MeshComponent.hpp`

**现象与影响**

- 一次句柄拷贝就产生多个"看似拥有"的引用，谁该调用 `Destroy()` 完全靠约定；双重释放、悬垂、泄漏都只是时间问题。
- 实测（运行一次编辑器后退出）`ReportMemoryLeaks` 残留 2 处：
  - `TransformParamsObj`（48 B，`TransformParams.hpp:96`）——每个 `TransformComponent` 自有一份且从不释放；
  - `CubeParamsObj`（72 B，`CubeParamsObj.hpp:30`）——编辑器节点持有的 Mesh 参数。
- `CameraManager::Shutdown()` 删除的 `CameraObj` 仍被 `Viewport::m_Camera`（`SceneCamera` 句柄）引用 → 关闭后句柄悬垂；且该方法被 `Renderer::Shutdown`（`Renderer.cpp:115`）与 `Engine::Shutdown`（`Engine.cpp:76`）各调用一次。
- `AssetRegistry` 的 `Deallocate` 为空，`EntityRegistry::Destroy` 会静默泄漏（当前无调用方，属于埋雷）。

**建议方案**

1. 明确区分两类句柄：**非拥有视图**（可拷贝，不负责释放）与**拥有句柄**（`Scope<T, Deleter>` 或带引用计数的 `Ref<T>`）。
2. 把 `Destroy()` 从公共接口移出（例如只允许通过 `AccessKey` 或 owner 类型调用），让"谁能释放"在编译期可见。
3. RHI 资源（`GraphicsBuffer`/`Image`/`FrameBuffer`/`Pipeline`/`Sampler`/`DescriptorSet`/`SetLayout`）统一为引用计数或统一归 `DeviceObj` 托管。
4. `CameraManager` 改为不拥有相机对象（只存弱引用/原始指针表），销毁责任回归创建者；`Shutdown()` 保证幂等。

**验收标准**

- 正常使用并退出编辑器后 `ReportMemoryLeaks` 无残留。
- `Viewport` 在相机销毁后不会被解引用（可加断言或弱引用检查 + 单测）。
- 重复调用 `Shutdown()` 幂等，且有测试覆盖。

**预估**：2–3 天（含回归与单测）。

---

### P1-2 DescriptorSet 缓存：键构造错误、无淘汰、无并发保护

**位置**：`Source/Runtime/RHI/Device.cpp:119-135`、`Include/Runtime/RHI/Device.hpp:98-103`

**现象与影响**

- `GetOrCreateDescriptorSet` 中 `key.BindingResources.resize(bindings.size() * 2)` 之后仍然 `push_back`，导致缓存键由「2N 个零 UUID + 实际资源 UUID」组成（应为 `reserve`）。
- `key.LastFrame` 只在查找时写入，且没有任何基于它的淘汰逻辑 → 缓存无上限增长。
- 三份缓存（SetLayout / Sampler / DescriptorSet）均无锁；而 `AssetRegistry::LoadAssetAsync` 会在 Job 线程创建资源 → 数据竞争。

**建议方案**

- `resize` 改 `reserve`（或 `clear()` + `push_back`），并补一个键构造的单元测试（不同 binding 集合/顺序/资源不得误命中）。
- 实现基于 `LastFrame` 的 LRU 淘汰（帧号超过阈值即释放）。
- 给缓存加锁，或在 API 上标注 main-thread-only 并加断言；与 P1-6 的线程契约一起定。

**验收标准**

- 单测覆盖键构造与命中/未命中行为。
- 连续创建大量不同 DescriptorSet 后缓存条目数有上限。
- TSan 下并发加载资源无竞争报告。

**预估**：0.5–1 天。

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

### P1-8 `Result<T,E>` 实现缺陷与错误处理风格不统一

**位置**：`Include/Core/Header/Result.hpp`；风格混用示例：`CZ_CORE_ASSERT`（多处）、`Result`（`VulkanDeviceObj::Create*`）、`throw std::runtime_error`（`Source/Backend/Vulkan/VulkanGraphicsContextObj.cpp`）、只打日志（`VulkanDeviceObj::CreateDescriptorPool`）

**现象与影响**

- 手写 union 版 `Result`：无拷贝赋值；错误态下 `value()` 返回未构造的 union 成员引用 → UB；无 `value_or`/`map`/`and_then` 等配套接口。
- 错误处理风格混用，调用方无法判断"某个失败需要检查还是已经被吞掉"。

**建议方案**

- 迁移到 `std::expected`（C++23，可用 `tl::expected` 过渡），或补全 Rule of Five 并提供安全访问接口。
- 统一策略：可恢复错误 → `Result` 冒泡到 `Application::Startup`；不可恢复 → Fatal 日志 + 停机；初始化失败不得静默忽略（当前 `Application::Startup` 忽略 `Engine::Init` 返回值）。

**验收标准**

- 错误态访问不再产生 UB（编译期或运行期明确失败）。
- 至少一条初始化失败路径被测试覆盖（`Startup` 返回 false 并给出可读错误）。

**预估**：0.5–1 天。

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

### P1-10 校验层报错：Set 0 Binding 0 采样图未绑定

**位置 / 证据**

- 运行 Debug 构建（校验层已启用，见 `Source/Backend/Vulkan/VulkanGraphicsContextObj.cpp`），首个场景绘制起每帧报：

  ```text
  VUID-vkCmdDrawIndexed-None-08114: the sampled image descriptor
  [VkDescriptorSet 0x290000000029, Set 0, Binding 0, Index 0, variable "_Texture"]
  is using imageView VkImageView 0x0 that is invalid or has been destroyed.
  ```

**已知信息（尚未定位到根因）**

- `Basic.slang` 的反射是 `[Set 0, Binding 0] u_Camera (Type: 4, Size: 128)`（uniform buffer），`CommandListObj::Draw(Scene, Camera)`（`Source/Runtime/RHI/CommandList.cpp:10-27`）绑定的也是相机缓冲 —— 与报错中的"采样图"不符。
- 编辑器侧 `VulkanImGuiRenderer::GetTextureID`（`Source/Editor/Renderer/Vulkan/VulkanImGuiRenderer.cpp:123-140`）会把视口纹理通过 `ImGui_ImplVulkan_AddTexture` 注册为 ImGui 纹理，且**忽略返回值**（`VK_NULL_HANDLE` 也会被缓存并当作 `ImTextureID` 使用）。
- 因此可能是 descriptor set 与 pipeline layout 不匹配（与 P1-2 的 key 构造问题相关），也可能是 ImGui 纹理注册路径传入了空 imageView/sampler。

**待办**

1. 定位该绘制来自哪条路径：临时日志或 Tracy 标记 `BindDescriptorSets` / `ImGui_ImplVulkan_AddTexture`，或抓帧（RenderDoc）对齐 pipeline layout 与 descriptor set。
2. 修掉绑定/布局不一致。
3. 让 `GetTextureID` 在 `AddTexture` 返回 `VK_NULL_HANDLE`、`GetOrCreateVKView()` 返回空、`GetSampler()` 为空时快速失败并给出可读错误（当前只打警告）。

**验收标准**：Debug 运行 60 秒，校验层除既有的 vertex-input 告警外无 descriptor 相关错误。

**预估**：0.5–1 天（以定位为主）。

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

## 三、回归基线（每个 P1 修复都必须跑）

```bash
# 1. 构建（不得引入新警告；当前基线为 12 条既有告警）
cmake -S . -B build && cmake --build build -j8

# 2. 单元测试（当前 13 cases / 75 assertions）
./build/Source/Test/CZTest

# 3. 运行验证：Debug + 校验层跑 60s，检查日志
./build/dist/Debug/Chozo.app/Contents/MacOS/Launch

# 4. 退出时检查泄漏报告（注意：Debug 下会 CZ_DEBUGBREAK 中断）
```

**建议补充**

- ASan/UBSan 构建 + 测试（覆盖 P1-1、P1-5）。
- TSan 跑"启动 + 异步加载 + 退出"（覆盖 P1-2、P1-6、P1-7）。
- 引入 CI（P2，见附录 B），把上述 1–4 步自动化。

---

## 附录 A：`dev-0.1.x` 引入的行为变化与注意事项

| 变化 | 说明 |
|---|---|
| `CZ_DEBUGBREAK()` 在 Debug 下真正中断 | 泄漏报告、堆下溢、Fatal 日志现在会 `__builtin_trap()`。当前退出时因 P1-1 的 2 处残留会被中断；修完 P1-1 即可消失 |
| `Handle` 默认构造为 `nullptr` | 之前是未初始化值；依赖"默认句柄非空"的代码（如有）会暴露出来 |
| 移除 `Handle(const TObject*)` 重载 | 该重载永远无法编译（const 指针赋给非 const 成员），全仓库无调用方 |
| `EntityRegistry::ForEach` → `Begin()` / `End()` | 原实现无法实例化；若后续需要遍历请用新接口 |
| `TransformComponent` 默认构造会分配一份 params | 每个组件一次堆分配，且随组件销毁不释放（属 P1-1 范围） |
| 网格重新上传会 `WaitIdle()` | 见 P1-11，属过渡方案 |
| `TransformSystem` 开始运行 | `WorldMatrix` 现在会真正被计算；此前恒为单位阵 |

---

## 附录 B：P2 摘要（不在本清单内）

- **CI/测试**：无 `.github/`；单测仅覆盖 allocator（9）与 Handle/EntityRegistry（4），RHI/RenderCore/Window/Log/Event 无测试。
- **死代码**：约 622 处注释掉的代码；`Renderer.cpp` 的 `#if 0`/`#if 1` 与 `static Pipeline testPipeline`；`Resources/Shaders/Test copy*.slang` 共 11 份重复文件；`Scripts/Embed.py` 与 `CMake/EmbedRuntime.cmake` 已停用。
- **文档**：根 `README.md` 是路线图而非使用说明；`Components/README.md`、`Scene/README.md` 与现状不符。
- **构建**：`FetchDependencies.cmake` 硬编码 `ghfast.top` 镜像前缀；ImGui 使用移动分支 `GIT_TAG docking`；各模块 `LINK` 未声明真实依赖（依赖顶层聚合库兜底）。
- **分层**：Editor 直接 `#include` Backend/Vulkan 与 SDL 内部头（`EditorLayer.cpp`、`VulkanImGuiRenderer.cpp`）。
- **跨平台**：CMake 有 Windows/Linux 分支，但 `CreateVKSurface` 只有 Win32/macOS，`Source/Core/Platform` 仅有 `Mac/`。

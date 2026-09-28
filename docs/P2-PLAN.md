# Chozo 引擎 P2 施工方案（工程化 / 可维护性）

> **用途**：`docs/TODO.md` 收录 P0/P1 缺陷；本文档收录 **P2 级工程化问题**的施工方案（CI、测试、死代码、文档、构建、分层、跨平台）。
> **来源**：2026-09-28 评审的 P2 清单 + P0/P1 修复过程中新发现的问题；所有证据均按 `dev-0.1.x` 当前代码重新核实（部分 P2 证据因 P1-1 重构而过期，已更新）。
> **状态**：Batch 1（G1/G2）、Batch 2（G3/G4/G5）已完成；Batch 3 进行中：G2 剩余已补齐（含渲染核心测试与三个并发/UB 缺陷修复），G8 部分完成，G6 待做，G7 待平台决策。
> **维护约定**：每组完成后把标题改成「—— 已完成」，附上 commit 与验证方式；新增 P2 问题请注明文件:行号与复现方式。

## 分组总览

| 组 | 目标 | 价值 | 成本 | 依赖 | 顺带推进的 P1 |
|---|---|---|---|---|---|
| **G1** CI 与质量门禁 —— 已完成 | 让「能编译 + 测试通过 + 格式合规」成为可执行门槛 | ★★★★★ | 1 天 | — | P1-9（断言写反会被 CI 暴露） |
| **G2** 测试基础设施 —— 已完成 | 恢复被注释单测、补齐核心模块、接入 `ctest` | ★★★★★ | 1.5–2 天 | G1 | P1-8（改造信心）、P1-2/P1-6（回归网） |
| **G3** 死代码与残留清理 —— 已完成 | 删除/归档 624 处注释代码、未编译文件、停用脚本 | ★★★★ | 1–1.5 天 | G2 | — |
| **G4** 文档重建 —— 已完成 | 根 README 可用化 + 所有权模型留档 | ★★★★ | 1 天 | — | P1-1 的约定固化 |
| **G5** 构建与依赖卫生 —— 已完成 | 依赖可复现、`LINK` 声明真实、镜像可关、SDK 定位不写死 | ★★★ | 1–1.5 天 | — | P1-12 的前置 |
| **G6** 分层解耦 | Editor 不再直接依赖 Vulkan/SDL 内部实现 | ★★★ | 1.5–2 天 | G5 | P1-11 的落点 |
| **G7** 跨平台 —— 进行中（平台可移植性已完成，Windows 完整构建待验证） | 先定目标平台，再分期落地 | ★★ | 3–5 天 | G1、P1-7 | P1-7（Linux 门禁逼出） |
| **G8** 渲染/着色器契约 —— 部分完成 | 顶点属性一致性、静默不绑定的兜底 | ★★ | 0.5 天 | — | P1-10 遗留项 |

## 当前证据基线（已核实）

| 项 | 现状 |
|---|---|
| CI | 无 `.github/`；`ctest` 未接入（无 `enable_testing`/`add_test`）；`CZTest` 只是被拷进 `.app` |
| 单测 | 13 用例（allocator 9 + Handle/EntityRegistry 4）；`MemoryTest.cpp` 整文件注释（18 用例）；Event/Log/RHI/RenderCore/Window 的 `Tests/` 为空目录 |
| 告警 | 全 TU 扫描 4 条：`ShaderReflectingPrinting.hpp` 2 条 sign-compare、`VulkanTextureObj.hpp` 未用私有字段、`Editor.cpp` C-linkage |
| 格式 | 无门禁；3 个项目文件不合规（`Backend/Vulkan/VulkanGraphicsBufferObj.cpp`、`Core/Platform/Mac/MacFile.mm`、`MacUtils.mm`），另有 `External/stb/*.h` 3 个第三方头 |
| 注释代码 | 624 处；`Renderer.cpp:41` `#if 0`、`:52` `#if 1`；`Renderer.hpp:44-57` 的 `CZ_RENDERER_SCOPE_PERF/TIMER` 宏引用不存在的 `ScopePerfTimer`/`ScopedTimer` |
| 未编译文件 | `RenderCore/Material.cpp`、`RenderCore/ProceduralMesh/Cube.cpp`（均不在 `MODULE_SRC`）；`ProceduralMesh/{Cube,Sphere,Quad,SphereParams}.hpp` 无任何 TU 包含（`Cube.hpp` 里 `override` 非虚函数，一旦被包含即编译失败） |
| 外部残留 | `Resources/Shaders/Test copy*.slang` 10 份被 git 跟踪；`Scripts/Embed.py` + `CMake/EmbedRuntime.cmake` 已停用（`CMakeLists.txt:170` 注释）；`External/stb` 未接入构建且无 `#include` |
| 桩代码 | `AssetsPanel::Draw`、`TextureViewerPanel::Draw` 只有 `Begin/End` |
| CMake | `CPP_RTTI_ENABLED`(:27)、`CZ_OPTION_BUILD_BENCHMARKS`(:40) 定义后从未使用；`FetchDependencies.cmake:12` **全局覆盖 CMake 内建 `FetchContent_Declare` 宏**；`:6` 硬编码 `ghfast.top`；ImGui `:101` 用移动分支 `GIT_TAG docking` |
| 依赖声明 | `RenderCore` 使用 `<Runtime/RHI/*>` 但 `LINK` 只有 `CZCoreLibs slang EnTT`；`App` 只 `LINK CZCoreLibs`（实际依赖 RHI/RenderCore/Window），依赖全靠顶层 `CZRuntimeLibs` 兜底 |
| 分层 | `EditorLayer.cpp:9` include `../Runtime/Window/SDLWindow/SDLWindowObj.hpp`；`VulkanImGuiRenderer.cpp:4-10` include 6 个 `Backend/Vulkan/*Obj.hpp` 内部头 |
| 跨平台 | `CreateVKSurface` 只有 Win32/macOS 分支；`Source/Core/Platform` 仅 `Mac/`；bundle/Info.plist 装配为 macOS 专属 |
| 杂项 | `Event.cpp:3` `new EventBus()` 永不释放；`Application.cpp:118` 不可达 `return true;`；`Application.cpp:83` `deltaTime` 硬编码 0.1；`Launch.cpp:12` 未用变量、失败时 `return 0` |

---

## G1 CI 与质量门禁 —— 已完成

**目标**：把已有的隐性标准（0 error、测试通过、格式合规）变成「红灯即拒」。

**落地内容**

- `CZ_OPTION_BUILD_RUNTIME`（默认 ON）：关闭时只构建 13 个 Core 模块 + `CZTest`，不 Fetch SDL/slang/ImGui/VMA/entt，也不需要 Vulkan SDK。核心配置实测 **configure 35s / build 6s**。
- `CMakePresets.json`：`core-{debug,release,asan}` 与 `full-{debug,release}` 的 configure/build/test preset，本地与 CI 跑同一组命令。
- `.github/workflows/ci.yml`：`format`（固定 clang-format 22.1.5，PR 上只检查改动文件）、`core`（Debug+Release 矩阵，构建 → 告警预算 → `ctest`）、`windows-core`（MSVC 核心构建，见 G7）、`sanitizers`（ASan+UBSan 跑测试）、`full-macos`（完整引擎）。
- **额度策略**：私有仓库按 Linux ×1 / Windows ×2 / macOS ×10 计费，`full-macos` 因此只在 PR、每周定时与手动触发时运行（每次 push 省下约 100 分钟额度）；每个 job 设 `timeout-minutes`，`ctest` 设 `--timeout`，避免挂死的测试把整段额度烧完。
- `.github/workflows/full-build.yml`：完整 macOS 构建（LunarG SDK 缓存安装），当前仅 `workflow_dispatch`，待 G5 完成后并入主 CI。
- `.github/scripts/check-format.sh`、`.github/scripts/check-warnings.sh`（本地可直接运行，避免"CI 专用逻辑"）；`.clang-format-ignore` 排除 `External/`、`build/` 与 ObjC++ 源文件。
- 格式化 3 个不合规文件（`VulkanGraphicsBufferObj.cpp` 等）；告警预算：core 为 0，full 暂为 4（P1-10 顶点属性 4 条）。

**验收结果**：core-debug / core-release / core-asan 三种配置构建 + 测试全绿；`check-format.sh` 247 文件 0 失败；`check-warnings.sh` 报告项目告警 0 条（core）。

**步骤**

1. **提供可裁剪的构建配置**（CI 便宜可靠的前提）：新增 `CZ_OPTION_BUILD_RUNTIME`（默认 ON），关闭时只构建 13 个 Core 模块 + `CZTest`，不 Fetch SDL/slang/ImGui/VMA/entt，也不要求 Vulkan SDK。这样核心测试可以在任何机器/CI runner 上几十秒内跑完。
2. **`CMakePresets.json`**：`core-debug` / `core-release` / `core-asan` / `full-debug` / `full-release` + 对应 build/test preset，让本地与 CI 跑**同一组命令**。
3. **`.github/workflows/ci.yml`**：
   - `format`：以仓库固定的 clang-format 版本检查改动文件（版本必须与服务端一致，否则会产生无意义的 diff）；
   - `core`：Linux runner，`core-debug` + `core-release` 构建并 `ctest`；
   - `sanitizers`：Linux + `-fsanitize=address,undefined` 跑 `CZTest`（覆盖 P1-5 类内存语义问题）；
   - `full-macos`：安装固定版本 Vulkan SDK 后做完整构建（依赖 G5 的 SDK 定位改造；在 G5 完成前用 `workflow_dispatch` 触发，避免绿灯依赖运气）。
4. **告警预算**：CI 中把当前 4 条告警登记为白名单存量，禁止新增（新增即红）。
5. **依赖缓存**：`actions/cache` 缓存 `build/_deps`（FetchContent 是最大耗时项）。

**验收**

- PR 上 `format` / `core` / `sanitizers` 三条全绿；故意引入格式错误、失败测试、新告警时分别变红（用一个 draft PR 验证门禁真的有效）。
- 本地 `cmake --preset core-release && cmake --build --preset core-release && ctest --preset core-release` 与 CI 结果一致。

**预估**：1 天。**风险**：macOS runner 的 Vulkan SDK 安装（下载体积大 + 目录布局假设），因此把 `full-macos` 放在 G5 之后转正。

---

## G2 测试基础设施 —— 已完成

**目标**：从 13 个用例到「核心模块都有回归网」，并让测试可被一条命令驱动。

**落地内容**

- 恢复 `MemoryTest.cpp`：18 个用例重新启用（删掉依赖已废弃 `MemoryTraits` 路径的那一条），并把 `ReportMemoryLeaks()` 换成 `GetMemoryLeaks()`——前者会触发 `CZ_DEBUGBREAK()`，Debug 下会打死测试进程。
- 新增 `EventTest` / `LogTest` / `TypeRegistryTest` / `VFSTest` / `JobSystemTest`，覆盖分发与短路、sink 生命周期与级别映射、类型分类掩码（含 P0-6 的回归用例）、VFS 协议解析与**路径穿越的当前行为**、Job 提交/等待/队列满回退/幂等。
- `include(CTest)` + `Source/Test/CMakeLists.txt` 注册 `add_test`，移除把 `CZTest` 拷进 `.app` 的 POST_BUILD。

**验收结果**：用例 **13 → 61**、断言 210，Debug / Release / ASan+UBSan 三种配置全绿；`ctest --preset core-*` 即完整运行。

**Batch 3 补齐**：把 `MeshComponent`/`SceneObj` 对 `Application` 单例的依赖改为注入（`SceneObj::SetMeshRegistry`、`MeshComponent` 以参数接收 registry、`RendererSpecification` 携带 mesh/shader registry），于是新增 `CZRenderCoreTests` 目标（自有 doctest main，`add_test` 注册，仅在 runtime 开启时构建）：
- `SceneTest`：默认组件、命名、销毁、改父、父/子解绑、网格资产生成、RenderData；
- `TransformSystemTest`：根节点、父子继承、三级链路、旋转继承、已销毁实体、环状层级；
- `ParamsTest`：值比较/赋值、两个 visitor 接口、`TParamsFactory` 深拷贝、MeshParams 字段。

写这些用例时暴露并修掉了三个真实缺陷：
1. `Handle` 同时声明 const / 非 const `operator==` → C++20 下 `a == b` 因反向候选而歧义（`-Wambiguous-reversed-operator`），在断言中出现反直觉结果；
2. `~JobSystemObj()` 在不持有 `WakeMutex` 时置 `IsRunning=false` → worker 丢失唤醒，`join()` 永久阻塞（压测约 1/12 复现，现 40/40 通过）；
3. `Submit()` 在发布任务**之后**才自增未完成计数 → worker 可先完成并递减导致计数回绕，`WaitAll()` 永久等待。

**步骤**

1. **恢复 `MemoryTest.cpp`**：18 个用例已写好只是被注释。取消注释后修 API 漂移（`Handle::Destroy` / `EntityRegistry::Destroy` 已删除）；把其中调用 `ReportMemoryLeaks()` 的用例改为使用 `GetMemoryLeaks()`——因为 `ReportMemoryLeaks()` 会触发 `CZ_DEBUGBREAK()`（Debug 下直接 trap，会打死测试进程）。
2. **接入 CTest**：根 `CMakeLists.txt` 加 `include(CTest)`；`Source/Test/CMakeLists.txt` 增加 `add_test(NAME CZTest COMMAND CZTest)`；移除把 `CZTest` 拷进 `.app` 的 POST_BUILD（测试程序不属于应用包）。
3. **补齐核心模块测试**（每模块 3–6 个用例，重点是**契约**而非覆盖率）：
   - `Log`：级别过滤、多 sink、`LogCallback` 收到格式化后的消息；
   - `Event`：`EventDispatcher` 按类型分发、handled 短路、`EventBus` 增删监听器；
   - `TypeRegistry`：注册幂等、掩码分类（`Node/Mesh/Light` 各自命中）、`GetTypeInfo` 在并发注册下不返回悬垂指针（P1-6 的回归网）；
   - `VFS`：协议解析、未挂载路径回退、**路径穿越（`../`）行为**——当前 `Resolve` 无任何防护，需要先用测试固定行为再决定是否加固；
   - `JobSystem`：提交/等待、队列满时回退到调用线程、任务抛异常不终止进程（P1-7 的回归网）；
   - `EntityRegistry`：已有，补「父对象析构后视图失效」的语义测试与 `ViewOf/ViewAs` 用法。
4. **RenderCore 的测试边界**：`RenderCore` 因 `ShaderCompiler`/`MeshObj::Upload` 依赖 Vulkan，不适合整体单测。把**不依赖 GPU** 的部分拆成独立测试目标：`TransformSystem`（父子世界矩阵、脏标记传播、环状层级保护）、`Scene` 的 ECS 行为、`Params` 访问者。前置条件：`MeshComponent` 目前通过 `Application::Get().GetEngine()->GetMeshRegistry()` 取全局单例，需要改成注入，否则测试只能靠 hack。
5. **（可选）ASan/UBSan**：由 G1 的 `sanitizers` preset 覆盖。

**验收**

- `ctest` 一条命令跑全部；用例数 ≥ 35；
- `TransformSystem` 有 ≥ 4 个用例（含环状层级与「子节点先于父节点被标记」的场景）；
- 全部用例在 `core-debug`、`core-release`、`core-asan` 三种配置下通过。

**预估**：1.5–2 天。**风险**：`ReportMemoryLeaks` 的 trap 行为（已在步骤 1 说明规避方式）；RenderCore 拆测试目标会牵出单例依赖，属设计改动，建议单独一个 PR。

---

## G3 死代码与残留清理 —— 已完成

**目标**：降低阅读噪音，删除未编译/未使用的资产，每一步都在 G2 的回归网下进行。

**结果**：95 个文件、**删除 1677 行**；注释代码计数 **624 → 86**（剩余为说明性注释，见下）；全量构建与三种核心配置测试均通过，编辑器运行正常。

**删除清单**

- 未参与构建的源文件：`Material.{cpp,hpp}`、`ProceduralMesh/Cube.cpp` 与 `Cube/Sphere/Quad/SphereParams.hpp`（它们"override"了非虚函数，一旦被包含即编译失败）、`Scene/Entity.cpp`、`Core/Profiler/TracyProfiler.cpp`（空 TU）、`MeshParamsInternal.hpp`（无人包含）。
- embed 脚手架：`Scripts/Embed.py`、`CMake/EmbedRuntime.cmake`、被注释的 include 与 `CZ_OPTION_EMBED_RUNTIME`。
- 10 份 `Test copy*.slang`（仅被已删除的 `#if 0` 着色器列表引用）。
- `AssetsPanel` / `TextureViewerPanel` 桩（只打开空窗口）。
- 死宏与死选项：`CPP_RTTI_ENABLED`、`CZ_OPTION_BUILD_BENCHMARKS`、引用不存在类型的 `CZ_RENDERER_SCOPE_*`、无人调用也无人实现的 `RHIResource::GetResourceType()`、仅被已删除 `Cube.cpp` 读取的 `MeshObj::LocalTransform`。
- 约 410 行注释掉的实现（分三轮按启发式清理，保留 `TODO/NOTE` 与说明性注释）。

**顺手修复**：`EventBus` 不再泄漏堆单例；`CZProfiler` 改为 INTERFACE 模块（消除 `ranlib: table of contents is empty`）；`Application::Run` 使用真实且钳制过的帧时间；`Application::OnEvent` 删除不可达 `return`；`Launch` 失败时返回非零退出码。

**验收说明**：原定"注释代码 < 50"未达成（当前 86，且剩余基本是说明性注释）。后续约定改为：**新增代码不得引入成段注释实现**；`grep -rn "^\s*//\s*[a-zA-Z_].*[;{)]" Source Include` 作为趋势指标而非硬门禁。

**步骤**（每步一个 PR，便于回滚）

1. 删除未编译/未被包含的文件：`RenderCore/Material.cpp`、`RenderCore/ProceduralMesh/Cube.cpp`、`ProceduralMesh/{Cube,Sphere,Quad,SphereParams}.hpp`；确认 `CubeParamsObj` 仍被使用后保留。
2. 归档停用脚本：`Scripts/Embed.py` + `CMake/EmbedRuntime.cmake` + `CMakeLists.txt:170`；若保留打包方案则移入 `tools/embed/` 并在 README 标注「未启用」。
3. 删除 10 份 `Resources/Shaders/Test copy*.slang`（先 grep 引用）。
4. 清理注释代码（624 处），优先 `Renderer.cpp`、`EditorLayer.cpp`、`VulkanCommandBufferObj.cpp`；删除大段注释掉的实现，保留「为什么这样做」的说明。
5. 清理无效宏与桩：`Renderer.hpp:44-57` 的 profiler 宏；`AssetsPanel`/`TextureViewerPanel` 桩；`CMakeLists.txt:27,40` 死选项；`VulkanTextureObj.hpp:21` 未用字段；`RHIResource::GetResourceType()`（只有 4 个类型实现、无调用）→ 补全或删除；`MeshObj::LocalTransform` 确认后删除。
6. `CZProfiler` 是空静态库（`TracyProfiler.cpp` 只有一行 `#include`）→ 构建期产生 `ranlib: ... table of contents is empty`；要么给模块加实现，要么改成 INTERFACE（header-only）模块。
7. 顺手修小 bug：`Event.cpp:3` 单例改函数内静态对象；`Application.cpp:118` 不可达 return；`Application.cpp:83` 用 `steady_clock` 计算真实帧时间（为 P1-11 提供帧号）；`Launch.cpp:12` 未用变量与失败返回码。

**验收**：注释代码 624 → < 50（`grep -rn "^\s*//\s*[a-zA-Z_].*[;{)]"`）；`ls Resources/Shaders` 无 `Test copy*`；`git ls-files` 无未编译 `.cpp`；全 TU 扫描仅剩白名单告警。

**预估**：1–1.5 天。

---

## G4 文档重建 —— 已完成

**目标**：新人 15 分钟跑起来；不会再犯「手工 Destroy」这类错。

**落地内容**

- 根 `README.md` 重写为使用说明（定位、环境要求、preset 构建/运行/测试、门禁脚本、目录结构、文档索引、已知限制），原路线图移至 `docs/ROADMAP.md`。
- 新增 `docs/ownership.md`：`Handle`（视图）与 `Scope`/容器（所有者）的规则、资源归属总表、生命周期注意事项（释放顺序、运行时重建、地址复用陷阱、dylib 边界）、新增资源检查清单、历史反例。
- 删除失配文档：`Components/README.md`（描述不存在的模板版 `MeshComponent<CubeParams>`）删除；`Scene/README.md` 移至 `docs/architecture/scene.md` 并加"历史设计草案，未完全实现"提示。
- `docs/TODO.md` 瘦身：P0/P1 已完成项折叠为一张带 commit 与验证方式的表（419 → 341 行），保留未完成项的完整施工说明；回归基线命令更新为 preset/ctest + 门禁脚本。

**步骤**

1. 根 `README.md` 改为使用说明（定位、目录结构、依赖与获取、构建/运行/测试命令、当前状态），把现有路线图移到 `docs/ROADMAP.md`。
2. 新增 `docs/ownership.md`（**本组最重要的产出**）：`Handle` = 非拥有视图（无 `Destroy`）、`Scope`/容器 = 唯一所有者、`ViewOf`/`ViewAs` 用法、各资源归属表（引用 `docs/TODO.md` P1-1）、反例（为什么删除 `Handle::Destroy()`、为什么 dylib 边界两侧不能依赖单例 → P1-12）。
3. 处理失配文档：`Include/Runtime/RenderCore/Components/README.md`（描述已删除的模板版 `MeshComponent<CubeParams>`）、`Include/Runtime/RenderCore/Scene/README.md`（24KB，与现状不符）→ 有价值内容并入 `docs/architecture/scene.md`，其余删除。
4. `docs/TODO.md` 瘦身：已完成项归档。

**验收**：干净环境按 README 从零构建成功（用新用户或容器验证）；`grep -rn "MeshComponent<" docs Include` 无失效引用。

**预估**：1 天。

---

## G5 构建与依赖卫生 —— 已完成

**目标**：依赖可复现、模块依赖真实、SDK 定位不写死。

**落地内容**

- 删除对 CMake 内建 `FetchContent_Declare` 宏的全局覆盖（会污染第三方项目内部的同名调用），改为显式的 `${CHOZO_GITHUB_MIRROR}` 前缀变量：默认直连 GitHub，`-DCHOZO_GITHUB_MIRROR=https://ghfast.top/https://github.com` 即可走镜像。
- ImGui 从移动分支 `GIT_TAG docking` 固定为 commit `2af6dd96`（2026-06-04），并注明升级流程。
- Vulkan 定位改为**由 loader 推导**：`CMake/FindDependencies.cmake` 从 `Vulkan_LIBRARY` 取出目录，再据此查找 `MoltenVK_icd.json`，不再假设 `${Vulkan_INCLUDE_DIR}/../Lib` 这类 SDK 布局（macOS 只因大小写不敏感才碰巧能用）。`Launch/CMakeLists.txt` 的 bundle 拷贝改为构建期、按存在性 staging + 可读日志，缺文件只提示不失败。
- `LINK` 声明补全真实依赖：`RenderCore → CZRHI`，`App → CZRHI CZRenderCore CZWindow`。
- **完整构建并入主 CI**（`ci.yml` 的 `full-macos` job）：用 Homebrew 的 `molten-vk`/`vulkan-headers`/`vulkan-loader` + `VULKAN_SDK=/opt/homebrew`，不再需要 LunarG 安装器（实测 `install_vulkan.py` 并非可脚本化的 Qt 安装器）；`full-build.yml` 已删除。
- `External/stb` **决策：保留**。它是第三方 vendored 代码、当前不在构建中也不产生成本；等贴图导入落地时再接入（或届时删除）。

**步骤**

1. 去掉对 `FetchContent_Declare` 的全局覆盖（`CMake/FetchDependencies.cmake:12`）：会污染所有第三方项目内部的同名调用；改为显式包装 `chozo_fetch(...)`。
2. 镜像可选化：`CHOZO_GITHUB_MIRROR` 变量 + 默认直连；README 说明国内开启方式。
3. 依赖版本固定：ImGui 从移动分支 `GIT_TAG docking`（`:101`）改为固定 commit。
4. Vulkan 定位改造：删除 `${Vulkan_INCLUDE_DIR}/../Lib`、`../share/vulkan/icd.d` 这类 SDK 布局假设（Windows 是小写 `lib`），改用 `find_package(Vulkan)` 目标 + `VULKAN_SDK` 环境变量；`Launch/CMakeLists.txt` 的 loader/MoltenVK/ICD 拷贝步骤在文件缺失时给出可读错误而不是静默失败。
5. `LINK` 补全：`RenderCore` → `CZRHI`；`App` → `CZRHI CZRenderCore CZWindow`；让顶层聚合库退化为别名而不是依赖来源。
6. `External/stb` 二选一：接入（按 P1-5/P1-6 约定改造）或删除。
7. 格式与告警基线：新增 `.clang-format-ignore`（排除 `External/`）、格式化 3 个不合规文件、固化告警白名单。

**验收**：无镜像/断网环境可配置；`cmake --fresh` 后构建通过；`grep -c ghfast CMake/*.cmake` 为 0（除非显式开启）。

**预估**：1–1.5 天。**风险**：改 FetchContent 后需要一次 `rm -rf build/_deps`。

---

## G6 分层解耦

**目标**：Editor 只通过公共接口与后端/平台交互，为 README Phase 4 的 `IUIRenderBackend` 铺路。

**步骤**

1. 定义抽象：`IUIRenderBackend`（`Init/NewFrame/Draw/RegisterTexture/ReleaseTexture/Shutdown`）；把 SDL 专属的 `SDLWindowObj::SetEventPreprocessor` 提升为 `WindowObj` 的通用接口。
2. 把 `VulkanImGuiRenderer` 从 Editor 搬到后端/UI 模块（它本质是 Vulkan 的 ImGui 适配层）。
3. `EditorLayer` 去掉 SDL 内部头，改用窗口抽象提供的原生句柄 + 图形上下文。
4. 与 P1-12（核心库单例分裂）一起排期：接口注册表若仍是每镜像一份就没意义。

**验收**：`grep -rn "Backend/Vulkan\|SDLWindowObj\|SDL3/" Source/Editor` 为空。

**预估**：1.5–2 天。**风险**：本组是唯一有真实返工风险的项（接口设计不当会牵动后续 UI 工作）；先只做「搬家 + 薄接口」，不顺手重写 ImGui 集成。

---

## G7 跨平台 —— 进行中

**平台决策（2026-09-28）**：**Windows 优先，macOS 其次，Linux 先占位。**

**已完成（平台可移植性改造）**

- **模块名不再硬编码**：新增 `Include/Core/DynamicLibrary/ModuleNames.hpp`，`Launch`（编辑器模块）与 `GraphicsContext`（图形后端模块）按平台取文件名（`CZVulkan.dll` / `libCZVulkan.dylib` / `libCZVulkan.so`），不再写死 `.dylib`。
- **平台层**：新增 `Platform/Windows/WindowsFile.cpp`（`GetModuleFileNameW`）与 `Platform/Linux/LinuxFile.cpp`（`/proc/self/exe`）实现 `Platform::File::GetExecutablePath()`，`Platform.h` 按平台包含对应头文件；macOS 实现保持原样。
- **Vulkan 平台宏**：`VulkanPCH.h` 为 Windows 定义 `VK_USE_PLATFORM_WIN32_KHR` 并包含 `<windows.h>`；Linux 的 surface 创建改为显式报错（"not implemented yet (XCB/Wayland)"），不再静默失败。
- **构建布局按平台分支**：macOS 仍是 `.app` bundle；Windows/Linux 输出到 `dist/<config>/bin`，资源复制到 `dist/<config>/Resources`（与引擎"相对可执行文件上级目录找 Resources"的约定一致）。`-rdynamic`/rpath 只在 Apple/UNIX 生效；Windows 打开 `CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS`（模块按名加载，DLL 必须导出符号）。
- **CI 增加 `windows-core` job**（`windows-latest` + MSVC，跑 `core-debug` 预设与 `ctest`）：不需要 Vulkan SDK，先验证模块系统、PCH、平台层与测试在 MSVC 下可用。告警门禁的脚本也已识别 MSVC 的 `warning C####:` 形式。

**仍待完成（Windows 转正）**

1. 让 `windows-core` 变绿并修 MSVC 报出的告警/错误（首次运行大概率需要一轮修正）。
2. Windows **完整构建**：CI 安装 Vulkan SDK（`choco install vulkan-sdk` 或 LunarG Windows ZIP + `VULKAN_SDK`），跑 `full-*` 预设；验证 `vkCreateWin32SurfaceKHR` 分支（代码已存在但从未在 Windows 上执行）。
3. Windows 的运行时装配：DLL 与 `Launch.exe` 同目录（已由输出目录保证）、`SDL3.dll`/`slang.dll`/`ChozoImGui.dll` 等依赖的拷贝步骤、以及 `Launch` 加载 `CZVulkan.dll` 的路径解析。
4. Linux 占位转正需要：XCB/Wayland surface、`Platform/Linux` 的窗口属性对接、CI 的 `libvulkan-dev`/X11 依赖，以及把 `windows-core` 的 Linux 对应 job 扩到完整构建。
5. 平台验证矩阵：目前只有 macOS 被实机验证；Windows/Linux 的结论必须来自 CI。

**预估**：Windows 转正 1–2 天（含一轮 CI 迭代）；Linux 转正 2–3 天。

## G8 渲染/着色器契约 —— 部分完成

**已完成**：`CommandListObj::Draw(Scene, Camera)` 的静默失败改为"每个条件只告警一次"（缺 set 0、相机未注册、空场景、未绑定管线），并跳过未上传的网格。

| 项 | 方案 |
|---|---|
| 顶点属性告警（P1-10 遗留，**暂不修**） | `Basic.slang:12-16` 声明 5 个属性只消费 1 个；推荐裁剪 `VSInput` 到实际使用字段，等法线/UV 真正用于光照时再随功能加回。备选：在着色器中使用它们（跟 PBR 一起做） |
| ~~静默不绑定~~ | 已完成（见上） |
| 默认纹理兜底 | 管线 Set 0 未绑定时提供「默认黑纹理 + 默认采样器」（`VulkanImGuiRenderer.cpp` 里被注释掉的 `m_DefaultBlackTexture` 已表达此意图） |
| 契约检查 | 启动时比对反射输入与网格顶点布局，不一致即报错，避免「声明了但没消费」「命名不一致」再次静默通过 |

**预估**：0.5 天。

---

## 建议排期

| 批次 | 内容 | 产出 | 预估 |
|---|---|---|---|
| **Batch 1**（织网）—— 已完成 | G1 + G2 | CI 红灯即拒；用例 13 → 61；三配置全绿 | 2–2.5 天 |
| **Batch 2**（清债 + 留档）—— 已完成 | G3 + G4 + G5 | 注释代码 624 → 86；README/所有权文档就位；依赖可复现；`full-macos` 已并入主 CI | 3–3.5 天 |
| **Batch 3**（结构性） | G6 + G2 剩余（RenderCore 无 GPU 测试）+ G7（决策 + Linux 编译门禁）+ G8 | Editor 无后端依赖；Linux 可编译；契约检查到位 | 3–4 天 |

**依赖关系**：G1 是其余一切的前提（没有 CI，「清理」无法验证）；G6 必须与 P1-12 一起排期；G7 必须在 G1、P1-7 之后。

## 不建议现在做 / 建议推迟

1. **Linux/Windows 全功能支持**：CI 与平台抽象成型前收益低。
2. **自绘 UI 库（Clay/Nuklear 路线）**：属产品级投入，先完成 `IUIRenderBackend` 抽象。
3. **大范围重命名/目录重组**：与 P1-12 的构建结构调整冲突，等其决策后再做。
4. **一次性重排全部代码格式**：污染 git blame；只处理改动文件 + 3 个不合规文件。

## 与 P1 的联动

| P1 | 被哪组推进 |
|---|---|
| P1-9 断言开关写反 | G1/G2：Debug+Release 双配置 + 测试后立刻暴露 |
| P1-7 JobSystem 可移植 | G7：Linux 门禁直接逼出 `pthread_threadid_np` |
| P1-8 `Result` 重写 | G2：先有错误路径测试与 ASan 才敢改 |
| P1-2 缓存淘汰 | G2：需要单测固定行为；G8 的默认纹理会参与缓存键 |
| P1-6 线程契约 | G2：`TypeRegistry`/`VFS` 用例即回归网；G4 的文档应含线程契约一节 |
| P1-11 帧延迟删除 | G3：真实帧时间后才可用帧号；G6 的抽象层是退休队列落点 |
| P1-12 单例分裂 | G5/G6：构建结构调整与 UI 后端抽象必须一起做 |

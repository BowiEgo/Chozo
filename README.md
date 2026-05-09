# Engine Refactoring Roadmap

A comprehensive, phased plan to modernize the engine core, rendering, tools, and editor architecture.

---

## Phase 1: Foundation & Core Abstractions

- [ ] **Build System & CI/CD**
    - [ ] Migrate from xmake to CMake
        - [x] Top-level `CMakeLists.txt` using modern CMake style (`target_*` commands, PUBLIC/PRIVATE separation).
        - [x] Per-module `CMakeLists.txt` (e.g., `Core/`, `RHI/`, `UI/`).
        - [ ] Configure compilation options, feature flags (`option()`), and default build types.
        - [x] Integrate third-party libraries (glm, volk, Vulkan SDK, Tracy, slang, etc.) via `FetchContent` or `find_package`.
    - [ ] Set up CI/CD pipeline (GitHub Actions / GitLab CI)
        - [ ] Automated builds (Debug/Release) and unit tests.
        - [ ] Static code analysis (clang-tidy, cppcheck) and code formatting checks (clang-format).
        - [ ] Archive executables and debug symbols upon success.

- [ ] **Header File Reorganization**
    - [ ] Centralize public includes into `include/Engine/`, sub‑directories per module.
    - [ ] Define header classification conventions:
        - `Core/Defines.h` – global macros, platform detection, export macros.
        - `Core/Types.h` – fundamental type aliases (`int32`, `uint64`, ...).
        - `Core/STL.h` – STL extensions and allocator‑aware containers.
    - [ ] Each module exposes only a limited set of public headers; internal ones stay in `Source/`.

- [ ] **Handle‑Body Pattern & Entity Registry**
    - [x] Implement `Handle<T>` template.
    - [ ] Implement `EntityRegistry<T>` for cameras, components, resources, etc.

- [ ] **Core Utility Modules**
    - [ ] **Logging System**
        - [ ] Multi‑sink support (console, file, network, Tracy channel).
        - [ ] Compile‑time and runtime log level control.
        - [ ] Format library integration (e.g., fmt).
        - [ ] Macros: `ENGINE_LOG(Channel, Level, ...)`.
    - [ ] **File System Abstraction**
        - [ ] Cross‑platform path handling, file I/O, memory mapping.
        - [ ] Virtual file system with mount points for packaged assets.
    - [ ] **Command‑line Module**
        - [ ] Argument parser, reusable for the console later.
        - [ ] Registerable command callbacks.

- [ ] **Memory Management Foundation**
    - [x] Custom `new`/`delete` operators (centralized in `Core/Memory.h`).
    - [x] Implement **Linear Allocator** (per‑frame) and **Pool Allocator** (fixed‑size objects).
    - [ ] Tracy integration: instrument allocations, locks, rendering areas.

- [ ] **Application & Window Boundary Cleanup**
    - [ ] **Application module**: game loop, event pump, main thread scheduling.
    - [ ] **Window module**: window creation, input events collection (independent of rendering API).

---

## Phase 2: Rendering & Shader Modernization

- [ ] **Rendering Backend Refactor**
    - [ ] Proper split of synchronization primitives:
        - `Semaphore`, `Fence` as standalone objects (no more generic `SyncObject`).
    - [ ] Resource type clarity:
        - `Texture`, `Image`, `ImageView`, `Sampler` – each with distinct role.
    - [ ] Introduce `RenderPass` abstraction (backed by `vk::RenderPass` or dynamic rendering).
    - [ ] **Command buffer abstraction**:
        - Explicit `CommandPool` and `CommandList` lifecycle.
        - Standardized sequence: `Begin`, `Submit Queue`, `Draw*`, `End`.

- [ ] **Render API Top‑Level Rename**
    - [ ] `IRHIAPI` → `RenderServer` (a modern facade for the entire rendering backend).

- [ ] **Shading Language Migration**
    - [ ] Migrate all shaders from GLSL to **Slang**:
        - [ ] Configure Slang compiler toolchain to generate SPIR‑V.
        - [ ] Adapt existing shaders (skybox, PBR, post‑process).
        - [ ] Leverage Slang’s module system and interface capabilities.

- [ ] **RenderGraph 2.0**
    - [ ] Resource aliasing and automatic texture lifetime management.
    - [ ] Multi‑queue asynchronous compute + graphics scheduling.
    - [ ] Built‑in barrier generation based on resource state tracking.
    - [ ] Visual debug output: export of graph structure.

- [ ] **Screen Picking Component**
    - [ ] Compute‑based picking: ID render + readback via `PickingManager`.
    - [ ] Integration with editor object selection.

---

## Phase 3: Systems & Toolchain

- [ ] **Job System**
    - [ ] Thread‑based work‑stealing job system.
    - [ ] Support job dependencies and sync points.
    - [ ] Tracy‑level visualization of task scheduling.

- [ ] **Serializer Module**
    - [ ] Binary serialization/deserialization with versioning (forward/backward compatibility).
    - [ ] Optional lightweight compression (e.g., LZ4).

- [ ] **Data Structures & Algorithms Module (DSA)**
    - [ ] Custom containers: fixed‑capacity arrays, hash tables, graphs, etc.
    - [ ] Integrated with custom allocators.

- [ ] **ECS & Scene Optimization**
    - [ ] Manage component lifecycle via `EntityRegistry`.
    - [ ] Archetype‑based storage with component caching and dirty‑flag propagation.
    - [ ] System scheduler integrated with the Job System.
    - [ ] Scene graph rebuilt on top of ECS, with support for deferred loading and spatial partitioning.

- [ ] **Performance Profiling Upgrade**
    - [ ] Full Tracy coverage: frame markers, memory counters, GPU zones, thread activity.
    - [ ] In‑engine dashboard for live performance metrics.

---

## Phase 4: Editor, UI, and Ecosystem

- [ ] **Project Module**
    - [ ] Asset database, project settings, module registration.

- [ ] **Editor Core**
    - [ ] Viewport management and basic docking (initially with ImGui docking branch).
    - [ ] In‑editor console integrating the command‑line module.
    - [ ] Port existing editor features (content browser, gizmos) to new architecture.

- [ ] **UI Abstraction & Decoupling**
    - [ ] Define `IRHIWindow`, `IUIRenderBackend` interfaces to isolate ImGui.
    - [ ] Develop a prototype self‑built UI library (Clay + Nuklear style) as an alternative backend.
    - [ ] Keep the ImGui compatibility path operational to avoid blocking editor development.

- [ ] **Final Polish**
    - [ ] Documentation: coding standards, module dependency graph, architecture decision records (ADR).
    - [ ] Performance benchmarks comparing critical paths (rendering, loading) before and after refactoring.
    - [ ] Migration of remaining legacy tools/features.

---

## Estimated Timeline

Phase 1 ████████████░░░░░░░░░░░░░░░░░░░░ 3–5 weeks

Phase 2 ░░░░░░░░░░░░████████████████░░░░░ 4–7 weeks

Phase 3 ░░░░░░░░░░░░░░░░░░░░░░░░████████ 4–6 weeks

Phase 4 ░░░░░░░░░░░░░░░░░░░░░░░░░░░░░████ 6–8 weeks

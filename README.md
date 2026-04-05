### 🔧 Project Progress

- [ ] **Renderer**
    - [ ] **Render Graph (Architecture)**
        - [ ] DAG Builder & Pass Sorting
        - [ ] Automatic Barrier Insertion (Scheme 4/6)
        - [ ] Resource Aliasing (Memory Reuse)

    - [ ] **Scene Rendering**
        - [ ] Basic PBR Lighting
        - [ ] Mesh Loading (Assimp/gltf)

    - [ ] **Advanced Rendering Features**
        - [ ] Bindless material system integration.
        - [ ] Designed a global BindlessIndex system for textures.
        - [ ] Optimized ImGui integration to utilize bindless indexing, significantly reducing draw call overhead and descriptor set switching.
        - [ ] Shader reflection system for automated pipeline layout generation.
        - [ ] Support for compute-based post-processing effects.

- [ ] **RHI (Vulkan Implementation)**
    - [x] Device & Swapchain Initialization
    - [x] Descriptor Set Layout Cache
    - [x] Basic Synchronization (Layout Transitions)
    - [ ] Pipeline State Object (PSO) Caching

- [ ] **Editor**
    - [x] Viewport Render Output (Off-screen)

    - [ ] **Viewport Polish**
        - [ ] Window Resize Logic (FBE Recreations)
        - [ ] Aspect Ratio Correction

    - [ ] **Content Browser**

    - [ ] **Property Inspector (ImGui Integration)**

### 🏷️ Naming Conventions

| Prefix   | Full Name                 | Target Objects                                                   | Example                            |
| :------- | :------------------------ | :--------------------------------------------------------------- | :--------------------------------- |
| **`C`**  | **Class**                 | Complex objects with lifecycle, logic, or state.                 | `CRenderer`, `CScene`, `CCamera`   |
| **`F`**  | **Foundation / Fact**     | Plain C++ structs or helper classes (usually "value" types).     | `FVector3`, `FColor`, `FTransform` |
| **`U`**  | **Utility / UI**          | UI widgets, textures, or auxiliary resource types.               | `UButton`, `UTexture2D`, `UWidget` |
| **`I`**  | **Interface**             | Pure virtual classes (Abstract contracts).                       | `IRenderPass`, `IShaderResource`   |
| **`E`**  | **Enum**                  | Enumeration types.                                               | `EPixelFormat`, `EBufferUsage`     |
| **`T`**  | **Template**              | Template classes or smart pointer wrappers.                      | `TArray`, `TRef`, `TCallback`      |
| **`S`**  | **System**                | (Optional) Global manager/Singleton instances.                   | `SConfig`, `SFileSystem`           |
| **`b`**  | **Boolean Variables**     | Prefix with b                                                    | `bIsReady`, `bEnableValidation`    |
| **`m_`** | **Member Variables**      | Prefix with m\_                                                  | `m_LogicalDevice`                  |
| **`VK`** | **Vulkan Native Handles** | Use VK as a prefix or infix to distinguish from RHI abstractions | `m_VKImage`                        |
| **`k`**  | **Static Constants**      | Prefix with k                                                    | `kMaxFramesInFlight`               |

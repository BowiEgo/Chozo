# 所有权模型（Ownership）

> 本文档固定 `dev-0.1.x` 重构后确立的对象所有权约定。**新增代码必须遵守**，否则会重新引入本次重构修掉的那类缺陷（悬垂句柄、双重释放、静默泄漏）。
> 相关背景见 `docs/TODO.md` 的 P1-1（已完成）与 P1-12（dylib 边界单例分裂）。

## 一句话规则

**`Handle<T>` 只是非拥有视图；所有权由 `Scope<T>` 或容器持有。句柄不能释放对象。**

## 两种角色

### 1. 视图：`Handle<T>`（`Include/Core/Header/Handle.hpp`）

```cpp
template <typename TObject> class Handle {
    // 默认构造为 nullptr，可拷贝、可比较
    TObject* Get() const;          // 原始指针（仍是非拥有）
    TObject* operator->();
    explicit operator bool() const;
    // 注意：没有 Destroy()
};
```

- 生命周期**不**由句柄决定：持有者被销毁后，视图即悬垂（与裸指针同义）。
- 传参、存字段、放进 `std::vector`、作为 map 键（`HandleMap`）都可以。
- 需要长期保存视图时，必须保证被持有者的生命周期**严格长于**视图（例如：组件里的视图指向场景/注册表所拥有的对象）。

### 2. 所有者：`Scope<T>` 或容器

`Scope<T>` = `std::unique_ptr<T, DeleteDeleter>`（`DeleteDeleter` 调用 `Delete()` → 析构 + `HeapFree`，纳入内存统计）。

```cpp
struct MeshObj {
    // ...
private:
    Scope<MeshParamsObj>   m_Params;        // 拥有
    Scope<GraphicsBufferObj> m_VertexBuffer; // 拥有
    Scope<GraphicsBufferObj> m_IndexBuffer;  // 拥有
};
// 组件、节点、管理器只保存视图：
MeshParams GetParams() const { return ViewAs<MeshParams>(m_Params); }
```

从所有者派生视图：

```cpp
Handle<T>  ViewOf(const Scope<T>& owner);        // -> Handle<T>
TView      ViewAs<TView>(const Scope<T>& owner); // -> 具体句柄类型，如 Texture/GraphicsBuffer
```

容器持有：`AssetRegistry`（资产）、`DeviceObj`（采样器/布局/描述符集缓存）、`SwapchainObj`（纹理、信号量、栅栏）、`GraphicsContextObj`（设备、交换链）。

## 资源归属总表

| 对象 | 唯一所有者 |
|---|---|
| `ImageObj` | `TextureObj` |
| `TextureObj` | `FrameBufferObj`（附件）/ `SwapchainObj`（颜色附件） |
| `FrameBufferObj` / `SceneObj` / `CameraObj` | `ViewportObj` |
| `ViewportObj` / `CommandPoolObj` / `CommandListObj` / `PipelineObj` | `RendererObj` |
| `GraphicsBufferObj` | `MeshObj`（顶点/索引）、`CameraManager`（每个相机的 UBO） |
| `ShaderResObj` | `ShaderObj` |
| `SamplerObj` / `SetLayoutObj` / `DescriptorSetObj` | `DeviceObj` 缓存 |
| `DeviceObj` / `SwapchainObj` | `GraphicsContextObj` |
| `RHIAPIObj` | `RHIAPI` 单例 |
| `GraphicsContextObj` / `RendererObj` / `WindowObj` / `StartupHostObj` | `Engine` / `Application` |
| `MeshObj` / `ShaderObj` | `AssetRegistry`（`MeshRegistry` / `ShaderRegistry`） |
| `TransformParamsObj` / `MeshParamsObj` | `EditorNode`（以及 `MeshObj` 持有自己的那份生成参数） |
| `TransformComponent` 的平移/旋转/缩放 | 值语义，无堆对象 |

## 生命周期注意事项

1. **释放顺序**：后创建的对象先释放（成员声明顺序即释放的逆序）。`DeviceObj` 的缓存在 `ReleaseCachedResources()` 中于 `vkDestroyDevice` 之前释放；`GraphicsContextObj` 在销毁 instance 之前先释放 device/swapchain。
2. **运行时重建**（视口 resize、网格重传）：先 `WaitIdle()`，再 `reset()` 旧所有者，最后创建新的。这样可避免销毁在飞帧仍在引用的资源。
3. **视图键"地址复用"陷阱**：把视图当 map 键时，不要假设地址唯一——对象销毁后新对象可能复用同一地址。需要稳定身份时使用 `RHIResource::GetID()`（UUID，永不复用），例如 ImGui 纹理注册。
4. **dylib 边界**：核心模块目前同时链接进可执行文件与各 dylib，除 `CZMemory` 外的单例仍是每镜像一份（P1-12）。在修复前，不要跨 dylib 边界依赖任何单例状态（`Logger`、`TypeRegister`、`CameraManager`、`Application`…）。
5. **跨镜像分配/释放**：内存统计是进程级唯一的（`CZMemory` 为共享库）；不要新增直接 `new`/`delete` 绕过 `HEAP_MALLOC` 的对象（除标准容器内部使用 `StlAllocator` 的场景）。

## 新增资源类型的检查清单

- [ ] 工厂返回 `Scope<XObj>`（调用方拥有），或由设备/容器自持（返回视图）。
- [ ] 容器用 `Scope<XObj>` 成员，并在析构/`reset()` 中释放；不要在外层手工 `Delete`。
- [ ] 子资源在该对象的**析构函数**中释放（例如 `~TextureObj` 释放 image），不要依赖外部再调用一次 `Destroy()`。
- [ ] 需要跨帧存活时，明确写下"谁比谁活得久"，并在析构里做必要的注销/断链。
- [ ] 补一个单元测试（参考 `Source/Core/Header/Tests/HandleTest.cpp` 或 `TypeRegistryTest.cpp`）。

## 反例（都曾在仓库中出现过）

| 反例 | 后果 |
|---|---|
| `Params.Clone()` 之后对克隆体调用 `Destroy()` | 组件持有的指针立即悬垂（P0-2，use-after-free） |
| 视图当 map 键 + 地址复用 | 返回指向已销毁 image view 的描述符集（P1-10，校验层报错） |
| `AssetRegistry::Clear()` 先 `asset.Destroy()` 再由策略缓存释放 | 双重释放 |
| 句柄拷贝后任一处 `Destroy()` | 其它副本悬垂 |

## 变更历史

- `0c96f2b`：引擎数据（组件参数、相机、场景、资产注册表）改为单一所有者。
- `07d3bb2`：RHI 资源改为单一所有者（`Device`/`Swapchain`/`Renderer`/`Viewport`/`Mesh`/`Shader`），删除 `Handle::Destroy()` 与 `DEFINE_HANDLE_DESTROY`，`CZMemory` 改为共享库。

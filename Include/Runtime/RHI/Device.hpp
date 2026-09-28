#pragma once

#include <Core/Header/Handle.hpp>
#include <Core/Memory/Memory.hpp>
#include <Runtime/RHI/CommandPool.hpp>
#include <Runtime/RHI/DescriptorSet.hpp>
#include <Runtime/RHI/FrameBuffer.hpp>
#include <Runtime/RHI/GraphicsBuffer.hpp>
#include <Runtime/RHI/Pipeline.hpp>
#include <Runtime/RHI/Sampler.hpp>
#include <Runtime/RHI/SetLayout.hpp>
#include <Runtime/RHI/ShaderRes.hpp>

namespace CZ {

struct DescriptorSetKey {
    float LastFrame = 0; // For LRU eviction
    UUID LayoutID;
    std::vector<UUID> BindingResources; // Indexed by binding slot

    bool operator==(const DescriptorSetKey& other) const {
        return LayoutID == other.LayoutID && BindingResources == other.BindingResources;
    }
};
} // namespace CZ

namespace std {
template <> struct hash<CZ::DescriptorSetKey> {
    size_t operator()(const DescriptorSetKey& key) const {
        size_t h = 0;
        HashCombine(h, std::hash<UUID>{}(key.LayoutID));
        for (auto id : key.BindingResources)
            HashCombine(h, std::hash<UUID>{}(id));
        return h;
    }
};
} // namespace std

namespace CZ {

struct DeviceSpecification {
    // --- Metadata ---
    std::string AppName;
    uint32_t AppVersion;

    // --- Feature Toggles ---
    // [Note] High-level feature requests that RHI will try to fulfill
    // bool PreferIntegratedGPU = false; // Whether to use iGPU for power saving
    // bool RequireRayTracing   = false;
};

class DeviceObj {
    friend class Handle<DeviceObj>;

public:
    DeviceObj(const DeviceSpecification& spec) : m_Spec(spec) {}
    virtual ~DeviceObj() = default;

    virtual void WaitIdle() = 0;

    // --- Caller-owned resources (the caller keeps the returned Scope) ---
    virtual Scope<CommandPoolObj> CreateCommandPool(CommandPoolSpecification& spec) = 0;

    virtual Scope<FrameBufferObj> CreateFrameBuffer(const FrameBufferSpecification& spec) = 0;

    virtual Scope<ShaderResObj> CreateShaderRes(const ShaderResSpecification& spec,
                                                const std::vector<uint32_t>* binary) = 0;

    virtual Scope<PipelineObj> CreatePipeline(const PipelineSpecification& spec,
                                              const std::vector<ShaderRes>& shaders,
                                              const ShaderReflection& reflection) = 0;

    virtual Scope<GraphicsBufferObj> CreateGraphicsBuffer(const GraphicsBufferSpecification& spec,
                                                          const Buffer* initialData = nullptr) = 0;

    // --- Device-owned (cached) resources, handed out as views ---
    std::vector<SetLayout> CreateSetLayouts(
        const std::unordered_map<uint32_t, std::vector<ShaderResourceBinding>>& bindings);

    Sampler GetOrCreateSampler(const SamplerSpecification spec);

    DescriptorSet GetOrCreateDescriptorSet(SetLayout setLayout,
                                           std::vector<DescriptorBinding>& bindings);

protected:
    /// Releases every cached (device-owned) object. Backends must call this before destroying
    /// their native device, because the caches outlive the derived destructor body.
    void ReleaseCachedResources() {
        m_DescriptorSetCache.clear();
        m_SamplerCache.clear();
        m_SetLayoutCache.clear();
    }

    virtual Scope<SamplerObj> CreateSamplerImpl(const SamplerSpecification& spec) = 0;

    virtual Scope<SetLayoutObj> CreateSetLayoutImpl(const SetLayoutDescription& desc) = 0;

    virtual Scope<DescriptorSetObj>
        CreateDescriptorSetImpl(SetLayout setLayout, std::vector<DescriptorBinding>& bindings) = 0;

private:
    SetLayout GetOrCreateLayout(const std::vector<ShaderResourceBinding>& bindings);
    SetLayout GetEmptySetLayout();
    SetLayout GetStaticSetLayout();

protected:
    DeviceSpecification m_Spec;

    struct DescriptorSetEntry {
        Scope<DescriptorSetObj> Set;
        float LastFrame = 0.0f;
    };

    std::unordered_map<size_t, Scope<SetLayoutObj>> m_SetLayoutCache;
    std::unordered_map<SamplerSpecification, Scope<SamplerObj>> m_SamplerCache;
    std::unordered_map<DescriptorSetKey, DescriptorSetEntry> m_DescriptorSetCache;
};

struct Device : Handle<class DeviceObj> {
    template <typename T> T* As() { return static_cast<T*>(InternalHandleReader::Unwrap(*this)); }
};

} // namespace CZ

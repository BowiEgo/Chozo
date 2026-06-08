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

    virtual CommandPool CreateCommandPool(CommandPoolSpecification& spec) = 0;

    virtual Sampler CreateSampler(const SamplerSpecification spec) = 0;

    virtual FrameBuffer CreateFrameBuffer(const FrameBufferSpecification& spec) = 0;

    virtual ShaderRes CreateShaderRes(const ShaderResSpecification& spec,
                                      const std::vector<uint32_t>* binary) = 0;

    virtual Pipeline CreatePipeline(const PipelineSpecification& spec,
                                    const std::vector<ShaderRes>& shaders,
                                    const ShaderReflection& reflection) = 0;

    virtual SetLayout CreateSetLayout(const SetLayoutDescription& desc) = 0;

    virtual DescriptorSet CreateDescriptorSet(SetLayout setLayout,
                                              std::vector<DescriptorBinding>& bindings) = 0;

    virtual GraphicsBuffer CreateGraphicsBuffer(const GraphicsBufferSpecification& spec,
                                                const Buffer* initialData = nullptr) = 0;

    std::vector<SetLayout> CreateSetLayouts(
        const std::unordered_map<uint32_t, std::vector<ShaderResourceBinding>>& bindings);

    Sampler GetOrCreateSampler(const SamplerSpecification spec);

    DescriptorSet GetOrCreateDescriptorSet(SetLayout setLayout,
                                           std::vector<DescriptorBinding>& bindings);

private:
    SetLayout GetOrCreateLayout(const std::vector<ShaderResourceBinding>& bindings);
    SetLayout GetEmptySetLayout();
    SetLayout GetStaticSetLayout();

protected:
    DeviceSpecification m_Spec;

    std::unordered_map<size_t, SetLayout> m_SetLayoutCache;

    std::unordered_map<SamplerSpecification, Sampler> m_SamplerCache;
    SetLayout m_StaticSamplerLayout;

    std::unordered_map<DescriptorSetKey, DescriptorSet> m_DescriptorSetCache;
};

struct Device : Handle<class DeviceObj> {
    template <typename T> T* As() { return static_cast<T*>(InternalHandleReader::Unwrap(*this)); }
};

} // namespace CZ

#include <Runtime/RHI/Device.hpp>
#include <Runtime/RHI/RHIAPI.hpp>

namespace CZ {

template <> void Handle<DeviceObj>::Destroy() {
    if (m_Obj) {
        for (auto& [_, layout] : m_Obj->m_SetLayoutCache) {
            layout.Destroy();
        }
        m_Obj->m_SetLayoutCache.clear();

        m_Obj->m_StaticSamplerLayout.Destroy();

        for (auto& [_, sampler] : m_Obj->m_SamplerCache) {
            sampler.Destroy();
        }
        m_Obj->m_SamplerCache.clear();

        for (auto& [_, desc] : m_Obj->m_DescriptorSetCache) {
            desc.Destroy();
        }
        m_Obj->m_DescriptorSetCache.clear();

        Delete(m_Obj);
        m_Obj = nullptr;
    }
}

std::vector<SetLayout> DeviceObj::CreateSetLayouts(
    const std::unordered_map<uint32_t, std::vector<ShaderResourceBinding>>& bindings) {
    std::vector<SetLayout> result;

    if (bindings.empty()) return result;

    uint32_t maxSet = 0;
    for (auto const& [setIndex, _] : bindings) {
        maxSet = std::max(maxSet, setIndex);
    }

    result.resize(maxSet + 1);

    for (uint32_t i = 0; i <= maxSet; ++i) {
        if (bindings.contains(i)) {
            auto rhiLayout = GetOrCreateLayout(bindings.at(i));
            result[i]      = rhiLayout;
        } else {
            result[i] = GetEmptySetLayout();
        }
    }

    return result;
}

SetLayout DeviceObj::GetOrCreateLayout(const std::vector<ShaderResourceBinding>& bindings) {
    SetLayoutDescription desc;
    desc.Bindings = bindings;

    size_t hash = desc.GetHash();

    if (m_SetLayoutCache.contains(hash)) {
        return m_SetLayoutCache[hash];
    }

    SetLayout newLayout    = CreateSetLayout(desc);
    m_SetLayoutCache[hash] = newLayout;

    return newLayout;
}

SetLayout DeviceObj::GetEmptySetLayout() {
    SetLayoutDescription emptyDesc;
    emptyDesc.Bindings = {};

    size_t emptyHash = emptyDesc.GetHash();

    if (m_SetLayoutCache.contains(emptyHash)) {
        return m_SetLayoutCache[emptyHash];
    }

    CZ_RHI_LOG(Info, "Creating global Empty Descriptor Set Layout.");

    SetLayout emptyLayout       = CreateSetLayout({});
    m_SetLayoutCache[emptyHash] = emptyLayout;

    return emptyLayout;
}

SetLayout DeviceObj::GetStaticSetLayout() {
    SetLayoutDescription desc;
    desc.AddBinding(0, UniformType::CombinedImageSampler, 1, ShaderStage::Fragment);

    size_t hash = desc.GetHash();

    if (m_SetLayoutCache.contains(hash)) {
        return m_SetLayoutCache[hash];
    }

    CZ_RHI_LOG(Info, "Creating global Static Descriptor Set Layout.");

    SetLayout staticLayout = CreateSetLayout(desc);
    m_SetLayoutCache[hash] = staticLayout;

    return staticLayout;
}

Sampler DeviceObj::GetOrCreateSampler(const SamplerSpecification spec) {
    auto it = m_SamplerCache.find(spec);
    if (it != m_SamplerCache.end()) {
        return it->second;
    }

    Sampler sampler      = CreateSampler(spec);
    m_SamplerCache[spec] = sampler;

    return sampler;
}

DescriptorSet DeviceObj::GetOrCreateDescriptorSet(SetLayout setLayout,
                                                  std::vector<DescriptorBinding>& bindings) {
    DescriptorSetKey key;
    key.LayoutID = setLayout->GetID();
    key.BindingResources.resize(bindings.size() * 2);
    for (const auto& b : bindings) {
        if (b.m_Buffer) key.BindingResources.push_back(b.m_Buffer->GetID());
        if (b.m_Texture) key.BindingResources.push_back(b.m_Texture->GetID());
    }

    auto it       = m_DescriptorSetCache.find(key);
    key.LastFrame = RHIAPI::Get()->GetGraphicsContext()->GetCurrentFrame();
    if (it != m_DescriptorSetCache.end()) return it->second;

    auto descSet              = CreateDescriptorSet(setLayout, bindings);
    m_DescriptorSetCache[key] = descSet;
    return descSet;
}

} // namespace CZ
#include <Runtime/RHI/Device.hpp>
#include <Runtime/RHI/RHIAPI.hpp>

namespace CZ {

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

    if (auto it = m_SetLayoutCache.find(hash); it != m_SetLayoutCache.end()) {
        return ViewAs<SetLayout>(it->second);
    }

    Scope<SetLayoutObj> newLayout = CreateSetLayoutImpl(desc);
    if (!newLayout) return SetLayout();

    SetLayout view         = ViewAs<SetLayout>(newLayout);
    m_SetLayoutCache[hash] = std::move(newLayout);

    return view;
}

SetLayout DeviceObj::GetEmptySetLayout() {
    SetLayoutDescription emptyDesc;
    emptyDesc.Bindings = {};

    size_t emptyHash = emptyDesc.GetHash();

    if (auto it = m_SetLayoutCache.find(emptyHash); it != m_SetLayoutCache.end()) {
        return ViewAs<SetLayout>(it->second);
    }

    CZ_RHI_LOG(Info, "Creating global Empty Descriptor Set Layout.");

    Scope<SetLayoutObj> emptyLayout = CreateSetLayoutImpl({});
    if (!emptyLayout) return SetLayout();

    SetLayout view              = ViewAs<SetLayout>(emptyLayout);
    m_SetLayoutCache[emptyHash] = std::move(emptyLayout);

    return view;
}

SetLayout DeviceObj::GetStaticSetLayout() {
    SetLayoutDescription desc;
    desc.AddBinding(0, UniformType::CombinedImageSampler, 1, ShaderStage::Fragment);

    size_t hash = desc.GetHash();

    if (auto it = m_SetLayoutCache.find(hash); it != m_SetLayoutCache.end()) {
        return ViewAs<SetLayout>(it->second);
    }

    CZ_RHI_LOG(Info, "Creating global Static Descriptor Set Layout.");

    Scope<SetLayoutObj> staticLayout = CreateSetLayoutImpl(desc);
    if (!staticLayout) return SetLayout();

    SetLayout view         = ViewAs<SetLayout>(staticLayout);
    m_SetLayoutCache[hash] = std::move(staticLayout);

    return view;
}

Sampler DeviceObj::GetOrCreateSampler(const SamplerSpecification spec) {
    if (auto it = m_SamplerCache.find(spec); it != m_SamplerCache.end()) {
        return ViewAs<Sampler>(it->second);
    }

    Scope<SamplerObj> sampler = CreateSamplerImpl(spec);
    if (!sampler) return Sampler();

    Sampler view         = ViewAs<Sampler>(sampler);
    m_SamplerCache[spec] = std::move(sampler);

    return view;
}

DescriptorSet DeviceObj::GetOrCreateDescriptorSet(SetLayout setLayout,
                                                  std::vector<DescriptorBinding>& bindings) {
    DescriptorSetKey key;
    key.LayoutID = setLayout->GetID();
    key.BindingResources.reserve(bindings.size() * 2);

    for (const auto& b : bindings) {
        if (b.m_Buffer) key.BindingResources.push_back(b.m_Buffer->GetID());
        if (b.m_Texture) key.BindingResources.push_back(b.m_Texture->GetID());
    }

    if (auto it = m_DescriptorSetCache.find(key); it != m_DescriptorSetCache.end()) {
        it->second.LastFrame = RHIAPI::Get()->GetGraphicsContext()->GetCurrentFrame();
        return ViewAs<DescriptorSet>(it->second.Set);
    }

    Scope<DescriptorSetObj> descSet = CreateDescriptorSetImpl(setLayout, bindings);
    if (!descSet) return DescriptorSet();

    DescriptorSet view = ViewAs<DescriptorSet>(descSet);

    DescriptorSetEntry entry;
    entry.Set                 = std::move(descSet);
    entry.LastFrame           = RHIAPI::Get()->GetGraphicsContext()->GetCurrentFrame();
    m_DescriptorSetCache[key] = std::move(entry);

    return view;
}

} // namespace CZ

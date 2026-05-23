#include <Runtime/RHI/Device.hpp>

namespace CZ {

DEFINE_HANDLE_DESTROY(DeviceObj)

std::vector<SetLayout> DeviceObj::CreateSetLayouts(
    const std::unordered_map<uint32_t, std::vector<ShaderResourceBinding>>& bindings) {
    std::vector<SetLayout> result;

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

    CZ_CORE_LOG(Info, "Creating global Empty Descriptor Set Layout.");

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

    CZ_CORE_LOG(Info, "Creating global Static Descriptor Set Layout.");

    SetLayout staticLayout = CreateSetLayout(desc);
    m_SetLayoutCache[hash] = staticLayout;

    return staticLayout;
}

} // namespace CZ
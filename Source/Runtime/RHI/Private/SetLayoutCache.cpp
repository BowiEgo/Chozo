#include "SetLayoutCache.h"

#include "RHIDevice.h"

DEFINE_LOG_CATEGORY(LogSetLayoutCache);

TRef<IRHISetLayout> CSetLayoutCache::GetOrCreateLayout(const FRHISetLayoutDescription& desc) {
    auto device = m_Device.lock();
    size_t hash = desc.GetHash();

    if (m_Cache.contains(hash)) {
        return m_Cache[hash];
    }

    TRef<IRHISetLayout> newLayout = device->CreateSetLayout(desc);
    m_Cache[hash]                 = newLayout;

    return newLayout;
}

TRef<IRHISetLayout> CSetLayoutCache::GetEmptySetLayout() {
    FRHISetLayoutDescription emptyDesc;
    emptyDesc.Bindings = {};

    size_t emptyHash = emptyDesc.GetHash();

    if (m_Cache.contains(emptyHash)) {
        return m_Cache[emptyHash];
    }

    CZ_LOG(LogSetLayoutCache, Info, "Creating global Empty Descriptor Set Layout.");

    TRef<IRHISetLayout> emptyLayout = m_Device.lock()->CreateSetLayout(emptyDesc);
    m_Cache[emptyHash]              = emptyLayout;

    return emptyLayout;
}

TRef<IRHISetLayout> CSetLayoutCache::GetStaticSetLayout() {
    FRHISetLayoutDescription desc;
    desc.AddBinding(0, EUniformType::CombinedImageSampler, 1, EShaderStage::Fragment);

    size_t hash = desc.GetHash();

    if (m_Cache.contains(hash)) {
        return m_Cache[hash];
    }

    CZ_LOG(LogSetLayoutCache, Info, "Creating global Static Descriptor Set Layout.");

    TRef<IRHISetLayout> staticLayout = m_Device.lock()->CreateSetLayout(desc);
    m_Cache[hash]                    = staticLayout;

    return staticLayout;
}

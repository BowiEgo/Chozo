#include "DescriptorSetCache.h"

#include "RHIDevice.h"

DEFINE_LOG_CATEGORY(LogDescriptorSetCache);

TRef<IRHIDescriptorSet> CDescriptorSetCache::GetOrCreateDescriptorSet(
    const FTextureDescriptorInfo& info, TRef<IRHISetLayout> setLayout, uint32 bindingSlot) {
    FDescriptorSetKey key{ (void*)info.Sampler.get(), (void*)info.Image.get(),
                           (void*)setLayout.get() };

    if (m_Cache.contains(key)) {
        return m_Cache[key];
    }

    m_Cache[key] = m_Device.lock()->CreateDescriptorSet(info, setLayout, bindingSlot);
    return m_Cache[key];
}

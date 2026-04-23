#include "DescriptorSetCache.h"

#include "RHIContext.h"
#include "RHIDevice.h"

DEFINE_LOG_CATEGORY(LogDescriptorSetCache);

// TRef<IRHIDescriptorSet> CDescriptorSetCache::GetOrCreateDescriptorSet(
//     const FTextureDescriptorInfo& info, TRef<IRHISetLayout> setLayout, uint32 bindingSlot) {
//     FDescriptorSetKey key{ (void*)info.Sampler.get(), (void*)info.Image, (void*)setLayout.get()
//     };

//     if (m_Cache.contains(key)) {
//         return m_Cache[key];
//     }

//     m_Cache[key] = m_Device.lock()->CreateDescriptorSet(info, setLayout, bindingSlot);
//     return m_Cache[key];
// }

TRef<IRHIDescriptorSet>
    CDescriptorSetCache::GetOrCreateDescriptorSet(TRef<IRHISetLayout> setLayout,
                                                  const std::vector<FDescriptorBinding>& bindings) {
    FDescriptorSetKey key;
    key.LayoutID = setLayout->GetID();
    key.BindingResources.resize(bindings.size() * 2);
    for (const auto& b : bindings) {
        key.BindingResources.push_back(b.Resource->GetID());
        if (b.Sampler) key.BindingResources.push_back(b.Sampler->GetID());
    }

    auto it       = m_Cache.find(key);
    key.LastFrame = m_Device.lock()->GetContext()->GetCurrentFrame();
    if (it != m_Cache.end()) return it->second;

    auto descSet = m_Device.lock()->CreateDescriptorSet(setLayout, bindings);
    m_Cache[key] = descSet;
    return descSet;
}

void CDescriptorSetCache::Trim() {
    float currentFrame = m_Device.lock()->GetContext()->GetCurrentFrame();

    for (auto it = m_Cache.begin(); it != m_Cache.end();) {
        if (currentFrame - it->first.LastFrame > 10) {
            it = m_Cache.erase(it);
        } else {
            ++it;
        }
    }
}

#pragma once

#include "RHIExport.h"

#include "RHIBuffer.h"
#include "RHIDescriptorSet.h"
#include "RHIImage.h"
#include "RHISampler.h"
#include "RHISetLayout.h"

DECLARE_LOG_CATEGORY_EXTERN(LogDescriptorSetCache, Info);

struct FDescriptorSetKey {
    float LastFrame = 0; // For LRU eviction
    FUUID LayoutID;
    std::vector<FUUID> BindingResources; // Indexed by binding slot

    bool operator==(const FDescriptorSetKey& other) const {
        return LayoutID == other.LayoutID && BindingResources == other.BindingResources;
    }
};

namespace std {
template <> struct hash<FDescriptorSetKey> {
    size_t operator()(const FDescriptorSetKey& key) const {
        size_t h = 0;
        HashCombine(h, std::hash<FUUID>{}(key.LayoutID));
        for (auto id : key.BindingResources)
            HashCombine(h, std::hash<FUUID>{}(id));
        return h;
    }
};
} // namespace std

class RHI_API CDescriptorSetCache {
public:
    CDescriptorSetCache(const WeakRef<IRHIDevice> device) : m_Device(device) {};
    ~CDescriptorSetCache() = default;

    // Get or create a descriptor set for the given resource (e.g., buffer, texture + sampler)
    TRef<IRHIDescriptorSet>
        GetOrCreateDescriptorSet(TRef<IRHISetLayout> setLayout,
                                 const std::vector<FDescriptorBinding>& bindings);

    void Trim();
    void Clear() { m_Cache.clear(); }

protected:
    WeakRef<IRHIDevice> m_Device;

    std::unordered_map<FDescriptorSetKey, TRef<IRHIDescriptorSet>> m_Cache;
};
#pragma once

#include "RHIExport.h"

#include "RHIDescriptorSet.h"
#include "RHIImage.h"
#include "RHISampler.h"
#include "RHISetLayout.h"

DECLARE_LOG_CATEGORY_EXTERN(LogDescriptorSetCache, Info);

struct FTextureDescriptorInfo {
    TRef<IRHISampler> Sampler;
    TRef<IRHIImage> Image;
    EImageLayout ImageLayout;

    bool operator==(const FTextureDescriptorInfo& other) const {
        return Sampler == other.Sampler && Image == other.Image && ImageLayout == other.ImageLayout;
    }
};

struct FDescriptorSetKey {
    void* Sampler;
    void* Image;
    void* ImageLayout;

    bool operator==(const FDescriptorSetKey& other) const {
        return Sampler == other.Sampler && Image == other.Image && ImageLayout == other.ImageLayout;
    }
};

namespace std {
template <> struct hash<FDescriptorSetKey> {
    size_t operator()(const FDescriptorSetKey& k) const {
        size_t h = 0;
        HashCombine(h, hash<void*>{}(k.Sampler));
        HashCombine(h, hash<void*>{}(k.Image));
        HashCombine(h, hash<void*>{}(k.ImageLayout));
        return h;
    }
};
} // namespace std

class CDescriptorSetCache {
public:
    CDescriptorSetCache(const WeakRef<IRHIDevice> device) : m_Device(device) {};
    ~CDescriptorSetCache() = default;

    // Get or create a descriptor set for the given resource (e.g., texture + sampler)
    TRef<IRHIDescriptorSet> GetOrCreateDescriptorSet(const FTextureDescriptorInfo& info,
                                                     TRef<IRHISetLayout> setLayout,
                                                     uint32 bindingSlot = 0);

    void Clear() { m_Cache.clear(); }

protected:
    WeakRef<IRHIDevice> m_Device;

    std::unordered_map<FDescriptorSetKey, TRef<IRHIDescriptorSet>> m_Cache;
};
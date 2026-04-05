#pragma once

#include "RHITexture.h"

DECLARE_LOG_CATEGORY_EXTERN(LogSamplerCache, Info);

class CSamplerCache {
public:
    CSamplerCache(WeakRef<IRHIDevice> device) : m_Device(device) {}
    ~CSamplerCache() = default;

    TRef<IRHISampler> GetOrCreateSampler(const FSamplerSpecification& spec);

    void Clear() {
        std::lock_guard<std::mutex> lock(m_Mutex);
        m_Cache.clear();
    }

private:
    WeakRef<IRHIDevice> m_Device;
    std::unordered_map<FSamplerSpecification, TRef<IRHISampler>> m_Cache;
    std::mutex m_Mutex; // Ensure thread safety as asset loading may happen in async threads
};

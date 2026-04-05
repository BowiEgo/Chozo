#include "SamplerCache.h"

#include "RHIDevice.h"

DEFINE_LOG_CATEGORY(LogSamplerCache);

TRef<IRHISampler> CSamplerCache::GetOrCreateSampler(const FSamplerSpecification& spec) {
    std::lock_guard<std::mutex> lock(m_Mutex);

    auto device = m_Device.lock();

    auto it = m_Cache.find(spec);
    if (it != m_Cache.end()) {
        return it->second;
    }

    TRef<IRHISampler> sampler = device->CreateSampler(spec);
    m_Cache[spec]             = sampler;

    CZ_LOG(LogSamplerCache, Info, "Created new cached sampler (Total: {})", m_Cache.size());
    return sampler;
}
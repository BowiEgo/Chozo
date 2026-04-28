#pragma once

#include "RHIExport.h"

#include "RHISetLayout.h"
#include "RHITypes.h"

DECLARE_LOG_CATEGORY_EXTERN(LogSetLayoutCache, Info);

class RHI_API CSetLayoutCache {
public:
    CSetLayoutCache(const WeakRef<IRHIDevice> device) : m_Device(device) {}
    ~CSetLayoutCache() { CZ_LOG(LogSetLayoutCache, Trace, "SetLayoutCache destroying..."); }

    TRef<IRHISetLayout> GetOrCreateLayout(const FRHISetLayoutDescription& desc);
    TRef<IRHISetLayout> GetEmptySetLayout();
    TRef<IRHISetLayout> GetStaticSetLayout();

    void Clear() { m_Cache.clear(); }

private:
    WeakRef<IRHIDevice> m_Device;
    std::unordered_map<size_t, TRef<IRHISetLayout>> m_Cache;
    TRef<IRHISetLayout> m_StaticSamplerLayout;
};
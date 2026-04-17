#pragma once

#include "RHISampler.h"

#include "VulkanRHIExport.h"

class VULKAN_RHI_API CVulkanSampler : public IRHISampler {
public:
    CVulkanSampler(const WeakRef<IRHIDevice> device, const FSamplerSpecification& spec);
    virtual ~CVulkanSampler();

    const vk::Sampler GetVKHandle() const { return m_VKSampler; }

private:
    void CreateVKSampler();
    vk::Sampler m_VKSampler;
};
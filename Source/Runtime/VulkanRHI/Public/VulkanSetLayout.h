#pragma once

#include "VulkanRHIExport.h"

#include "RHISetLayout.h"
#include "RHITypes.h"

DECLARE_LOG_CATEGORY_EXTERN(LogVulkanSetLayout, Info);

class VULKAN_RHI_API CVulkanSetLayout : public IRHISetLayout {
public:
    CVulkanSetLayout(const WeakRef<IRHIDevice> device, const FRHISetLayoutDescription& desc);
    virtual ~CVulkanSetLayout();

    const vk::DescriptorSetLayout GetVKHandle() const { return *m_SetLayout; }

private:
    vk::raii::DescriptorSetLayout CreateVKSetLayout(const FRHISetLayoutDescription& desc);

    vk::raii::DescriptorSetLayout m_SetLayout;
};
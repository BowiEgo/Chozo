#pragma once

#include "RHIPipeline.h"
#include "VulkanRHIExport.h"

DECLARE_LOG_CATEGORY_EXTERN(LogVulkanRHIPipeline, Info);

class CVulkanRHIDevice;

class VULKAN_RHI_API CVulkanRHIPipeline : public IRHIPipeline {
    friend class CVulkanRHI;
    friend class CVulkanRHICommandBuffer;

public:
    CVulkanRHIPipeline(const FRHIPipelineCreateInfo& info, const TRef<CVulkanRHIDevice> device);
    virtual ~CVulkanRHIPipeline();

private:
    void Init();
    const vk::raii::Pipeline& GetVKPipeline() const { return m_Pipeline; }

private:
    WeakRef<CVulkanRHIDevice> m_Device;

    vk::raii::PipelineLayout m_PipelineLayout = nullptr;
    vk::raii::Pipeline m_Pipeline = nullptr;
};

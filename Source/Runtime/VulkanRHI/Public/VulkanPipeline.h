#pragma once

#include "RHIPipeline.h"

#include "VulkanRHIExport.h"

DECLARE_LOG_CATEGORY_EXTERN(LogVulkanPipeline, Info);

class CVulkanDevice;

class VULKAN_RHI_API CVulkanPipeline : public IRHIPipeline {
    friend class CVulkan;
    friend class CVulkanCommandBuffer;

public:
    CVulkanPipeline(const FPipelineSpecification& spec, const TRef<CVulkanDevice>& device);
    virtual ~CVulkanPipeline();

private:
    void Init();
    const vk::Pipeline GetVKPipeline() const { return *m_Pipeline; }
    const vk::raii::Pipeline& GetRAIIPipeline() const { return m_Pipeline; }

private:
    WeakRef<CVulkanDevice> m_Device;

    vk::raii::PipelineLayout m_PipelineLayout = nullptr;
    vk::raii::Pipeline m_Pipeline = nullptr;
};

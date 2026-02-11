#pragma once

#include "RHIPipeline.h"
#include "VulkanRHIExport.h"

DECLARE_LOG_CATEGORY_EXTERN(LogVulkanRHIPipeline, Info);

class CVulkanRHIDevice;

class VULKAN_RHI_API CVulkanRHIPipeline : public IRHIPipeline {
    friend class CVulkanGraphicsContext;

public:
    CVulkanRHIPipeline(const FRHIPipelineCreateInfo& info, const TRef<CVulkanRHIDevice> device);
    virtual ~CVulkanRHIPipeline();

    virtual void Bind() override {};

private:
    void Init();
    const vk::raii::Pipeline& GetVKPipeline() const { return m_Pipeline; }

private:
    WeakRef<CVulkanRHIDevice> m_Device;

    vk::raii::PipelineLayout m_PipelineLayout = nullptr;
    vk::raii::Pipeline m_Pipeline = nullptr;
};

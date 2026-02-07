#pragma once

#include "RHIPipeline.h"
#include "VulkanRHIExport.h"

DECLARE_LOG_CATEGORY_EXTERN(LogVulkanRHIPipeline, Info);

class CVulkanRHIDevice;

class VULKAN_RHI_API CVulkanRHIPipeline : public IRHIPipeline {
public:
    CVulkanRHIPipeline(const FRHIPipelineCreateInfo& info,
                       const TRef<CVulkanRHIDevice> device);
    virtual ~CVulkanRHIPipeline() = default;

    virtual void Bind() override {};

private:
    void Init();

private:
    TRef<CVulkanRHIDevice> m_Device;

    vk::raii::PipelineLayout m_PipelineLayout = nullptr;
    vk::raii::Pipeline m_GraphicsPipeline = nullptr;
};
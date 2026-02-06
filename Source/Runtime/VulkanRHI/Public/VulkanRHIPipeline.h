#pragma once

#include "RHIPipeline.h"
#include "VulkanRHIExport.h"

DECLARE_LOG_CATEGORY_EXTERN(LogVulkanRHIPipeline, Info);

class CVulkanRHIDevice;

class VULKAN_RHI_API CVulkanRHIPipeline : public IRHIPipeline {
public:
    CVulkanRHIPipeline(const FRHIPipelineCreateInfo& info);
    virtual ~CVulkanRHIPipeline() = default;

private:
};
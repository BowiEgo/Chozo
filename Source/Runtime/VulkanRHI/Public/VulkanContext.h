#pragma once

#include "RHIContext.h"

#include "VulkanRHIExport.h"

DECLARE_LOG_CATEGORY_EXTERN(LogVulkanContext, Info);

class VULKAN_RHI_API CVulkanContext : public IRHIContext {

public:
    CVulkanContext(const FContextSpec& spec);
    virtual ~CVulkanContext();
};

#pragma once

#include "RHIContext.h"

#include "VulkanExport.h"

DECLARE_LOG_CATEGORY_EXTERN(LogVulkanContext, Info);

class VULKAN_API CVulkanContext : public IRHIContext {

public:
    CVulkanContext(const FContextSpec& spec);
    virtual ~CVulkanContext();
};

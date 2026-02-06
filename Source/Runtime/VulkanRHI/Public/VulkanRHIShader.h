#pragma once

#include "RHIShader.h"
#include "VulkanRHIExport.h"

DECLARE_LOG_CATEGORY_EXTERN(LogVulkanRHIShader, Info);

class CVulkanRHIDevice;

class VULKAN_RHI_API CVulkanRHIShader : public IRHIShader {
public:
    CVulkanRHIShader(const FRHIShaderCreateInfo& info,
                     const CVulkanRHIDevice* device);
    virtual ~CVulkanRHIShader() = default;

private:
    vk::raii::ShaderModule m_Module = nullptr;
};
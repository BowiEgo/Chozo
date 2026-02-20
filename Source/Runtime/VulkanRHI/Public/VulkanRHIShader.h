#pragma once

#include "RHIShader.h"
#include "VulkanRHIExport.h"

DECLARE_LOG_CATEGORY_EXTERN(LogVulkanRHIShader, Info);

class CVulkanRHIDevice;

class VULKAN_RHI_API CVulkanRHIShader : public IRHIShader {
public:
    CVulkanRHIShader(const FRHIShaderCreateInfo& info, const std::vector<uint32_t>* binary,
                     const TRef<CVulkanRHIDevice>& device);
    virtual ~CVulkanRHIShader();

    const vk::ShaderModule GetModule() { return *m_Module; }

private:
    WeakRef<CVulkanRHIDevice> m_Device;

    vk::raii::ShaderModule m_Module = nullptr;
};
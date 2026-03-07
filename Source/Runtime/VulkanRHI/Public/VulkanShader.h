#pragma once

#include "RHIShader.h"

#include "VulkanRHIExport.h"

DECLARE_LOG_CATEGORY_EXTERN(LogVulkanShader, Info);

class CVulkanDevice;

class VULKAN_RHI_API CVulkanShader : public IRHIShader {
public:
    CVulkanShader(const FShaderSpecification& spec, const TRef<CVulkanDevice>& device,
                  const std::vector<uint32_t>* binary);
    virtual ~CVulkanShader();

    const vk::ShaderModule GetModule() { return *m_Module; }

private:
    WeakRef<CVulkanDevice> m_Device;

    vk::raii::ShaderModule m_Module = nullptr;
};
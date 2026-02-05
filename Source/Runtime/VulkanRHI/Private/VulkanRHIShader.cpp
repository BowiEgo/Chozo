#include "VulkanRHIShader.h"
#include "VulkanRHIDevice.h"

DEFINE_LOG_CATEGORY(LogVulkanRHIShader);

CVulkanRHIShader::CVulkanRHIShader(const FRHIShaderCreateInfo& info)
    : IRHIShader(info) {

    auto* deviceImpl = static_cast<const CVulkanRHIDevice*>(info.Device.Raw());
    if (!deviceImpl) {
        CZ_LOG(LogVulkanRHIShader, Error,
               "Invalid Device handle for shader creation");
        return;
    }

    vk::ShaderModuleCreateInfo mkInfo{};
    mkInfo.codeSize = info.Binary->size() * sizeof(uint32_t);
    mkInfo.pCode = info.Binary->data();

    try {
        m_Module = deviceImpl->GetVKDevice().createShaderModule(mkInfo);

        CZ_LOG(LogVulkanRHIShader, Info, "Vulkan Shader Module created: {0}",
               info.Name);
    } catch (const std::exception& e) {
        CZ_LOG(LogVulkanRHIShader, Error,
               "Failed to create Vulkan shader module: {0}", e.what());
    }
}

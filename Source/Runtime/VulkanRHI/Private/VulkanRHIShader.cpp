#include "VulkanRHIShader.h"
#include "VulkanRHIDevice.h"

DEFINE_LOG_CATEGORY(LogVulkanRHIShader);

CVulkanRHIShader::CVulkanRHIShader(const FRHIShaderCreateInfo& info,
                                   const std::vector<uint32_t>* binary,
                                   const TRef<CVulkanRHIDevice>& device)
    : IRHIShader(info), m_Device(device) {

    if (!device) {
        CZ_LOG(LogVulkanRHIShader, Error, "Invalid Device handle for shader creation");
        return;
    }

    const vk::raii::Device& raiiDevice = device->GetRAIILogicalDevice();

    vk::ShaderModuleCreateInfo mkInfo{};
    mkInfo.codeSize = binary->size() * sizeof(uint32_t);
    mkInfo.pCode = binary->data();

    try {
        m_Module = raiiDevice.createShaderModule(mkInfo);

        CZ_LOG(LogVulkanRHIShader, Info, "Vulkan Shader Module created: {0}", info.Name);
    } catch (const std::exception& e) {
        CZ_LOG(LogVulkanRHIShader, Error, "Failed to create Vulkan shader module: {0}", e.what());
    }
}

CVulkanRHIShader::~CVulkanRHIShader() {}

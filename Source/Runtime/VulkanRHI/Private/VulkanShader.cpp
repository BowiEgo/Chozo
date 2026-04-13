#include "VulkanShader.h"

#include "VulkanDevice.h"

DEFINE_LOG_CATEGORY(LogVulkanShader);

CVulkanShader::CVulkanShader(const FRHIShaderSpecification& spec, const TRef<CVulkanDevice>& device,
                             const std::vector<uint32_t>* binary,
                             const FShaderReflection reflection)
    : IRHIShader(spec, reflection), m_Device(device) {

    if (!device) {
        CZ_LOG(LogVulkanShader, Error, "Invalid Device handle for shader creation");
        return;
    }

    const vk::raii::Device& raiiDevice = device->GetRAIILogicalDevice();

    vk::ShaderModuleCreateInfo createInfo{};
    createInfo.codeSize = binary->size() * sizeof(uint32_t);
    createInfo.pCode    = binary->data();

    try {
        m_Module = raiiDevice.createShaderModule(createInfo);

        CZ_LOG(LogVulkanShader, Info, "Vulkan Shader Module created: {0}", spec.Name);
    } catch (const std::exception& e) {
        CZ_LOG(LogVulkanShader, Error, "Failed to create Vulkan shader module: {0}", e.what());
    }
}

CVulkanShader::~CVulkanShader() {}

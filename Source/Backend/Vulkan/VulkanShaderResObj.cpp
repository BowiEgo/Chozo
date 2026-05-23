#include "VulkanShaderResObj.hpp"
#include "VulkanDeviceObj.hpp"

#include <Runtime/RHI/Sampler.hpp>

namespace CZ {

DEFINE_LOG_CATEGORY_STATIC(LogVulkanShaderRes, Info);

VulkanShaderResObj::VulkanShaderResObj(const VulkanDeviceObj* deviceObj,
                                       const ShaderResSpecification& spec,
                                       const std::vector<uint32_t>* binary)
    : ShaderResObj(spec), m_DeviceObj(deviceObj) {
    Init(binary);
}

VulkanShaderResObj::~VulkanShaderResObj() {
    VkDevice logicalDevice = m_DeviceObj->GetLogicalDevice();

    if (m_VkModule != VK_NULL_HANDLE) {
        vkDestroyShaderModule(logicalDevice, m_VkModule, nullptr);
    }
}

void VulkanShaderResObj::Init(const std::vector<uint32_t>* binary) {
    if (!m_DeviceObj) {
        CZ_LOG(LogVulkanShaderRes, Error, "Device is no longer valid during Shader creation!");
        return;
    }

    VkDevice logicalDevice = m_DeviceObj->GetLogicalDevice();

    VkShaderModuleCreateInfo moduleInfo{};
    moduleInfo.sType    = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    moduleInfo.pCode    = binary->data();
    moduleInfo.codeSize = binary->size() * sizeof(uint32_t);

    VkShaderModule vkModule = VK_NULL_HANDLE;
    VkResult result         = vkCreateShaderModule(logicalDevice, &moduleInfo, nullptr, &vkModule);
    if (result != VK_SUCCESS) {
        CZ_LOG(LogVulkanShaderRes, Error, "Failed to create shader module: {}",
               VulkanUtils::VkResultToString(result));
        return;
    }
    m_VkModule = vkModule;
}

} // namespace CZ
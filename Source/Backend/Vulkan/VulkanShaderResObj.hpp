#pragma once

#include <Runtime/RHI/ShaderRes.hpp>
#include <vulkan/vulkan_core.h>

#ifndef VMA_IMPLEMENTATION
typedef struct VmaAllocation_T* VmaAllocation;
#endif

namespace CZ {

class VulkanDeviceObj;

class VulkanShaderResObj : public ShaderResObj {
public:
    VulkanShaderResObj(const VulkanDeviceObj* deviceObj, const ShaderResSpecification& spec,
                       const std::vector<uint32_t>* binary);
    ~VulkanShaderResObj() override;

    VkShaderStageFlagBits GetVkStage() const { return VulkanUtils::StageToFlagBits(m_Spec.Stage); }

    VkShaderModule GetVkShaderModule() const { return m_VkModule; }

private:
    void Init(const std::vector<uint32_t>* binary);

    const VulkanDeviceObj* m_DeviceObj;

    VkShaderModule m_VkModule = VK_NULL_HANDLE;
};

} // namespace CZ
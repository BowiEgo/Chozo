#pragma once

#include <Runtime/RHI/SetLayout.hpp>

namespace CZ {

class VulkanDeviceObj;

class VulkanSetLayoutObj : public SetLayoutObj {
public:
    VulkanSetLayoutObj(const VulkanDeviceObj* deviceObj, const SetLayoutDescription& desc);
    ~VulkanSetLayoutObj() override;

    static Result<VulkanSetLayoutObj*, VkResult> Create(VulkanDeviceObj* deviceObj,
                                                        const SetLayoutDescription& desc) {
        if (!deviceObj)
            return Result<VulkanSetLayoutObj*, VkResult>::Error(VK_ERROR_INITIALIZATION_FAILED);

        auto* obj = CZ_NEW(MEMORY_USAGE_RENDER, VulkanSetLayoutObj, deviceObj, desc);
        if (!obj) return Result<VulkanSetLayoutObj*, VkResult>::Error(VK_ERROR_OUT_OF_HOST_MEMORY);

        VkResult res = obj->Init();
        if (res != VK_SUCCESS) {
            Delete(obj);
            return Result<VulkanSetLayoutObj*, VkResult>::Error(res);
        }

        return Result<VulkanSetLayoutObj*, VkResult>::Success(obj);
    }

    bool Recreate();

    VkDescriptorSetLayout GetVkSetLayout() const { return m_VkSetLayout; }

private:
    VkResult Init();

    const VulkanDeviceObj* m_DeviceObj;
    const SetLayoutDescription m_Desc;

    VkDescriptorSetLayout m_VkSetLayout;
};
} // namespace CZ
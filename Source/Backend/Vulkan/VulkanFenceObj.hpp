#pragma once

#include <Runtime/RHI/Fence.hpp>
#include <vulkan/vulkan_core.h>

#include "VulkanDeviceObj.hpp"

namespace CZ {

class VulkanFenceObj : public FenceObj {
public:
    VulkanFenceObj(const VulkanDeviceObj* deviceObj) : m_DeviceObj((deviceObj)) {}

    ~VulkanFenceObj() override;

    static Result<VulkanFenceObj*, VkResult> Create(const VulkanDeviceObj* deviceObj) {
        if (!deviceObj)
            return Result<VulkanFenceObj*, VkResult>::Error(VK_ERROR_INITIALIZATION_FAILED);

        auto* obj = CZ_NEW(MEMORY_USAGE_RENDER, VulkanFenceObj, deviceObj);
        if (!obj) return Result<VulkanFenceObj*, VkResult>::Error(VK_ERROR_OUT_OF_HOST_MEMORY);

        VkResult res = obj->Init();
        if (res != VK_SUCCESS) {
            Delete(obj);
            return Result<VulkanFenceObj*, VkResult>::Error(res);
        }

        return Result<VulkanFenceObj*, VkResult>::Success(obj);
    }

    bool WaitAndReset(uint64_t timeout) const override;

    VkFence GetVKFence() const { return m_VkFence; }

private:
    VkResult Init();

    const VulkanDeviceObj* m_DeviceObj;

    VkFence m_VkFence;
};
} // namespace CZ
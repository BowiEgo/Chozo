#pragma once
#include <cstdint>

#include <Runtime/RHI/Fence.hpp>
#include <vulkan/vulkan_core.h>

#include "VulkanDeviceObj.hpp"

namespace CZ {

class VulkanFenceObj : public FenceObj {
public:
    VulkanFenceObj(const VulkanDeviceObj* deviceObj) : m_DeviceObj((deviceObj)) {}

    ~VulkanFenceObj() override;

    static Scope<VulkanFenceObj> Create(const VulkanDeviceObj* deviceObj) {
        if (!deviceObj) return nullptr;

        auto obj = CZ_CREATE_SCOPE(MEMORY_USAGE_RENDER, VulkanFenceObj, deviceObj);

        VkResult res = obj->Init();
        if (res != VK_SUCCESS) return nullptr;

        return obj;
    }

    bool WaitAndReset(uint64_t timeout) const override;

    VkFence GetVKFence() const { return m_VkFence; }

private:
    VkResult Init();

    const VulkanDeviceObj* m_DeviceObj;

    VkFence m_VkFence;
};
} // namespace CZ
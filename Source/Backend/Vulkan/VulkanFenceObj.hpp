#pragma once

#include <Runtime/RHI/Fence.hpp>

#include "VulkanDeviceObj.hpp"

namespace CZ {

class VulkanFenceObj : public FenceObj {
public:
    VulkanFenceObj(const VulkanDeviceObj* deviceObj);

    ~VulkanFenceObj() override;

    bool WaitAndReset(uint64_t timeout) const override;

    VkFence GetVKFence() const { return m_VkFence; }

private:
    const VulkanDeviceObj* m_DeviceObj;

    VkFence m_VkFence;
};
} // namespace CZ
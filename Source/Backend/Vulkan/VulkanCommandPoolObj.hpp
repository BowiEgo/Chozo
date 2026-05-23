#pragma once

#include <Runtime/RHI/CommandPool.hpp>

#include "VulkanCommandBufferObj.hpp"

namespace CZ {

class VulkanDeviceObj;

class VulkanCommandPoolObj : public CommandPoolObj {
    friend class VulkanCommandBufferObj;

public:
    VulkanCommandPoolObj(const VulkanDeviceObj* deviceObj, CommandPoolSpecification& spec);
    ~VulkanCommandPoolObj() override;

    CommandList AllocateCommandBuffer() override;

    VkCommandPool GetVkCommandPool() { return m_VkCommandPool; }

private:
    const VulkanDeviceObj* m_DeviceObj;

    VkCommandPool m_VkCommandPool = nullptr;
};

} // namespace CZ
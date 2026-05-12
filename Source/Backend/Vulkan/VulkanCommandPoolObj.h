#pragma once

#include "../Source/Runtime/RHI/CommandPoolObj.h"
#include <Runtime/RHI/CommandPool.h>
#include <vulkan/vulkan_core.h>

#include "VulkanCommandBufferObj.h"

namespace CZ {

class VulkanDeviceObj;

class VulkanCommandPoolObj : public CommandPoolObj {
    friend class VulkanCommandBufferObj;

public:
    VulkanCommandPoolObj(const VulkanDeviceObj* deviceObj, const CommandPoolSpecification& spec);
    ~VulkanCommandPoolObj() override;

    CommandList AllocateCommandBuffer() override;

    VkCommandPool GetVkCommandPool() { return m_VkCommandPool; }

private:
    const VulkanDeviceObj* m_DeviceObj;

    VkCommandPool m_VkCommandPool = nullptr;
};

} // namespace CZ
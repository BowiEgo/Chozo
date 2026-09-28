#pragma once

#include "VulkanCommandBufferObj.hpp"

#include <Core/Header/Result.hpp>
#include <Runtime/RHI/CommandPool.hpp>

#include <vulkan/vulkan_core.h>

namespace CZ {

class VulkanDeviceObj;

class VulkanCommandPoolObj : public CommandPoolObj {
    friend class VulkanCommandBufferObj;

public:
    VulkanCommandPoolObj(const VulkanDeviceObj* deviceObj, CommandPoolSpecification& spec)
        : CommandPoolObj(spec), m_DeviceObj(deviceObj) {}
    ~VulkanCommandPoolObj() override;

    static Scope<VulkanCommandPoolObj> Create(const VulkanDeviceObj* deviceObj,
                                              CommandPoolSpecification& spec) {
        if (!deviceObj) return nullptr;

        auto obj = CZ_CREATE_SCOPE(MEMORY_USAGE_RENDER, VulkanCommandPoolObj, deviceObj, spec);

        VkResult res = obj->Init();
        if (res != VK_SUCCESS) return nullptr;

        return obj;
    }

    Scope<CommandListObj> AllocateCommandBuffer() override;

    VkCommandPool GetVkCommandPool() { return m_VkCommandPool; }

private:
    VkResult Init();

    const VulkanDeviceObj* m_DeviceObj;

    VkCommandPool m_VkCommandPool = nullptr;
};

} // namespace CZ
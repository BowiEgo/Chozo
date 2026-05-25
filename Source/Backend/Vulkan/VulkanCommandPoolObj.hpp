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

    static Result<VulkanCommandPoolObj*, VkResult> Create(const VulkanDeviceObj* deviceObj,
                                                          CommandPoolSpecification& spec) {
        if (!deviceObj)
            return Result<VulkanCommandPoolObj*, VkResult>::Error(VK_ERROR_INITIALIZATION_FAILED);

        auto* obj = CZ_NEW(MEMORY_USAGE_RENDER, VulkanCommandPoolObj, deviceObj, spec);
        if (!obj)
            return Result<VulkanCommandPoolObj*, VkResult>::Error(VK_ERROR_OUT_OF_HOST_MEMORY);

        VkResult res = obj->Init();
        if (res != VK_SUCCESS) {
            Delete(obj);
            return Result<VulkanCommandPoolObj*, VkResult>::Error(res);
        }

        return Result<VulkanCommandPoolObj*, VkResult>::Success(obj);
    }

    CommandList AllocateCommandBuffer() override;

    VkCommandPool GetVkCommandPool() { return m_VkCommandPool; }

private:
    VkResult Init();

    const VulkanDeviceObj* m_DeviceObj;

    VkCommandPool m_VkCommandPool = nullptr;
};

} // namespace CZ
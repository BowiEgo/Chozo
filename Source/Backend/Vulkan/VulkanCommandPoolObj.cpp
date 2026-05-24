#include "VulkanCommandPoolObj.hpp"

#include "VulkanCommandBufferObj.hpp"
#include "VulkanDeviceObj.hpp"

#include <Core/Log/LogMacros.hpp>
#include <Core/Memory/MemoryTypes.hpp>
#include <Runtime/RHI/CommandList.hpp>

namespace CZ {

VulkanCommandPoolObj::VulkanCommandPoolObj(const VulkanDeviceObj* deviceObj,
                                           CommandPoolSpecification& spec)
    : CommandPoolObj(spec), m_DeviceObj(deviceObj) {

    if (!deviceObj) {
        CZ_BACKEND_LOG(Error, "Device is no longer valid during CommandPool creation!");
        return;
    }

    VkDevice logicalDevice = deviceObj->GetLogicalDevice();

    VkCommandPoolCreateInfo poolInfo{};
    poolInfo.sType            = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.flags            = VulkanUtils::MapCommandPoolFlags(m_Spec.Flags);
    poolInfo.queueFamilyIndex = m_Spec.QueueIndex;

    VkResult result = vkCreateCommandPool(logicalDevice, &poolInfo, nullptr, &m_VkCommandPool);
    if (result != VK_SUCCESS) {
        CZ_BACKEND_LOG(Error, "Create CommandPool failed: {}",
                       VulkanUtils::VkResultToString(result));
    }
}

VulkanCommandPoolObj::~VulkanCommandPoolObj() {
    if (m_VkCommandPool != VK_NULL_HANDLE) {
        VkDevice logicalDevice = m_DeviceObj->GetLogicalDevice();
        vkDestroyCommandPool(logicalDevice, m_VkCommandPool, nullptr);
        m_VkCommandPool = VK_NULL_HANDLE;
    }
}

CommandList VulkanCommandPoolObj::AllocateCommandBuffer() {
    return CommandList(CZ_NEW(MEMORY_USAGE_RENDER, VulkanCommandBufferObj, this));
}

} // namespace CZ
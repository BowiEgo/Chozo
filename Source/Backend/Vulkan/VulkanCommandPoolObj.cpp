#include "VulkanCommandPoolObj.hpp"

#include "VulkanCommandBufferObj.hpp"
#include "VulkanDeviceObj.hpp"

#include <Core/Log/LogMacros.hpp>
#include <Core/Memory/MemoryTypes.hpp>
#include <Runtime/RHI/CommandList.hpp>

namespace CZ {

VulkanCommandPoolObj::~VulkanCommandPoolObj() {
    if (m_VkCommandPool != VK_NULL_HANDLE) {
        VkDevice logicalDevice = m_DeviceObj->GetLogicalDevice();
        vkDestroyCommandPool(logicalDevice, m_VkCommandPool, nullptr);
        m_VkCommandPool = VK_NULL_HANDLE;
    }
}

CommandList VulkanCommandPoolObj::AllocateCommandBuffer() {
    auto result = VulkanCommandBufferObj::Create(this);
    if (result) return CommandList(result.value());
    return CommandList();
}

// ---- Private ----

VkResult VulkanCommandPoolObj::Init() {
    VkResult result;

    VkDevice logicalDevice = m_DeviceObj->GetLogicalDevice();

    VkCommandPoolCreateInfo poolInfo{};
    poolInfo.sType            = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.flags            = VulkanUtils::MapCommandPoolFlags(m_Spec.Flags);
    poolInfo.queueFamilyIndex = m_Spec.QueueIndex;

    result = vkCreateCommandPool(logicalDevice, &poolInfo, nullptr, &m_VkCommandPool);

    return result;
}

} // namespace CZ
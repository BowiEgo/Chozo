#include "VulkanFenceObj.hpp"

namespace CZ {

VulkanFenceObj::VulkanFenceObj(const VulkanDeviceObj* deviceObj) : m_DeviceObj((deviceObj)) {
    VkDevice logicalDevice = m_DeviceObj->GetLogicalDevice();

    VkFenceCreateInfo fenceInfo = {};
    fenceInfo.sType             = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.pNext             = nullptr;
    fenceInfo.flags             = VK_FENCE_CREATE_SIGNALED_BIT;

    if (vkCreateFence(logicalDevice, &fenceInfo, nullptr, &m_VkFence) != VK_SUCCESS) {
        CZ_BACKEND_LOG(Error, "Failed to create Vulkan fence!");
    }
}

VulkanFenceObj::~VulkanFenceObj() {
    VkDevice logicalDevice = m_DeviceObj->GetLogicalDevice();

    if (m_VkFence != VK_NULL_HANDLE) vkDestroyFence(logicalDevice, m_VkFence, nullptr);
}

bool VulkanFenceObj::WaitAndReset(uint64_t timeout) const {
    if (m_VkFence == VK_NULL_HANDLE) {
        CZ_BACKEND_LOG(Error, "Invalid fence in WaitAndResetFence");
        return VK_ERROR_INITIALIZATION_FAILED;
    }

    VkDevice logicalDevice = m_DeviceObj->GetLogicalDevice();

    VkResult result = vkWaitForFences(logicalDevice, 1, &m_VkFence, VK_TRUE, timeout);

    if (result == VK_SUCCESS) {
        vkResetFences(logicalDevice, 1, &m_VkFence);
        return true;
    } else if (result == VK_TIMEOUT) {
        CZ_BACKEND_LOG(Warning, "Fence wait timed out!");
    } else {
        CZ_BACKEND_LOG(Error, "Failed to wait for fence! Result: {}", (int)result);
    }

    return false;
}
} // namespace CZ
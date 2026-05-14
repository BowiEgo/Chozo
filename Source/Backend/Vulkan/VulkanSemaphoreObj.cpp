#include "VulkanSemaphoreObj.h"
#include "VulkanDeviceObj.h"
#include "VulkanUtils.h"

namespace CZ {

DEFINE_LOG_CATEGORY_STATIC(LogVulkanSemaphore, Info);

VulkanSemaphoreObj::VulkanSemaphoreObj(const VulkanDeviceObj* deviceObj)
    : m_DeviceObj((deviceObj)) {
    Recreate();
}

VulkanSemaphoreObj::~VulkanSemaphoreObj() {
    VkDevice logicalDevice = m_DeviceObj->GetLogicalDevice();

    if (m_Semaphore != VK_NULL_HANDLE) vkDestroySemaphore(logicalDevice, m_Semaphore, nullptr);
}

bool VulkanSemaphoreObj::Recreate() {
    VkDevice logicalDevice = m_DeviceObj->GetLogicalDevice();

    VkSemaphoreCreateInfo semaphoreInfo = {};
    semaphoreInfo.sType                 = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkResult result = vkCreateSemaphore(logicalDevice, &semaphoreInfo, nullptr, &m_Semaphore);

    if (result == VK_SUCCESS) {
        return true;
    } else {
        CZ_LOG(LogVulkanSemaphore, Error, "Failed to wait for semaphore! Result: {}",
               VulkanUtils::VkResultToString(result));
    }

    return false;
}

} // namespace CZ
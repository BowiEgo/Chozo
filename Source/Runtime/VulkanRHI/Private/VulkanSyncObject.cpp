#include "VulkanRHIDevice.h"
#include "VulkanRHISyncObject.h"

DEFINE_LOG_CATEGORY(LogVulkanRHISyncObject);

void CVulkanRHISyncObject::WaitAndResetFence(TRef<CVulkanRHIDevice> device) const {
    // Wait for the fence to be signaled before resetting it

    auto& logicalDevice = device->GetLogicalDevice();
    auto result = logicalDevice.waitForFences({*m_DrawFence}, vk::True, UINT64_MAX);
    if (result != vk::Result::eSuccess) {
        CZ_LOG(LogVulkanRHISyncObject, Error, "failed to wait for fence!");
    }
    logicalDevice.resetFences({*m_DrawFence});
}
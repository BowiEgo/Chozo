#include "VulkanRHIDevice.h"
#include "VulkanRHISyncObject.h"

DEFINE_LOG_CATEGORY(LogVulkanRHISyncObject);

void CVulkanRHISyncObject::WaitAndResetFence(const TRef<CVulkanRHIDevice>& device) const {
    // Wait for the fence to be signaled before resetting it
    auto& logicalDevice = device->GetLogicalDevice();
    auto result = logicalDevice.waitForFences({ *m_DrawFence }, vk::True, UINT64_MAX);
    if (result != vk::Result::eSuccess) {
        CZ_LOG(LogVulkanRHISyncObject, Error, "failed to wait for fence!");
    }
    logicalDevice.resetFences({ *m_DrawFence });
}

void CVulkanRHISyncObject::RecreateSemaphores(const TRef<CVulkanRHIDevice>& device) {
    const vk::raii::Device& raiiDevice = device->GetRAIILogicalDevice();

    vk::SemaphoreCreateInfo semaphoreInfo;
    m_PresentCompleteSemaphore = vk::raii::Semaphore(raiiDevice, semaphoreInfo);
    m_RenderFinishedSemaphore = vk::raii::Semaphore(raiiDevice, semaphoreInfo);

    CZ_LOG(LogVulkanRHISyncObject, Trace, "SyncObject semaphores recreated to clear pollution.");
}
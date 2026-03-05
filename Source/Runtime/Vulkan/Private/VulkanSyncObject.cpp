#include "VulkanSyncObject.h"

#include "VulkanDevice.h"

DEFINE_LOG_CATEGORY(LogVulkanSyncObject);

CVulkanSyncObject::CVulkanSyncObject(const TRef<CVulkanDevice>& device) {
    const vk::raii::Device& raiiDevice = device->GetRAIILogicalDevice();

    vk::SemaphoreCreateInfo semInfo;
    m_PresentCompleteSemaphore = vk::raii::Semaphore(raiiDevice, semInfo);
    m_RenderFinishedSemaphore = vk::raii::Semaphore(raiiDevice, semInfo);

    vk::FenceCreateInfo fenceInfo;
    fenceInfo.flags = vk::FenceCreateFlagBits::eSignaled;

    m_Fence = vk::raii::Fence(raiiDevice, fenceInfo);
}

void CVulkanSyncObject::WaitAndResetFence(const TRef<CVulkanDevice>& device) const {
    // Wait for the fence to be signaled before resetting it
    auto& logicalDevice = device->GetLogicalDevice();
    auto result = logicalDevice.waitForFences({ *m_Fence }, vk::True, UINT64_MAX);
    if (result != vk::Result::eSuccess) {
        CZ_LOG(LogVulkanSyncObject, Error, "failed to wait for fence!");
    }
    logicalDevice.resetFences({ *m_Fence });
}

void CVulkanSyncObject::RecreateSemaphores(const TRef<CVulkanDevice>& device) {
    const vk::raii::Device& raiiDevice = device->GetRAIILogicalDevice();

    vk::SemaphoreCreateInfo semaphoreInfo;
    m_PresentCompleteSemaphore = vk::raii::Semaphore(raiiDevice, semaphoreInfo);
    m_RenderFinishedSemaphore = vk::raii::Semaphore(raiiDevice, semaphoreInfo);

    CZ_LOG(LogVulkanSyncObject, Trace, "SyncObject semaphores recreated to clear pollution.");
}
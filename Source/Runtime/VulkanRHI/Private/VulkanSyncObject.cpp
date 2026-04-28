#include "VulkanSyncObject.h"

#include "VulkanDevice.h"

DEFINE_LOG_CATEGORY(LogVulkanSyncObject);

CVulkanSyncObject::CVulkanSyncObject(const TRef<CVulkanDevice>& device) {
    const vk::raii::Device& raiiDevice = device->GetRAIILogicalDevice();

    vk::SemaphoreCreateInfo semInfo{};
    m_PresentCompleteSemaphore = vk::raii::Semaphore(raiiDevice, semInfo);
    m_RenderFinishedSemaphore  = vk::raii::Semaphore(raiiDevice, semInfo);

    vk::FenceCreateInfo fenceInfo{};
    fenceInfo.flags = vk::FenceCreateFlagBits::eSignaled;

    m_Fence = vk::raii::Fence(raiiDevice, fenceInfo);
}

vk::Result CVulkanSyncObject::WaitAndResetFence(const TRef<CVulkanDevice>& device,
                                                uint64_t timeout) const {
    if (!*m_Fence) {
        CZ_LOG(LogVulkanSyncObject, Error, "Invalid fence in WaitAndResetFence");
        return vk::Result::eErrorInitializationFailed;
    }

    // Wait for the fence to be signaled before resetting it
    auto& logicalDevice = device->GetLogicalDevice();
    auto result         = logicalDevice.waitForFences({ *m_Fence }, vk::True, UINT64_MAX);

    if (result == vk::Result::eSuccess) {
        logicalDevice.resetFences({ *m_Fence });
    } else {
        CZ_LOG(LogVulkanSyncObject, Error, "failed to wait for fence!");
    }

    return result;
}

void CVulkanSyncObject::RecreateSemaphores(TRef<CVulkanDevice>& device) {
    device->WaitIdle();

    const vk::raii::Device& raiiDevice = device->GetRAIILogicalDevice();

    vk::SemaphoreCreateInfo semaphoreInfo{};
    m_PresentCompleteSemaphore = vk::raii::Semaphore(raiiDevice, semaphoreInfo);
    m_RenderFinishedSemaphore  = vk::raii::Semaphore(raiiDevice, semaphoreInfo);

    m_JustRecreated = true;

    CZ_LOG(LogVulkanSyncObject, Trace, "SyncObject semaphores recreated to clear pollution.");
}
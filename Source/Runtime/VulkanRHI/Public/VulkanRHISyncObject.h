#pragma once

#include "RHISyncObject.h"
#include "VulkanRHIExport.h"

DECLARE_LOG_CATEGORY_EXTERN(LogVulkanRHISyncObject, Info);

class CVulkanRHIDevice;

class VULKAN_RHI_API CVulkanRHISyncObject : public IRHISyncObject {
public:
    CVulkanRHISyncObject(const TRef<CVulkanRHIDevice>& device) {
        const vk::raii::Device& raiiDevice = device->GetRAIILogicalDevice();

        vk::SemaphoreCreateInfo semInfo;
        m_PresentCompleteSemaphore = vk::raii::Semaphore(raiiDevice, semInfo);
        m_RenderFinishedSemaphore = vk::raii::Semaphore(raiiDevice, semInfo);

        vk::FenceCreateInfo fenceInfo;
        fenceInfo.flags = vk::FenceCreateFlagBits::eSignaled;

        m_DrawFence = vk::raii::Fence(raiiDevice, fenceInfo);
    }
    virtual ~CVulkanRHISyncObject() = default;

    void WaitAndResetFence(TRef<CVulkanRHIDevice> device) const;

    const vk::Semaphore GetPresentCompleteSemaphore() const { return *m_PresentCompleteSemaphore; }
    const vk::Semaphore GetRenderFinishedSemaphore() const { return *m_RenderFinishedSemaphore; }
    const vk::Fence GetDrawFence() const { return *m_DrawFence; }

private:
    vk::raii::Semaphore m_PresentCompleteSemaphore = nullptr;
    vk::raii::Semaphore m_RenderFinishedSemaphore = nullptr;
    mutable vk::raii::Fence m_DrawFence = nullptr;
};

#pragma once

#include "RHISyncObject.h"
#include "VulkanRHIExport.h"

DECLARE_LOG_CATEGORY_EXTERN(LogVulkanRHISyncObject, Info);

class CVulkanRHIDevice;

class VULKAN_RHI_API CVulkanRHISyncObject : public IRHISyncObject {
public:
    CVulkanRHISyncObject(const TRef<CVulkanRHIDevice> device) {
        const vk::raii::Device& vkDevice = device->GetLogicalDevice();
        vk::SemaphoreCreateInfo semInfo;

        m_PresentCompleteSemaphore = vk::raii::Semaphore(vkDevice, semInfo);
        m_RenderFinishedSemaphore = vk::raii::Semaphore(vkDevice, semInfo);

        vk::FenceCreateInfo fenceInfo;
        fenceInfo.flags = vk::FenceCreateFlagBits::eSignaled;

        m_DrawFence = vk::raii::Fence(vkDevice, fenceInfo);
    }
    virtual ~CVulkanRHISyncObject() = default;

    const vk::raii::Semaphore& GetPresentCompleteSemaphore() const {
        return m_PresentCompleteSemaphore;
    }
    const vk::raii::Semaphore& GetRenderFinishedSemaphore() const {
        return m_RenderFinishedSemaphore;
    }
    const vk::raii::Fence& GetDrawFence() const { return m_DrawFence; }

private:
    vk::raii::Semaphore m_PresentCompleteSemaphore = nullptr;
    vk::raii::Semaphore m_RenderFinishedSemaphore = nullptr;
    vk::raii::Fence m_DrawFence = nullptr;
};

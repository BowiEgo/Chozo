#pragma once

#include "RHISyncObject.h"

#include "VulkanDevice.h"

#include "VulkanRHIExport.h"

DECLARE_LOG_CATEGORY_EXTERN(LogVulkanSyncObject, Info);

class CVulkanDevice;

class VULKAN_RHI_API CVulkanSyncObject : public IRHISyncObject {
public:
    CVulkanSyncObject(const TRef<CVulkanDevice>& device);
    virtual ~CVulkanSyncObject() = default;

    vk::Result WaitAndResetFence(const TRef<CVulkanDevice>& device, uint64_t timeout) const;
    void RecreateSemaphores(TRef<CVulkanDevice>& device);

    bool WasJustRecreated() const { return m_JustRecreated; }
    void ClearRecreatedFlag() { m_JustRecreated = false; }

    const vk::Semaphore GetPresentCompleteSemaphore() const { return *m_PresentCompleteSemaphore; }
    const vk::Semaphore GetRenderFinishedSemaphore() const { return *m_RenderFinishedSemaphore; }
    const vk::Fence GetVKFence() const { return *m_Fence; }

private:
    vk::raii::Semaphore m_PresentCompleteSemaphore = nullptr;
    vk::raii::Semaphore m_RenderFinishedSemaphore = nullptr;
    mutable vk::raii::Fence m_Fence = nullptr;

    bool m_JustRecreated;
};

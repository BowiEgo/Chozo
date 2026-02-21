#pragma once

#include "RHISwapchain.h"
#include "RHISyncObject.h"
#include "VulkanRHIDevice.h"
#include "VulkanRHIExport.h"
#include "VulkanRHITexture2D.h"
#include "VulkanUtils.h"

DECLARE_LOG_CATEGORY_EXTERN(LogVulkanRHISwapchain, Info);

static constexpr uint32 INVALID_IMAGE_INDEX = 0xFFFFFFFF;

class VULKAN_RHI_API CVulkanRHISwapchain : public IRHISwapchain {
public:
    CVulkanRHISwapchain(const FRHISwapchainSpecification& spec,
                        const TRef<CVulkanRHIDevice>& device, const vk::raii::SurfaceKHR& surface);
    virtual ~CVulkanRHISwapchain();

    virtual const EPixelFormat GetImageFormat() const override {
        return ChozoUtils::Vulkan::FromVKFormat(m_VKImageFormat);
    }
    virtual const EPixelFormat GetDepthFormat() const override {
        return ChozoUtils::Vulkan::FromVKFormat(m_VKDepthFormat);
    }
    virtual const FExtent2D GetExtent() const override {
        return FExtent2D(m_VKExtent.width, m_VKExtent.height);
    }

    virtual void SetPresentMode(const EPresentMode mode) override;
    virtual void RecreateSwapchain(const FExtent2D& frameBufferSize) override;

private:
    void Init();

public:
    bool RecreateSwapchainIfNeeded();
    void RecreateSwapchain() { RecreateSwapchain(m_Spec.FrameBufferSize); }

    void SetLayout(uint32_t index, vk::ImageLayout layout) {
        m_ColorAttachments[index].As<CVulkanRHITexture2D>()->SetCurrentLayout(layout);
    }
    vk::ImageLayout GetLayout(uint32_t index) const {
        return m_ColorAttachments[index].As<CVulkanRHITexture2D>()->GetCurrentLayout();
    }

    const vk::SwapchainKHR GetVKSwapchain() const { return *m_VKSwapchain; }
    const vk::Extent2D GetVKExtent() const { return m_VKExtent; }
    const uint32 GetImageCount() const { return m_ImageCount; }
    const vk::Format GetVKImageFormat() const { return m_VKImageFormat; }
    const vk::Format GetVKDepthFormat() const { return m_VKDepthFormat; }
    vk::Semaphore GetImageAvailableSemaphore(uint32 index) const {
        return *m_ImageAvailableSemaphores[index];
    }
    vk::Semaphore GetRenderFinishedSemaphore(uint32 index) const {
        return *m_RenderFinishedSemaphores[index];
    }

    const uint32 AcquireNextImage(vk::Semaphore semaphore);

private:
    WeakRef<CVulkanRHIDevice> m_Device;

    const vk::raii::SurfaceKHR& m_VKSurface;
    vk::raii::SwapchainKHR m_VKSwapchain = nullptr;
    vk::Format m_VKImageFormat, m_VKDepthFormat;
    vk::Extent2D m_VKExtent;
    uint32 m_ImageCount;

    std::vector<vk::raii::Semaphore> m_ImageAvailableSemaphores;
    std::vector<vk::raii::Semaphore> m_RenderFinishedSemaphores;

    bool m_NeedsRecreation = false;
};

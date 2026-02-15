#pragma once

#include "RHISwapchain.h"
#include "RHISyncObject.h"
#include "VulkanRHIDevice.h"
#include "VulkanRHIExport.h"
#include "VulkanUtils.h"

DECLARE_LOG_CATEGORY_EXTERN(LogVulkanRHISwapchain, Info);

class VULKAN_RHI_API CVulkanRHISwapchain : public IRHISwapchain {
    static constexpr uint32 INVALID_IMAGE_INDEX = 0xFFFFFFFF;

public:
    CVulkanRHISwapchain(const FRHISwapchainCreateInfo& info, const vk::raii::SurfaceKHR& surface,
                        const TRef<CVulkanRHIDevice> device);
    virtual ~CVulkanRHISwapchain();

private:
    void Init();
    void CleanupSwapchain();

public:
    void RecreateSwapchain();
    void CreateVKRenderPass();

    const vk::raii::SwapchainKHR& GetVKSwapchain() const { return m_Swapchain; }
    const std::vector<vk::Image>& GetVKImages() const { return m_Images; }
    const std::vector<vk::raii::ImageView>& GetVKImageViews() const { return m_ImageViews; }
    const vk::Extent2D GetVKExtent() const { return m_Extent; }
    // vk::Format& GetVKDepthFormat() { return m_ImageFormat; }

    virtual const uint32 AcquireNextImage(TRef<IRHISyncObject> syncObject) override;
    virtual const EPixelFormat GetImageFormat() override {
        return ChozoUtils::Vulkan::FromVKFormat(m_ImageFormat);
    }

    virtual void RecreateSwapchain(const FExtent2D& frameBufferSize) override;

private:
    WeakRef<CVulkanRHIDevice> m_Device;

    const vk::raii::SurfaceKHR& m_Surface;
    vk::raii::SwapchainKHR m_Swapchain = nullptr;
    std::vector<vk::Image> m_Images;
    std::vector<vk::raii::ImageView> m_ImageViews;
    vk::Format m_ImageFormat, m_DepthFormat;
    vk::Extent2D m_Extent;
};

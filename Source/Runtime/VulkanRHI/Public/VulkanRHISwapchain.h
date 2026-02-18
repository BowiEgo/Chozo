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

    void SetLayout(uint32_t index, vk::ImageLayout layout) { m_ImageLayouts[index] = layout; }
    vk::ImageLayout GetLayout(uint32_t index) const { return m_ImageLayouts[index]; }

    const vk::raii::SwapchainKHR& GetVKSwapchain() const { return m_Swapchain; }
    const std::vector<vk::Image>& GetVKImages() const { return m_Images; }
    const std::vector<vk::raii::ImageView>& GetVKImageViews() const { return m_ImageViews; }
    const vk::ImageView GetVKImageView(uint32 index) const { return *m_ImageViews[index]; }
    const vk::Extent2D GetVKExtent() const { return m_Extent; }
    // vk::Format& GetVKDepthFormat() { return m_ImageFormat; }
    const uint32 GetImageCount() const { return m_ImageCount; }
    const vk::raii::RenderPass& GetVKRenderPass() const { return m_RenderPass; }
    const vk::Format& GetVKImageFormat() const { return m_ImageFormat; }
    const vk::Format& GetVKDepthFormat() const { return m_DepthFormat; }

    virtual const uint32 AcquireNextImage(TRef<IRHISyncObject> syncObject) override;
    virtual const EPixelFormat GetImageFormat() const override {
        return ChozoUtils::Vulkan::FromVKFormat(m_ImageFormat);
    }
    virtual const EPixelFormat GetDepthFormat() const override {
        return ChozoUtils::Vulkan::FromVKFormat(m_DepthFormat);
    }
    virtual const FExtent2D GetExtent() const override {
        return FExtent2D(m_Extent.width, m_Extent.height);
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
    uint32 m_ImageCount;
    vk::raii::RenderPass m_RenderPass = nullptr;

    std::vector<vk::ImageLayout> m_ImageLayouts;
};

#pragma once

#include "RHISwapchain.h"
#include "VulkanRHIDevice.h"
#include "VulkanRHIExport.h"
#include "VulkanUtils.h"

DECLARE_LOG_CATEGORY_EXTERN(LogVulkanRHISwapchain, Info);

class VULKAN_RHI_API CVulkanRHISwapchain : public IRHISwapchain {
public:
    CVulkanRHISwapchain(const FRHISwapchainCreateInfo& info,
                        const vk::raii::SurfaceKHR& surface,
                        const TRef<CVulkanRHIDevice> device);
    virtual ~CVulkanRHISwapchain() = default;

private:
    void init(const vk::raii::SurfaceKHR& surface);

public:
    void CreateVKRenderPass();
    virtual const EPixelFormat GetImageFormat() const {
        return ChozoUtils::Vulkan::FromVulkanFormat(m_ImageFormat);
    }
    // vk::Format& GetDepthFormat() { return m_ImageFormat; }

private:
    TRef<CVulkanRHIDevice> m_Device;

    vk::raii::SwapchainKHR m_Swapchain = nullptr;
    std::vector<vk::Image> m_Images;
    std::vector<vk::raii::ImageView> m_ImageViews;
    vk::Format m_ImageFormat, m_DepthFormat;
    vk::Extent2D m_Extent;

    vk::RenderPass m_RenderPass = nullptr;
};
#pragma once

#include "RHISwapchain.h"
#include "VulkanRHIDevice.h"
#include "VulkanRHIExport.h"

DECLARE_LOG_CATEGORY_EXTERN(LogVulkanRHISwapchain, Info);

class VULKAN_RHI_API CVulkanRHISwapchain : public IRHISwapchain {
public:
    CVulkanRHISwapchain(const FRHISwapchainCreateInfo& info,
                        const vk::raii::SurfaceKHR& surface,
                        const TRef<CVulkanRHIDevice> device);
    virtual ~CVulkanRHISwapchain() = default;

private:
    void CreateVKSwapchain(const vk::raii::SurfaceKHR& surface);

private:
    TRef<CVulkanRHIDevice> m_Device;

    vk::raii::SwapchainKHR m_Swapchain = nullptr;
    std::vector<vk::Image> m_SwapchainImages;
    std::vector<vk::raii::ImageView> m_SwapchainImageViews;
    vk::Format m_SwapchainImageFormat;
    vk::Extent2D m_SwapchainExtent;
};
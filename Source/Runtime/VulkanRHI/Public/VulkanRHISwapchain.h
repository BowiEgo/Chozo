#pragma once

#include "RHISwapchain.h"
#include "VulkanRHIExport.h"

DECLARE_LOG_CATEGORY_EXTERN(LogVulkanRHISwapchain, Info);

class VULKAN_RHI_API CVulkanRHISwapchain : public IRHISwapchain {
public:
    CVulkanRHISwapchain(const vk::raii::Instance& instance,
                        const vk::raii::SurfaceKHR& surface,
                        const FRHISwapchainCreateInfo& info);
    virtual ~CVulkanRHISwapchain() = default;

private:
    void Init();
    void CreateVKSwapchain();

private:
    const vk::raii::Instance& m_Instance;
    const vk::raii::SurfaceKHR& m_Surface;

    vk::raii::SwapchainKHR m_Swapchain = nullptr;
    std::vector<vk::Image> m_SwapchainImages;
    std::vector<vk::raii::ImageView> m_SwapchainImageViews;
    vk::Format m_SwapchainImageFormat;
    vk::Extent2D m_SwapchainExtent;
};
#pragma once

#include "../Source/Runtime/RHI/SwapchainObj.h"
#include "VulkanGraphicsContextObj.h"
#include <Runtime/RHI/Swapchain.h>
#include <vulkan/vulkan_core.h>

namespace CZ {

static constexpr uint32 INVALID_IMAGE_INDEX = 0xFFFFFFFF;

class VulkanSwapchainObj : public SwapchainObj {
public:
    VulkanSwapchainObj(const Device device, const SwapchainSpecification& spec,
                       VulkanGraphicsContextObj* ctxObj);
    ~VulkanSwapchainObj() override;

    PixelFormat GetImageFormat() const override {
        return VulkanUtils::FromVKFormat(m_VkImageFormat);
    }
    PixelFormat GetDepthFormat() const override {
        return VulkanUtils::FromVKFormat(m_VKDepthFormat);
    }
    const Extent2D GetExtent() const override { return { m_VkExtent.width, m_VkExtent.height }; }

    uint32_t GetImageCount() const override { return m_ImageCount; }

    void SetPresentMode(const PresentMode mode) override {
        m_PresentMode     = mode;
        m_NeedsRecreation = true;
    }
    void Recreate(const Extent2D& frameBufferSize) override;

    VkSwapchainKHR GetVkSwapchain() const { return m_VkSwapchain; }

    VkFormat GetVkImageFormat() const { return m_VkImageFormat; }

private:
    void Init();

    VkSwapchainKHR m_VkSwapchain = VK_NULL_HANDLE;
    VkFormat m_VkImageFormat = VK_FORMAT_UNDEFINED, m_VKDepthFormat = VK_FORMAT_UNDEFINED;
    VkExtent2D m_VkExtent = { 0, 0 };
    uint32_t m_ImageCount = 0;

    std::vector<VkSemaphore> m_ImageAvailableSemaphores;
    std::vector<VkSemaphore> m_RenderFinishedSemaphores;

    bool m_NeedsRecreation = false;

    VulkanGraphicsContextObj* m_ContextObj;
};

} // namespace CZ
#pragma once

#include <Runtime/RHI/Swapchain.hpp>

#include "VulkanUtils.hpp"

namespace CZ {

static constexpr uint32 INVALID_IMAGE_INDEX = 0xFFFFFFFF;

class VulkanGraphicsContextObj;

class VulkanSwapchainObj : public SwapchainObj {
public:
    VulkanSwapchainObj(const VulkanGraphicsContextObj* ctxObj, const SwapchainSpecification& spec);
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

    uint32 AcquireNextImageIndex(VkSemaphore sem);

    void MarkNeedsRecreation();

    bool RecreateIfNeeded();

private:
    void Init();

    void Recreate() { Recreate(m_Spec.FrameBufferSize); }

    VkSwapchainKHR m_VkSwapchain = VK_NULL_HANDLE;
    VkFormat m_VkImageFormat = VK_FORMAT_UNDEFINED, m_VKDepthFormat = VK_FORMAT_UNDEFINED;
    VkExtent2D m_VkExtent = { 0, 0 };

    const VulkanGraphicsContextObj* m_ContextObj;
};

} // namespace CZ
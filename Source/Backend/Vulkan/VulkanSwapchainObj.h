#pragma once

#include <Runtime/RHI/Swapchain.h>

#include <Core/Memory/Memory.h>
#include <Core/Platform/Platform.h>

#include "VulkanUtils.h"

#ifdef CZ_PLATFORM_MACOS
    #define VK_USE_PLATFORM_METAL_EXT
#endif
#include <vulkan/vulkan.h>

namespace CZ {

static constexpr uint32 INVALID_IMAGE_INDEX = 0xFFFFFFFF;

class VulkanSwapchainObj : public SwapchainObj {
public:
    VulkanSwapchainObj(const Device device, const SwapchainSpecification& spec);
    ~VulkanSwapchainObj() override;

    PixelFormat GetImageFormat() const override {
        return VulkanUtils::FromVKFormat(m_VKImageFormat);
    }
    PixelFormat GetDepthFormat() const override {
        return VulkanUtils::FromVKFormat(m_VKDepthFormat);
    }
    const Extent2D GetExtent() const override { return { m_VKExtent.width, m_VKExtent.height }; }

    void SetPresentMode(const PresentMode mode) override {
        m_PresentMode     = mode;
        m_NeedsRecreation = true;
    }
    void Recreate(const Extent2D& frameBufferSize) override;

private:
    void Init();

    VkSwapchainKHR m_VKSwapchain = VK_NULL_HANDLE;
    VkFormat m_VKImageFormat = VK_FORMAT_UNDEFINED, m_VKDepthFormat = VK_FORMAT_UNDEFINED;
    VkExtent2D m_VKExtent = { 0, 0 };
    uint32_t m_ImageCount = 0;

    std::vector<VkSemaphore> m_ImageAvailableSemaphores;
    std::vector<VkSemaphore> m_RenderFinishedSemaphores;

    bool m_NeedsRecreation = false;
};

} // namespace CZ
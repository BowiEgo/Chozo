#include "VulkanRHISwapchain.h"

#include "VulkanRHIDevice.h"
#include "VulkanUtils.h"

CVulkanRHISwapchain::CVulkanRHISwapchain(const vk::raii::Instance& instance,
                                         const vk::raii::SurfaceKHR& surface,
                                         const FRHISwapchainCreateInfo& info)
    : IRHISwapchain(info), m_Instance(instance), m_Surface(surface) {
    Init();
}

void CVulkanRHISwapchain::Init() { CreateVKSwapchain(); }

void CVulkanRHISwapchain::CreateVKSwapchain() {
    TRef<CVulkanRHIDevice> vkDevice = m_Data.Device.As<CVulkanRHIDevice>();
    const vk::raii::PhysicalDevice& physicalDevice =
        vkDevice->GetVKPhysicalDevice();
    const vk::raii::Device& logicalDevice = vkDevice->GetVKLogicalDevice();

    ChozoUtils::Vulkan::SwapchainSupportDetails details =
        ChozoUtils::Vulkan::QuerySwapchainSupport(physicalDevice, m_Surface);

    int pixelWidth = m_Data.FrameBufferWidth,
        pixelHeight = m_Data.FrameBufferHeight;

    vk::SurfaceFormatKHR surfaceFormat =
        ChozoUtils::Vulkan::ChooseSwapSurfaceFormat(details.formats);
    vk::PresentModeKHR presentMode =
        ChozoUtils::Vulkan::ChooseSwapPresentMode(details.presentModes);
    vk::Extent2D extent = ChozoUtils::Vulkan::ChooseSwapExtent(
        details.capabilities, pixelWidth, pixelHeight);

    // Determine image count (Minimum + 1 for triple buffering)
    uint32_t imageCount = details.capabilities.minImageCount + 1;
    if (details.capabilities.maxImageCount > 0 &&
        imageCount > details.capabilities.maxImageCount) {
        imageCount = details.capabilities.maxImageCount;
    }

    vk::SwapchainCreateInfoKHR createInfo;
    createInfo.surface = *m_Surface;
    createInfo.minImageCount = imageCount;
    createInfo.imageFormat = surfaceFormat.format;
    createInfo.imageColorSpace = surfaceFormat.colorSpace;
    createInfo.imageExtent = extent;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = vk::ImageUsageFlagBits::eColorAttachment;

    // If indices are different, use Concurrent mode; otherwise use
    // Exclusive
    FQueueFamilyIndices indices =
        ChozoUtils::Vulkan::FindQueueFamilies(physicalDevice, m_Surface);
    uint32_t queueFamilyIndices[] = {indices.Graphics.value(),
                                     indices.Present.value()};
    if (indices.Graphics != indices.Present) {
        createInfo.imageSharingMode = vk::SharingMode::eConcurrent;
        createInfo.queueFamilyIndexCount = 2;
        createInfo.pQueueFamilyIndices = queueFamilyIndices;
    } else {
        createInfo.imageSharingMode = vk::SharingMode::eExclusive;
    }

    // Use the current transform of the surface to avoid unintended rotation
    if (details.capabilities.supportedTransforms &
        vk::SurfaceTransformFlagBitsKHR::eIdentity) {
        createInfo.preTransform = vk::SurfaceTransformFlagBitsKHR::eIdentity;
    } else {
        createInfo.preTransform = details.capabilities.currentTransform;
    }

    createInfo.compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque;
    createInfo.presentMode = presentMode;
    createInfo.clipped = true;
    createInfo.oldSwapchain = nullptr;

    m_Swapchain = vk::raii::SwapchainKHR(logicalDevice, createInfo);

    // Retrieve the images created by the swapchain
    m_SwapchainImages = m_Swapchain.getImages();
    m_SwapchainImageFormat = surfaceFormat.format;
    m_SwapchainExtent = extent;
}

#include "VulkanSwapchainObj.h"

#include "VulkanDeviceObj.h"
#include "VulkanGraphicContextObj.h"
#include "VulkanImageObj.h"
#include "VulkanTextureObj.h"

#include <Core/Header/Assert.h>
#include <Core/Log/LogMacros.h>
#include <Core/Memory/Memory.h>
#include <Runtime/RHI/RHIAPI.h>

namespace CZ {

DEFINE_LOG_CATEGORY_STATIC(LogVulkanSwapchain, Info);

extern "C" {

SwapchainObj* CreateVulkanSwapchainObj(const Device device, const SwapchainSpecification& spec) {
    return CZ_NEW(MEMORY_USAGE_RENDER, VulkanSwapchainObj, device, spec);
}
}

VulkanSwapchainObj::VulkanSwapchainObj(const Device device, const SwapchainSpecification& spec)
    : SwapchainObj(device, spec) {
    Init();
}

VulkanSwapchainObj::~VulkanSwapchainObj() {
    auto deviceObj = static_cast<VulkanDeviceObj*>(m_Device.Unwrap());

    CZ_CORE_ASSERT(deviceObj, "Device is no longer valid during Swapchain destroying!");

    VkDevice logicalDevice = deviceObj->GetLogicalDevice();

    for (auto sem : m_ImageAvailableSemaphores) {
        if (sem != VK_NULL_HANDLE) vkDestroySemaphore(logicalDevice, sem, nullptr);
    }
    m_ImageAvailableSemaphores.clear();

    for (auto sem : m_RenderFinishedSemaphores) {
        if (sem != VK_NULL_HANDLE) vkDestroySemaphore(logicalDevice, sem, nullptr);
    }
    m_RenderFinishedSemaphores.clear();

    vkDestroySwapchainKHR(logicalDevice, m_VKSwapchain, nullptr);
}

void VulkanSwapchainObj::Recreate(const Extent2D& frameBufferSize) {
    auto deviceObj = static_cast<VulkanDeviceObj*>(m_Device.Unwrap());

    CZ_CORE_ASSERT(deviceObj, "Device is no longer valid during Swapchain initialization!");

    deviceObj->WaitIdle();

    m_Spec.FrameBufferSize = frameBufferSize;
    Init();

    m_NeedsRecreation = false;
}

void VulkanSwapchainObj::Init() {
    auto deviceObj = static_cast<VulkanDeviceObj*>(m_Device.Unwrap());
    auto ctxObj    = static_cast<VulkanGraphicContextObj*>(deviceObj->GetGraphicContext().Unwrap());

    VkSurfaceKHR vkSurface = ctxObj->GetVKSurface();

    CZ_CORE_ASSERT(deviceObj, "Device is no longer valid during Swapchain initialization!");
    CZ_CORE_ASSERT(vkSurface != VK_NULL_HANDLE,
                   "Surface handle is null before Swapchain initialization!");

    VkDevice logicalDevice          = deviceObj->GetLogicalDevice();
    VkPhysicalDevice physicalDevice = deviceObj->GetPhysicalDevice();

    VulkanUtils::SwapchainSupportDetails details =
        VulkanUtils::QuerySwapchainSupport(physicalDevice, vkSurface);

    CZ_LOG(LogVulkan, Info, "Vulkan surface current extent: {}x{}",
           details.Capabilities.currentExtent.width, details.Capabilities.currentExtent.height);
    CZ_LOG(LogVulkan, Info, "Vulkan surface min extent: {}x{}",
           details.Capabilities.minImageExtent.width, details.Capabilities.minImageExtent.height);
    CZ_LOG(LogVulkan, Info, "Vulkan surface max extent: {}x{}",
           details.Capabilities.maxImageExtent.width, details.Capabilities.maxImageExtent.height);

    int pixelWidth = m_Spec.FrameBufferSize.Width, pixelHeight = m_Spec.FrameBufferSize.Height;

    VkSurfaceFormatKHR surfaceFormat = VulkanUtils::ChooseSwapSurfaceFormat(details.Formats);

    VkPresentModeKHR presentMode =
        VulkanUtils::ChooseSwapPresentMode(m_PresentMode, details.PresentModes);

    VkExtent2D extent{};
    bool hasSwapchainMaintenance =
        deviceObj->IsExtensionSupported(VK_EXT_SWAPCHAIN_MAINTENANCE_1_EXTENSION_NAME);

    if (hasSwapchainMaintenance) {
        CZ_LOG(LogVulkanSwapchain, Info,
               "VK_EXT_swapchain_maintenance1 supported, using physical size");
        extent.width  = pixelWidth;
        extent.height = pixelHeight;
    } else {
        CZ_LOG(LogVulkanSwapchain, Warning,
               "VK_EXT_swapchain_maintenance1 not supported, falling back to surface extent");
        extent = VulkanUtils::ChooseSwapExtent(details.Capabilities, pixelWidth, pixelHeight);
    }

    // Determine image count
    m_ImageCount = details.Capabilities.minImageCount + 1;
    if (details.Capabilities.maxImageCount > 0 &&
        m_ImageCount > details.Capabilities.maxImageCount) {
        m_ImageCount = details.Capabilities.maxImageCount;
    }

    VkSwapchainCreateInfoKHR createInfo{};
    createInfo.sType            = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface          = vkSurface;
    createInfo.minImageCount    = m_ImageCount;
    createInfo.imageFormat      = surfaceFormat.format;
    createInfo.imageColorSpace  = surfaceFormat.colorSpace;
    createInfo.imageExtent      = extent;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage       = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    QueueFamilyIndices indices    = VulkanUtils::FindQueueFamilies(physicalDevice, vkSurface);
    uint32_t queueFamilyIndices[] = { indices.Graphics.value(), indices.Present.value() };
    if (indices.Graphics != indices.Present) {
        createInfo.imageSharingMode      = VK_SHARING_MODE_CONCURRENT;
        createInfo.queueFamilyIndexCount = 2;
        createInfo.pQueueFamilyIndices   = queueFamilyIndices;
    } else {
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    }

    if (details.Capabilities.supportedTransforms & VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR) {
        createInfo.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
    } else {
        createInfo.preTransform = details.Capabilities.currentTransform;
    }

    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    createInfo.presentMode    = presentMode;
    createInfo.clipped        = VK_TRUE;

    VkSwapchainPresentScalingCreateInfoEXT scalingInfo{};
    if (hasSwapchainMaintenance) {
        scalingInfo.sType           = VK_STRUCTURE_TYPE_SWAPCHAIN_PRESENT_SCALING_CREATE_INFO_EXT;
        scalingInfo.scalingBehavior = VK_PRESENT_SCALING_STRETCH_BIT_EXT;
        createInfo.pNext            = &scalingInfo;
        CZ_LOG(LogVulkanSwapchain, Info, "Added present scaling info to swapchain creation");
    }

    // Preserve old swapchain for recreation
    VkSwapchainKHR oldSwapchain = m_VKSwapchain;
    if (oldSwapchain != VK_NULL_HANDLE) {
        createInfo.oldSwapchain = oldSwapchain;
    }

    VkSwapchainKHR newSwapchain = VK_NULL_HANDLE;
    VkResult result = vkCreateSwapchainKHR(logicalDevice, &createInfo, nullptr, &newSwapchain);
    if (result != VK_SUCCESS) {
        CZ_LOG(LogVulkanSwapchain, Error, "Failed to create swapchain! VkResult = {}",
               VulkanUtils::VkResultToString(result));
        return;
    }

    // Destroy old swapchain if it was replaced
    if (oldSwapchain != VK_NULL_HANDLE) {
        vkDestroySwapchainKHR(logicalDevice, oldSwapchain, nullptr);
    }
    m_VKSwapchain   = newSwapchain;
    m_VKImageFormat = surfaceFormat.format;
    m_VKDepthFormat = VK_FORMAT_D32_SFLOAT;
    m_VKExtent      = extent;

    // Get swapchain images
    uint32_t imageCount = 0;
    vkGetSwapchainImagesKHR(logicalDevice, m_VKSwapchain, &imageCount, nullptr);
    std::vector<VkImage> images(imageCount);
    vkGetSwapchainImagesKHR(logicalDevice, m_VKSwapchain, &imageCount, images.data());

    VkSemaphoreCreateInfo semiInfo{};
    semiInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    m_ColorAttachments.clear();
    m_ColorAttachments.reserve(images.size());

    m_ImageAvailableSemaphores.clear();
    m_RenderFinishedSemaphores.clear();

    for (auto rawImage : images) {
        // Wrap each VkImage into RHI Texture object
        TextureSpecification texSpec;
        texSpec.Name   = "Swapchain_ColorAttachment_" + std::to_string(m_ColorAttachments.size());
        texSpec.Size   = Extent2D(m_VKExtent.width, m_VKExtent.height);
        texSpec.Format = VulkanUtils::FromVKFormat(m_VKImageFormat);
        texSpec.Usage  = TextureUsage::Attachment;

        // Create a VulkanImage that holds the external swapchain image
        VulkanImageObj* imageObj = CZ_NEW(MEMORY_USAGE_RENDER, VulkanImageObj, m_Device,
                                          texSpec.ToImageSpec(), rawImage, true);

        Image image(imageObj);

        VulkanTextureObj* textureObj =
            CZ_NEW(MEMORY_USAGE_RENDER, VulkanTextureObj, m_Device, texSpec, image);

        Texture texture(textureObj);

        m_ColorAttachments.push_back(texture);

        // Create semaphores
        VkSemaphore imageAvailable, renderFinished;
        if (vkCreateSemaphore(logicalDevice, &semiInfo, nullptr, &imageAvailable) != VK_SUCCESS ||
            vkCreateSemaphore(logicalDevice, &semiInfo, nullptr, &renderFinished) != VK_SUCCESS) {
            CZ_LOG(LogVulkanSwapchain, Error, "Failed to create semaphores for swapchain images");
            // Cleanup partially created resources (not shown for brevity)
            return;
        }
        m_ImageAvailableSemaphores.push_back(imageAvailable);
        m_RenderFinishedSemaphores.push_back(renderFinished);
    }

    CZ_LOG(LogVulkanSwapchain, Info, "=== Swapchain Creation Debug ===");
    CZ_LOG(LogVulkanSwapchain, Info, "Physical framebuffer size: {}x{}", pixelWidth, pixelHeight);
    CZ_LOG(LogVulkanSwapchain, Info, "Vulkan surface current extent: {}x{}",
           details.Capabilities.currentExtent.width, details.Capabilities.currentExtent.height);
    CZ_LOG(LogVulkanSwapchain, Info, "Final chosen extent: {}x{}", extent.width, extent.height);
    CZ_LOG(LogVulkanSwapchain, Info, "Swapchain created with {} images at {}x{}", images.size(),
           extent.width, extent.height);
}

} // namespace CZ
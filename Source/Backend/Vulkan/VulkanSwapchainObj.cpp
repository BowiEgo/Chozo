#include "VulkanSwapchainObj.hpp"

#include "VulkanDeviceObj.hpp"
#include "VulkanFenceObj.hpp"
#include "VulkanGraphicsContextObj.hpp"
#include "VulkanImageObj.hpp"
#include "VulkanSemaphoreObj.hpp"
#include "VulkanTextureObj.hpp"

namespace CZ {

VulkanSwapchainObj::VulkanSwapchainObj(const VulkanGraphicsContextObj* ctxObj,
                                       const SwapchainSpecification& spec)
    : SwapchainObj(spec), m_ContextObj(ctxObj) {
    Init();

    m_RenderFinishedSemaphores.reserve(m_ColorAttachments.size());
    for (size_t i = 0; i < m_ColorAttachments.size(); ++i) {
        m_RenderFinishedSemaphores.emplace_back(
            CZ_NEW(MEMORY_USAGE_RENDER, VulkanSemaphoreObj, ctxObj->m_DeviceObj));
    }

    m_ImageAvailableSemaphores.reserve(ctxObj->GetMaxFramesInFlight());
    for (uint32_t i = 0; i < ctxObj->GetMaxFramesInFlight(); ++i) {
        m_ImageAvailableSemaphores.emplace_back(
            CZ_NEW(MEMORY_USAGE_RENDER, VulkanSemaphoreObj, ctxObj->m_DeviceObj));
    }

    m_InFlightFences.reserve(ctxObj->GetMaxFramesInFlight());
    for (uint32_t i = 0; i < ctxObj->GetMaxFramesInFlight(); ++i) {
        auto result = VulkanFenceObj::Create(ctxObj->m_DeviceObj);
        if (result) m_InFlightFences.emplace_back(result.value());
    }
}

VulkanSwapchainObj::~VulkanSwapchainObj() {
    auto deviceObj = m_ContextObj->m_DeviceObj;

    CZ_CORE_ASSERT(deviceObj, "Device is no longer valid during Swapchain destroying!");

    VkDevice logicalDevice = deviceObj->GetLogicalDevice();

    deviceObj->WaitIdle();

    vkDestroySwapchainKHR(logicalDevice, m_VkSwapchain, nullptr);
}

uint32 VulkanSwapchainObj::AcquireNextImageIndex(VkSemaphore sem) {
    auto deviceObj = m_ContextObj->m_DeviceObj;

    VkDevice logicalDevice = deviceObj->GetLogicalDevice();

    uint32_t imageIndex = 0;
    VkResult result     = vkAcquireNextImageKHR(logicalDevice,
                                                m_VkSwapchain,  // VkSwapchainKHR
                                                UINT64_MAX,     // timeout (uint64_t)
                                                sem,            // VkSemaphore to signal
                                                VK_NULL_HANDLE, // fence (optional)
                                                &imageIndex);

    if (result == VK_SUCCESS || result == VK_SUBOPTIMAL_KHR) {
        if (result == VK_SUBOPTIMAL_KHR) {
            m_NeedsRecreation = true;
        }
        return imageIndex;
    } else if (result == VK_ERROR_OUT_OF_DATE_KHR) {
        m_NeedsRecreation = true;
        return INVALID_IMAGE_INDEX;
    } else {
        CZ_BACKEND_LOG(Error, "vkAcquireNextImageKHR failed with {}",
                       VulkanUtils::VkResultToString(result));
        return INVALID_IMAGE_INDEX;
    }
}

void VulkanSwapchainObj::Recreate(const Extent2D& frameBufferSize) {
    auto deviceObj = m_ContextObj->m_DeviceObj;

    CZ_CORE_ASSERT(deviceObj, "Device is no longer valid during Swapchain initialization!");

    deviceObj->WaitIdle();

    m_Spec.FrameBufferSize = frameBufferSize;
    Init();

    m_NeedsRecreation = false;
}

void VulkanSwapchainObj::MarkNeedsRecreation() {
    if (!m_NeedsRecreation) {
        CZ_BACKEND_LOG(Trace, "Swapchain marked for recreation");
        m_NeedsRecreation = true;
    }
}

bool VulkanSwapchainObj::RecreateIfNeeded() {
    if (m_NeedsRecreation) {
        Recreate();
        return true;
    }

    return false;
}

void VulkanSwapchainObj::Init() {
    auto deviceObj         = m_ContextObj->m_DeviceObj;
    VkSurfaceKHR vkSurface = m_ContextObj->GetVKSurface();

    CZ_CORE_ASSERT(deviceObj, "Device is no longer valid during Swapchain initialization!");
    CZ_CORE_ASSERT(vkSurface != VK_NULL_HANDLE,
                   "Surface handle is null before Swapchain initialization!");

    VkDevice logicalDevice          = deviceObj->GetLogicalDevice();
    VkPhysicalDevice physicalDevice = deviceObj->GetPhysicalDevice();

    VulkanUtils::SwapchainSupportDetails details =
        VulkanUtils::QuerySwapchainSupport(physicalDevice, vkSurface);

    // CZ_BACKEND_LOG(Info, "Vulkan surface current extent: {}x{}",
    //        details.Capabilities.currentExtent.width, details.Capabilities.currentExtent.height);
    // CZ_BACKEND_LOG(Info, "Vulkan surface min extent: {}x{}",
    //        details.Capabilities.minImageExtent.width,
    //        details.Capabilities.minImageExtent.height);
    // CZ_BACKEND_LOG(Info, "Vulkan surface max extent: {}x{}",
    //        details.Capabilities.maxImageExtent.width,
    //        details.Capabilities.maxImageExtent.height);

    int pixelWidth = m_Spec.FrameBufferSize.Width, pixelHeight = m_Spec.FrameBufferSize.Height;

    VkSurfaceFormatKHR surfaceFormat = VulkanUtils::ChooseSwapSurfaceFormat(details.Formats);

    VkPresentModeKHR presentMode =
        VulkanUtils::ChooseSwapPresentMode(m_PresentMode, details.PresentModes);

    VkExtent2D extent{};
    bool hasSwapchainMaintenance =
        deviceObj->IsExtensionSupported(VK_EXT_SWAPCHAIN_MAINTENANCE_1_EXTENSION_NAME);

    if (hasSwapchainMaintenance) {
        CZ_BACKEND_LOG(Info, "VK_EXT_swapchain_maintenance1 supported, using physical size");
        extent.width  = pixelWidth;
        extent.height = pixelHeight;
    } else {
        CZ_BACKEND_LOG(Warning, "VK_EXT_swapchain_maintenance1 not supported, falling back to "
                                "surface extent");
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
    createInfo.imageUsage       = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;

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
        CZ_BACKEND_LOG(Info, "Added present scaling info to swapchain creation");
    }

    // Preserve old swapchain for recreation
    VkSwapchainKHR oldSwapchain = m_VkSwapchain;
    if (oldSwapchain != VK_NULL_HANDLE) {
        createInfo.oldSwapchain = oldSwapchain;
    }

    VkSwapchainKHR newSwapchain = VK_NULL_HANDLE;
    VkResult result = vkCreateSwapchainKHR(logicalDevice, &createInfo, nullptr, &newSwapchain);
    if (result != VK_SUCCESS) {
        CZ_BACKEND_LOG(Error, "Failed to create swapchain! VkResult = {}",
                       VulkanUtils::VkResultToString(result));
        return;
    }

    // Destroy old swapchain if it was replaced
    if (oldSwapchain != VK_NULL_HANDLE) {
        vkDestroySwapchainKHR(logicalDevice, oldSwapchain, nullptr);
    }
    m_VkSwapchain   = newSwapchain;
    m_VkImageFormat = surfaceFormat.format;
    m_VKDepthFormat = VK_FORMAT_D32_SFLOAT;
    m_VkExtent      = extent;

    // Get swapchain images
    uint32_t imageCount = 0;
    vkGetSwapchainImagesKHR(logicalDevice, m_VkSwapchain, &imageCount, nullptr);
    std::vector<VkImage> images(imageCount);
    vkGetSwapchainImagesKHR(logicalDevice, m_VkSwapchain, &imageCount, images.data());

    m_ColorAttachments.clear();
    m_ColorAttachments.reserve(images.size());

    for (auto rawImage : images) {
        // Wrap each VkImage into RHI Texture object
        TextureSpecification texSpec;
        texSpec.Name   = "Swapchain_ColorAttachment_" + std::to_string(m_ColorAttachments.size());
        texSpec.Size   = Extent2D(m_VkExtent.width, m_VkExtent.height);
        texSpec.Format = VulkanUtils::FromVKFormat(m_VkImageFormat);
        texSpec.Usage  = TextureUsage::Attachment;

        // Create a VulkanImage that holds the external swapchain image
        VulkanImageObj* imageObj =
            CZ_NEW(MEMORY_USAGE_RENDER, VulkanImageObj, m_ContextObj->m_DeviceObj,
                   texSpec.ToImageSpec(), rawImage, true);

        Image image(imageObj);

        VulkanTextureObj* textureObj = CZ_NEW(MEMORY_USAGE_RENDER, VulkanTextureObj,
                                              m_ContextObj->m_DeviceObj, texSpec, image);

        Texture texture(textureObj);

        m_ColorAttachments.push_back(texture);
    }

    CZ_BACKEND_LOG(Info, "=== Swapchain Creation Debug ===");
    CZ_BACKEND_LOG(Info, "Physical framebuffer size: {}x{}", pixelWidth, pixelHeight);
    CZ_BACKEND_LOG(Info, "Vulkan surface current extent: {}x{}",
                   details.Capabilities.currentExtent.width,
                   details.Capabilities.currentExtent.height);
    CZ_BACKEND_LOG(Info, "Final chosen extent: {}x{}", extent.width, extent.height);
    CZ_BACKEND_LOG(Info, "Swapchain created with {} images at {}x{}", images.size(), extent.width,
                   extent.height);
}

} // namespace CZ
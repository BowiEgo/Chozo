#include "VulkanSwapchain.h"

#include "VulkanDevice.h"
#include "VulkanSyncObject.h"

DEFINE_LOG_CATEGORY(LogVulkanSwapchain);

CVulkanSwapchain::CVulkanSwapchain(const FSwapchainSpecification& spec,
                                   const TRef<CVulkanDevice>& device,
                                   const vk::raii::SurfaceKHR& surface)
    : IRHISwapchain(spec), m_Device(device), m_VKSurface(surface) {
    Init();
}

CVulkanSwapchain::~CVulkanSwapchain() {
    CZ_LOG(LogVulkanSwapchain, Trace, "VulkanSwapchain destroying...");
}

const uint32 CVulkanSwapchain::AcquireNextImage(vk::Semaphore semaphore) {
    try {
        auto resultValue = m_VKSwapchain.acquireNextImage(UINT32_MAX, semaphore);

        if (resultValue.result == vk::Result::eSuboptimalKHR) {
            m_NeedsRecreation = true; // Mark for later to avoid breaking RAII flow
        }
        return resultValue.value;

    } catch (const vk::OutOfDateKHRError& e) {
        m_NeedsRecreation = true;
        return INVALID_IMAGE_INDEX;
    }
}

void CVulkanSwapchain::SetPresentMode(const EPresentMode mode) {
    m_PresentMode     = mode;
    m_NeedsRecreation = true;
}

void CVulkanSwapchain::Recreate(const FExtent2D& frameBufferSize) {
    auto device = m_Device.lock();
    CZ_CORE_ASSERT(device, "Device is no longer valid during Swapchain recreation!");

    device->GetLogicalDevice().waitIdle();
    m_Spec.FrameBufferSize = frameBufferSize;

    Init();

    m_NeedsRecreation = false;
}

void CVulkanSwapchain::MarkNeedsRecreation() {
    if (!m_NeedsRecreation) {
        CZ_LOG(LogVulkanSwapchain, Trace, "Swapchain marked for recreation");
        m_NeedsRecreation = true;
    }
}

bool CVulkanSwapchain::RecreateIfNeeded() {
    if (m_NeedsRecreation) {
        Recreate();
        return true;
    }

    return false;
}

void CVulkanSwapchain::Init() {
    auto device = m_Device.lock();
    CZ_CORE_ASSERT(device, "Device is no longer valid during Swapchain initialization!");
    CZ_CORE_ASSERT(*m_VKSurface, "Surface handle is null before Swapchain initialization!");

    const vk::raii::PhysicalDevice& raiihysicalDevice = device->GetRAIIPhysicalDevice();
    const vk::raii::Device& raiiDevice                = device->GetRAIILogicalDevice();

    ChozoUtils::Vulkan::SwapchainSupportDetails details =
        ChozoUtils::Vulkan::QuerySwapchainSupport(raiihysicalDevice, m_VKSurface);

    CZ_LOG(LogVulkan, Info, "Vulkan surface current extent: {}x{}",
           details.capabilities.currentExtent.width, details.capabilities.currentExtent.height);
    CZ_LOG(LogVulkan, Info, "Vulkan surface min extent: {}x{}",
           details.capabilities.minImageExtent.width, details.capabilities.minImageExtent.height);
    CZ_LOG(LogVulkan, Info, "Vulkan surface max extent: {}x{}",
           details.capabilities.maxImageExtent.width, details.capabilities.maxImageExtent.height);

    int pixelWidth = m_Spec.FrameBufferSize.Width, pixelHeight = m_Spec.FrameBufferSize.Height;

    vk::SurfaceFormatKHR surfaceFormat =
        ChozoUtils::Vulkan::ChooseSwapSurfaceFormat(details.formats);

    vk::PresentModeKHR presentMode =
        ChozoUtils::Vulkan::ChooseSwapPresentMode(m_PresentMode, details.presentModes);
    // vk::Extent2D extent =
    //     ChozoUtils::Vulkan::ChooseSwapExtent(details.capabilities, pixelWidth, pixelHeight);

    vk::Extent2D extent;
    bool hasSwapchainMaintenance =
        device->IsExtensionSupported(VK_EXT_SWAPCHAIN_MAINTENANCE_1_EXTENSION_NAME);

    if (hasSwapchainMaintenance) {
        CZ_LOG(LogVulkanSwapchain, Info,
               "VK_EXT_swapchain_maintenance1 supported, using physical size");
        extent.width  = pixelWidth;
        extent.height = pixelHeight;
    } else {
        CZ_LOG(LogVulkanSwapchain, Warning,
               "VK_EXT_swapchain_maintenance1 not supported, falling back to surface extent");
        extent =
            ChozoUtils::Vulkan::ChooseSwapExtent(details.capabilities, pixelWidth, pixelHeight);
    }

    // Determine image count (Minimum + 1 for triple buffering)
    m_ImageCount = details.capabilities.minImageCount + 1;
    if (details.capabilities.maxImageCount > 0 &&
        m_ImageCount > details.capabilities.maxImageCount) {
        m_ImageCount = details.capabilities.maxImageCount;
    }

    vk::SwapchainCreateInfoKHR createInfo;
    createInfo.surface          = *m_VKSurface;
    createInfo.minImageCount    = m_ImageCount;
    createInfo.imageFormat      = surfaceFormat.format;
    createInfo.imageColorSpace  = surfaceFormat.colorSpace;
    createInfo.imageExtent      = extent;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage       = vk::ImageUsageFlagBits::eColorAttachment;

    // If indices are different, use Concurrent mode; otherwise use Exclusive
    FQueueFamilyIndices indices =
        ChozoUtils::Vulkan::FindQueueFamilies(raiihysicalDevice, m_VKSurface);
    uint32_t queueFamilyIndices[] = { indices.Graphics.value(), indices.Present.value() };
    if (indices.Graphics != indices.Present) {
        createInfo.imageSharingMode      = vk::SharingMode::eConcurrent;
        createInfo.queueFamilyIndexCount = 2;
        createInfo.pQueueFamilyIndices   = queueFamilyIndices;
    } else {
        createInfo.imageSharingMode = vk::SharingMode::eExclusive;
    }

    // Use the current transform of the surface to avoid unintended rotation
    if (details.capabilities.supportedTransforms & vk::SurfaceTransformFlagBitsKHR::eIdentity) {
        createInfo.preTransform = vk::SurfaceTransformFlagBitsKHR::eIdentity;
    } else {
        createInfo.preTransform = details.capabilities.currentTransform;
    }

    createInfo.compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque;
    createInfo.presentMode    = presentMode;
    createInfo.clipped        = true;

    vk::SwapchainPresentScalingCreateInfoEXT scalingInfo;
    if (hasSwapchainMaintenance) {
        // Use single scaling flag, cannot combine multiple bits
        scalingInfo.setScalingBehavior(vk::PresentScalingFlagBitsEXT::eStretch
                                       //  | vk::PresentScalingFlagBitsEXT::eAspectRatioStretch
        );

        createInfo.setPNext(&scalingInfo);

        CZ_LOG(LogVulkanSwapchain, Info, "Added present scaling info to swapchain creation");
    }

    vk::raii::SwapchainKHR oldSwapchain = std::move(m_VKSwapchain);
    if (*oldSwapchain) {
        createInfo.oldSwapchain = *oldSwapchain;
    }
    m_VKSwapchain   = vk::raii::SwapchainKHR(raiiDevice, createInfo);
    m_VKImageFormat = surfaceFormat.format;
    m_VKDepthFormat = vk::Format::eD32Sfloat;
    m_VKExtent      = extent;

    // Retrieve the images created by the swapchain
    auto images = m_VKSwapchain.getImages();
    vk::SemaphoreCreateInfo semiInfo;

    m_ColorAttachments.clear();
    m_ColorAttachments.reserve(images.size());

    m_ImageAvailableSemaphores.clear();
    m_RenderFinishedSemaphores.clear();

    for (auto rawImage : images) {
        // Wrap each VkImage into RHI Texture object
        FTexture2DSpecification texSpec;
        texSpec.Name   = "Swapchain_ColorAttachment_" + std::to_string(m_ColorAttachments.size());
        texSpec.Size   = FExtent2D(m_VKExtent.width, m_VKExtent.height);
        texSpec.Format = ChozoUtils::Vulkan::FromVKFormat(m_VKImageFormat);
        texSpec.Usage  = ETextureUsage::Attachment; // Swapchain images are used as render targets

        TRef<CVulkanTexture2D> texture = CreateRef<CVulkanTexture2D>(
            WeakRef<IRHIDevice>(device), texSpec, rawImage,
            false); // Here m_IsOwned will be false because Swapchain owns the Image lifecycle

        m_ColorAttachments.push_back(texture);

        m_ImageAvailableSemaphores.emplace_back(raiiDevice, semiInfo);
        m_RenderFinishedSemaphores.emplace_back(raiiDevice, semiInfo);
    }

    CZ_LOG(LogVulkanSwapchain, Info, "=== Swapchain Creation Debug ===");
    CZ_LOG(LogVulkanSwapchain, Info, "Physical framebuffer size: {}x{}", pixelWidth, pixelHeight);
    CZ_LOG(LogVulkanSwapchain, Info, "Vulkan surface current extent: {}x{}",
           details.capabilities.currentExtent.width, details.capabilities.currentExtent.height);
    CZ_LOG(LogVulkanSwapchain, Info, "Final chosen extent: {}x{}", extent.width, extent.height);
    CZ_LOG(LogVulkanSwapchain, Info, "Swapchain created with {} images at {}x{}", images.size(),
           extent.width, extent.height);
}

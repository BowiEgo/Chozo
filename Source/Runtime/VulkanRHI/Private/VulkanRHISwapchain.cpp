#include "VulkanRHISwapchain.h"

#include "VulkanRHIDevice.h"
#include "VulkanRHISyncObject.h"

DEFINE_LOG_CATEGORY(LogVulkanRHISwapchain);

CVulkanRHISwapchain::CVulkanRHISwapchain(const FRHISwapchainSpecification& spec,
                                         const TRef<CVulkanRHIDevice>& device,
                                         const vk::raii::SurfaceKHR& surface)
    : IRHISwapchain(spec), m_Device(device), m_VKSurface(surface) {
    Init();
}

CVulkanRHISwapchain::~CVulkanRHISwapchain() {
    CZ_LOG(LogVulkanRHISwapchain, Trace, "VulkanRHISwapchain destroying...");
}

const uint32 CVulkanRHISwapchain::AcquireNextImage(TRef<IRHISyncObject> syncObject) {
    auto vkSync = syncObject.As<CVulkanRHISyncObject>();
    vk::Semaphore semaphore = vkSync->GetPresentCompleteSemaphore();

    auto resultValue =
        m_VKSwapchain.acquireNextImage((std::numeric_limits<uint64_t>::max)(), semaphore);

    if (resultValue.result == vk::Result::eErrorOutOfDateKHR ||
        resultValue.result == vk::Result::eSuboptimalKHR) {

        RecreateSwapchain();
        return INVALID_IMAGE_INDEX;
    }

    return resultValue.value;
}

void CVulkanRHISwapchain::RecreateSwapchain(const FExtent2D& frameBufferSize) {
    auto device = m_Device.lock();
    CZ_CORE_ASSERT(device, "Device is no longer valid during Swapchain recreation!");

    device->GetLogicalDevice().waitIdle();
    m_Spec.FrameBufferSize = frameBufferSize;

    Init();
}

void CVulkanRHISwapchain::Init() {
    auto device = m_Device.lock();
    CZ_CORE_ASSERT(device, "Device is no longer valid during Swapchain initialization!");
    CZ_CORE_ASSERT(*m_VKSurface, "Surface handle is null before Swapchain initialization!");

    const vk::raii::PhysicalDevice& physicalDevice = device->GetRAIIPhysicalDevice();
    const vk::raii::Device& raiiDevice = device->GetRAIILogicalDevice();

    ChozoUtils::Vulkan::SwapchainSupportDetails details =
        ChozoUtils::Vulkan::QuerySwapchainSupport(physicalDevice, m_VKSurface);

    int pixelWidth = m_Spec.FrameBufferSize.Width, pixelHeight = m_Spec.FrameBufferSize.Height;

    vk::SurfaceFormatKHR surfaceFormat =
        ChozoUtils::Vulkan::ChooseSwapSurfaceFormat(details.formats);

    vk::PresentModeKHR presentMode =
        ChozoUtils::Vulkan::ChooseSwapPresentMode(details.presentModes);
    vk::Extent2D extent =
        ChozoUtils::Vulkan::ChooseSwapExtent(details.capabilities, pixelWidth, pixelHeight);

    // Determine image count (Minimum + 1 for triple buffering)
    m_ImageCount = details.capabilities.minImageCount + 1;
    if (details.capabilities.maxImageCount > 0 &&
        m_ImageCount > details.capabilities.maxImageCount) {
        m_ImageCount = details.capabilities.maxImageCount;
    }

    vk::SwapchainCreateInfoKHR createInfo;
    createInfo.surface = *m_VKSurface;
    createInfo.minImageCount = m_ImageCount;
    createInfo.imageFormat = surfaceFormat.format;
    createInfo.imageColorSpace = surfaceFormat.colorSpace;
    createInfo.imageExtent = extent;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = vk::ImageUsageFlagBits::eColorAttachment;

    // If indices are different, use Concurrent mode; otherwise use Exclusive
    FQueueFamilyIndices indices =
        ChozoUtils::Vulkan::FindQueueFamilies(physicalDevice, m_VKSurface);
    uint32_t queueFamilyIndices[] = { indices.Graphics.value(), indices.Present.value() };
    if (indices.Graphics != indices.Present) {
        createInfo.imageSharingMode = vk::SharingMode::eConcurrent;
        createInfo.queueFamilyIndexCount = 2;
        createInfo.pQueueFamilyIndices = queueFamilyIndices;
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
    createInfo.presentMode = presentMode;
    createInfo.clipped = true;

    if (*m_VKSwapchain) {
        createInfo.oldSwapchain = *m_VKSwapchain;
    } else {
        createInfo.oldSwapchain = nullptr;
    }
    m_VKSwapchain = vk::raii::SwapchainKHR(raiiDevice, createInfo);
    m_VKImageFormat = surfaceFormat.format;
    m_VKDepthFormat = vk::Format::eD32Sfloat;
    m_VKExtent = extent;

    // Retrieve the images created by the swapchain
    auto images = m_VKSwapchain.getImages();

    m_ColorAttachments.clear();
    m_ColorAttachments.reserve(images.size());
    for (auto rawImage : images) {
        // Wrap each VkImage into RHI Texture object
        FTextureSpecification texSpec;
        texSpec.Name = "Swapchain_ColorAttachment_" + std::to_string(m_ColorAttachments.size());
        texSpec.Size = FExtent2D(m_VKExtent.width, m_VKExtent.height);
        texSpec.Format = ChozoUtils::Vulkan::FromVKFormat(m_VKImageFormat);
        texSpec.Usage = ETextureUsage::ColorAttachment;
        TRef<CVulkanRHITexture2D> texture = CreateRef<CVulkanRHITexture2D>(
            texSpec, device, rawImage,
            false); // Here m_IsOwned will be false because Swapchain owns the Image lifecycle

        m_ColorAttachments.push_back(texture);
    }
}

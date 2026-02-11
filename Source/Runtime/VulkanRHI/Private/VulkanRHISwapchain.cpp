#include "VulkanRHISwapchain.h"

#include "VulkanRHIDevice.h"
#include "VulkanRHISyncObject.h"

DEFINE_LOG_CATEGORY(LogVulkanRHISwapchain);

CVulkanRHISwapchain::CVulkanRHISwapchain(const FRHISwapchainCreateInfo& info,
                                         const vk::raii::SurfaceKHR& surface,
                                         const TRef<CVulkanRHIDevice> device)
    : IRHISwapchain(info), m_Surface(surface), m_Device(WeakRef(device)) {
    Init();
}

CVulkanRHISwapchain::~CVulkanRHISwapchain() {
    CZ_LOG(LogVulkanRHISwapchain, Trace, "VulkanRHISwapchain destroying...");
}

void CVulkanRHISwapchain::Init() {
    CZ_CORE_ASSERT(*m_Surface, "Surface handle is null before creating swapchain!");

    const vk::raii::PhysicalDevice& physicalDevice = m_Device->GetPhysicalDevice();
    const vk::raii::Device& logicalDevice = m_Device->GetLogicalDevice();

    ChozoUtils::Vulkan::SwapchainSupportDetails details =
        ChozoUtils::Vulkan::QuerySwapchainSupport(physicalDevice, m_Surface);

    int pixelWidth = m_Info.FrameBufferSize.Width, pixelHeight = m_Info.FrameBufferSize.Height;

    vk::SurfaceFormatKHR surfaceFormat =
        ChozoUtils::Vulkan::ChooseSwapSurfaceFormat(details.formats);

    vk::PresentModeKHR presentMode =
        ChozoUtils::Vulkan::ChooseSwapPresentMode(details.presentModes);
    vk::Extent2D extent =
        ChozoUtils::Vulkan::ChooseSwapExtent(details.capabilities, pixelWidth, pixelHeight);

    // Determine image count (Minimum + 1 for triple buffering)
    uint32_t imageCount = details.capabilities.minImageCount + 1;
    if (details.capabilities.maxImageCount > 0 && imageCount > details.capabilities.maxImageCount) {
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
    FQueueFamilyIndices indices = ChozoUtils::Vulkan::FindQueueFamilies(physicalDevice, m_Surface);
    uint32_t queueFamilyIndices[] = {indices.Graphics.value(), indices.Present.value()};
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

    if (*m_Swapchain) {
        createInfo.oldSwapchain = *m_Swapchain;
    } else {
        createInfo.oldSwapchain = nullptr;
    }

    m_Swapchain = vk::raii::SwapchainKHR(logicalDevice, createInfo);

    // Retrieve the images created by the swapchain
    m_Images = m_Swapchain.getImages();
    m_ImageFormat = surfaceFormat.format;

    m_ImageViews.reserve(m_Images.size());
    for (const auto& image : m_Images) {
        vk::ImageViewCreateInfo viewInfo;
        viewInfo.image = image;
        viewInfo.viewType = vk::ImageViewType::e2D;
        viewInfo.format = m_ImageFormat;

        // [Note] FIX: aspectMask must NOT be 0
        viewInfo.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
        viewInfo.subresourceRange.baseMipLevel = 0;
        viewInfo.subresourceRange.levelCount = 1;
        viewInfo.subresourceRange.baseArrayLayer = 0;
        viewInfo.subresourceRange.layerCount = 1;

        viewInfo.components.r = vk::ComponentSwizzle::eIdentity;
        viewInfo.components.g = vk::ComponentSwizzle::eIdentity;
        viewInfo.components.b = vk::ComponentSwizzle::eIdentity;
        viewInfo.components.a = vk::ComponentSwizzle::eIdentity;

        m_ImageViews.emplace_back(logicalDevice, viewInfo);
    }
    m_Extent = extent;
}

void CVulkanRHISwapchain::CleanupSwapchain() { m_ImageViews.clear(); }

void CVulkanRHISwapchain::RecreateSwapchain() {
    m_Device->GetLogicalDevice().waitIdle();

    // 2. 获取新的窗口大小
    // int width = 0, height = 0;
    // // 假设你持有一个 Window 引用或通过 RHI 获取
    // m_Window->GetFramebufferSize(&width, &height);
    //
    // // [Note] Handle minimization: if size is 0, wait until window is restored
    // while (width == 0 || height == 0) {
    //     m_Window->GetFramebufferSize(&width, &height);
    //     m_Window->WaitEvents(); // 避免空转 CPU
    // }
    //
    // 3. 清理旧资源
    CleanupSwapchain();
    Init();

    // 5. 重新创建 ImageView
    // GetImages() 并循环创建新的 m_ImageViews
    // auto images = m_Swapchain.getImages();
    // for (auto& img : images) {
    //     m_ImageViews.push_back(CreateImageView(img, ...));
    // }
}

void CVulkanRHISwapchain::CreateVKRenderPass() {}

const uint32 CVulkanRHISwapchain::AcquireNextImage(TRef<IRHISyncObject> syncObject) {
    auto vkSync = syncObject.As<CVulkanRHISyncObject>();
    vk::Semaphore semaphore = *vkSync->GetPresentCompleteSemaphore();

    auto resultValue =
        m_Swapchain.acquireNextImage((std::numeric_limits<uint64_t>::max)(), semaphore);

    if (resultValue.result == vk::Result::eErrorOutOfDateKHR ||
        resultValue.result == vk::Result::eSuboptimalKHR) {

        RecreateSwapchain();
        return INVALID_IMAGE_INDEX;
    }

    return resultValue.value;
}

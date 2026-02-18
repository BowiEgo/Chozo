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
    m_ImageCount = details.capabilities.minImageCount + 1;
    if (details.capabilities.maxImageCount > 0 &&
        m_ImageCount > details.capabilities.maxImageCount) {
        m_ImageCount = details.capabilities.maxImageCount;
    }

    vk::SwapchainCreateInfoKHR createInfo;
    createInfo.surface = *m_Surface;
    createInfo.minImageCount = m_ImageCount;
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
    m_DepthFormat = vk::Format::eD32Sfloat;

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
    m_ImageLayouts.assign(m_Images.size(), vk::ImageLayout::eUndefined);

    CreateVKRenderPass();
}

void CVulkanRHISwapchain::CleanupSwapchain() { m_ImageViews.clear(); }

void CVulkanRHISwapchain::RecreateSwapchain() { RecreateSwapchain(m_Info.FrameBufferSize); }

void CVulkanRHISwapchain::CreateVKRenderPass() {

    // 1. Attachment Description
    vk::AttachmentDescription colorAttachment{};
    colorAttachment.format = m_ImageFormat;
    colorAttachment.samples = vk::SampleCountFlagBits::e1;
    colorAttachment.loadOp = vk::AttachmentLoadOp::eClear;
    colorAttachment.storeOp = vk::AttachmentStoreOp::eStore;
    colorAttachment.stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
    colorAttachment.stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
    colorAttachment.initialLayout = vk::ImageLayout::eUndefined;
    colorAttachment.finalLayout = vk::ImageLayout::ePresentSrcKHR;

    // 2. Attachment Reference
    vk::AttachmentReference colorAttachmentRef{};
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout = vk::ImageLayout::eColorAttachmentOptimal;

    // 3. Subpass
    vk::SubpassDescription subpass{};
    subpass.pipelineBindPoint = vk::PipelineBindPoint::eGraphics;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentRef;

    // 4. Subpass Dependency
    vk::SubpassDependency dependency{};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput;
    dependency.srcAccessMask = vk::AccessFlagBits::eNone;
    dependency.dstStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput;
    dependency.dstAccessMask = vk::AccessFlagBits::eColorAttachmentWrite;

    // 5. Create Render Pass
    // vk::RenderPassCreateInfo renderPassInfo{};
    // renderPassInfo.attachmentCount = 1;
    // renderPassInfo.pAttachments = &colorAttachment;
    // renderPassInfo.subpassCount = 1;
    // renderPassInfo.pSubpasses = &subpass;
    // renderPassInfo.dependencyCount = 1;
    // renderPassInfo.pDependencies = &dependency;

    // auto& device = m_Device->GetLogicalDevice();
    // m_RenderPass = vk::raii::RenderPass(device, renderPassInfo);
}

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

void CVulkanRHISwapchain::RecreateSwapchain(const FExtent2D& frameBufferSize) {
    m_Device->GetLogicalDevice().waitIdle();
    m_Info.FrameBufferSize = frameBufferSize;
    CleanupSwapchain();
    Init();

    // 5. 重新创建 ImageView
    // GetImages() 并循环创建新的 m_ImageViews
    // auto images = m_Swapchain.getImages();
    // for (auto& img : images) {
    //     m_ImageViews.push_back(CreateImageView(img, ...));
    // }
}
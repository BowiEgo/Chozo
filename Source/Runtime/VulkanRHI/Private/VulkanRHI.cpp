#include "VulkanRHI.h"
#include "VulkanRHIPipeline.h"
#include "VulkanUtils.h"

DEFINE_LOG_CATEGORY(LogVulkanRHI);

extern "C" {
VULKAN_RHI_API IRHI* CreateVulkanRHI(const FRHICreateInfo& info) { return new CVulkanRHI(info); }
}

static VKAPI_ATTR VkBool32 VKAPI_CALL
    DebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                  VkDebugUtilsMessageTypeFlagsEXT messageType,
                  const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData) {

    // Log the validation layer message based on its severity
    if (messageSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT) {
        CZ_LOG(LogVulkanRHI, Error, "Validation Layer: {0}", pCallbackData->pMessage);
    } else {
        CZ_LOG(LogVulkanRHI, Warning, "Validation Layer: {0}", pCallbackData->pMessage);
    }
    return VK_FALSE; // indicates that the Vulkan call that triggered the
                     // validation layer message should not be aborted
}

CVulkanRHI::CVulkanRHI(const FRHICreateInfo& info) : m_Info(info) { Init(); }
CVulkanRHI::~CVulkanRHI() { CZ_LOG(LogVulkanRHI, Trace, "VulkanRHI destroying..."); }

void CVulkanRHI::Init() {
    CreateVKInstance();
    if constexpr (GIsDebug) {
        SetupVKDebugMessenger();
        CZ_LOG(LogVulkan, Info, "Vulkan Debug Messenger enabled.");
    }
    CreateVKSurface();
}

void CVulkanRHI::CreateVKInstance() {
    // Check validation layer support
    if (ChozoUtils::Vulkan::EnableValidationLayers &&
        !ChozoUtils::Vulkan::CheckValidationLayerSupport(m_Context)) {
        CZ_LOG(LogVulkanRHI, Warning, "Validation layers requested, but not available!");
    }

    // Get required extensions from GLFW
    auto extensions = m_Info.RequiredExtensions;

    if (ChozoUtils::Vulkan::EnableValidationLayers) {
        extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }

    // Check if the required GLFW extensions are supported by the Vulkan
    // implementation.
    if (!ChozoUtils::Vulkan::CheckInstanceExtensions(m_Context, extensions)) {
        throw std::runtime_error("Required Vulkan extensions are not supported!");
    }

    // Fill in ApplicationInfo and InstanceCreateInfo
    const vk::ApplicationInfo appInfo = vk::ApplicationInfo()
                                            .setPApplicationName("Chozo Engine")
                                            .setApplicationVersion(VK_MAKE_VERSION(1, 0, 0))
                                            .setPEngineName("Chozo")
                                            .setEngineVersion(VK_MAKE_VERSION(1, 0, 0))
                                            .setApiVersion(vk::ApiVersion14);

    vk::InstanceCreateInfo createInfo =
        vk::InstanceCreateInfo()
            .setPApplicationInfo(&appInfo)
            .setEnabledExtensionCount(static_cast<uint32_t>(extensions.size()))
            .setPpEnabledExtensionNames(extensions.data());

    // Add validation layers if enabled
    if (ChozoUtils::Vulkan::EnableValidationLayers) {
        createInfo
            .setEnabledLayerCount(
                static_cast<uint32_t>(ChozoUtils::Vulkan::ValidationLayers.size()))
            .setPpEnabledLayerNames(ChozoUtils::Vulkan::ValidationLayers.data());
    }

    // Create RAII Instance
    try {
        m_Instance = vk::raii::Instance(m_Context, createInfo);
        CZ_LOG(LogVulkanRHI, Info, "Vulkan Instance created.");
    } catch (const vk::SystemError& err) {
        CZ_LOG(LogVulkanRHI, Fatal, "Vulkan System Error: {0}", err.what());
    } catch (const std::exception& e) {
        CZ_LOG(LogVulkanRHI, Fatal, "Vulkan Error: {0}", e.what());
    }
}

void CVulkanRHI::SetupVKDebugMessenger() {
    if (!ChozoUtils::Vulkan::EnableValidationLayers)
        return;

    vk::DebugUtilsMessageSeverityFlagsEXT severityFlags(
        vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose |
        vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning |
        vk::DebugUtilsMessageSeverityFlagBitsEXT::eError);

    vk::DebugUtilsMessageTypeFlagsEXT messageTypeFlags(
        vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral |
        vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance |
        vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation);

    vk::DebugUtilsMessengerCreateInfoEXT messengerInfo =
        vk::DebugUtilsMessengerCreateInfoEXT()
            .setMessageSeverity(severityFlags)
            .setMessageType(messageTypeFlags)
            .setPfnUserCallback(DebugCallback); // Static method for handling debug messages

    m_DebugMessenger = vk::raii::DebugUtilsMessengerEXT(m_Instance, messengerInfo);
}

void CVulkanRHI::CreateVKSurface() {
    VkSurfaceKHR surfaceHandle;
    VkResult result;

    auto rawHandle = m_Info.NativeWindow;

    try {
#ifdef CZ_PLATFORM_WINDOWS
        /* Using Vulkan-Hpp Win32 structure */
        vk::Win32SurfaceCreateInfoKHR createInfo({}, GetModuleHandle(nullptr), (HWND)rawHandle);

        // Directly initialize the RAII wrapper
        m_Surface = vk::raii::SurfaceKHR(m_Instance, createInfo);

#elif defined(CZ_PLATFORM_LINUX)
        // Implement Xlib/Wayland logic here...
#elif defined(CZ_PLATFORM_MACOS)
        // Implement Metal/Cocoa logic here...
#endif

        CZ_LOG(LogVulkanRHI, Info, "Vulkan Surface created.");
    } catch (const std::exception& e) {
        CZ_LOG(LogVulkanRHI, Fatal, "Failed to create Window Surface: {0}", e.what());
    }
}

void CVulkanRHI::CreateCommandPool() {
    auto RHIDevice = m_Device.As<CVulkanRHIDevice>();
    uint32 graphicsQueueIndex = RHIDevice->GetGraphicsQueueIndex();

    FRHICommandPoolCreateInfo info;
    info.QueueIndex = graphicsQueueIndex;
    m_MainCommandPool = CreateRef<CVulkanRHICommandPool>(info, m_Device);
}

void CVulkanRHI::BeginRenderingToSwapchain(const TRef<IRHICommandBuffer> cmd, uint32_t imageIndex,
                                           bool bClear) {

    auto vlkCmd = &cmd.As<CVulkanRHICommandBuffer>()->GetVKCommandBuffer();
    auto swapchain = m_Swapchain.As<CVulkanRHISwapchain>();
    vk::Extent2D extent = swapchain->GetVKExtent();
    vk::ClearValue clearColor = vk::ClearColorValue(0.02f, 0.02f, 0.02f, 1.0f);

    m_ImageIndex = imageIndex;

    // Before starting rendering, transition the swapchain image to COLOR_ATTACHMENT_OPTIMAL
    vk::ImageLayout currentLayout = swapchain->GetLayout(imageIndex);
    if (currentLayout != vk::ImageLayout::eColorAttachmentOptimal) {
        TransitionImageLayout(cmd, imageIndex,
                              currentLayout,                              // curLayout
                              vk::ImageLayout::eColorAttachmentOptimal,   // newLayout
                              {},                                         // srcAccess
                              vk::AccessFlagBits2::eColorAttachmentWrite, // dstAccess
                              vk::PipelineStageFlagBits2::eColorAttachmentOutput,
                              vk::PipelineStageFlagBits2::eColorAttachmentOutput);
        swapchain->SetLayout(imageIndex, vk::ImageLayout::eColorAttachmentOptimal);
    }

    // [Note] Configure attachment info dynamically
    vk::RenderingAttachmentInfo colorAttachment;
    colorAttachment.setImageView(swapchain->GetVKImageView(imageIndex));
    colorAttachment.setImageLayout(vk::ImageLayout::eColorAttachmentOptimal);
    // [Note] If bClear is true, we wipe the screen; if false, we draw on top (for UI)
    colorAttachment.setLoadOp(bClear ? vk::AttachmentLoadOp::eClear : vk::AttachmentLoadOp::eLoad);
    colorAttachment.setStoreOp(vk::AttachmentStoreOp::eStore);
    if (bClear) {
        colorAttachment.setClearValue(clearColor);
    }

    vk::RenderingInfo renderingInfo;
    renderingInfo.setRenderArea(vk::Rect2D({0, 0}, extent));
    renderingInfo.setLayerCount(1);
    renderingInfo.setColorAttachments(colorAttachment);

    vlkCmd->beginRendering(renderingInfo);
}

void CVulkanRHI::EndRendering(const TRef<IRHICommandBuffer> cmd) {
    auto vlkCmd = &cmd.As<CVulkanRHICommandBuffer>()->GetVKCommandBuffer();
    vlkCmd->endRendering();

    // After rendering, transition the swapchain image to PRESENT_SRC
    TransitionImageLayout(cmd, m_ImageIndex, vk::ImageLayout::eColorAttachmentOptimal,
                          vk::ImageLayout::ePresentSrcKHR,
                          vk::AccessFlagBits2::eColorAttachmentWrite,         // srcAccessMask
                          {},                                                 // dstAccessMask
                          vk::PipelineStageFlagBits2::eColorAttachmentOutput, // srcStage
                          vk::PipelineStageFlagBits2::eBottomOfPipe           // dstStage
    );

    m_Swapchain.As<CVulkanRHISwapchain>()->SetLayout(m_ImageIndex, vk::ImageLayout::ePresentSrcKHR);
}

void CVulkanRHI::DrawFrame(const TRef<IRHICommandBuffer> commandBuffer,
                           const TRef<IRHISyncObject> syncObject,
                           RecordCallback recordCallback) { // [Note] 增加回调函数
    const auto& queue = m_Device->GetGraphicsQueue();
    const auto& vlkSync = syncObject.As<CVulkanRHISyncObject>();

    // 1. CPU waits for GPU to ensure resource safety
    vlkSync->WaitAndResetFence(m_Device);

    try {
        // 2. accuireNextImage
        auto [result, imageIndex] = m_Swapchain->GetVKSwapchain().acquireNextImage(
            UINT64_MAX, *vlkSync->GetPresentCompleteSemaphore(), nullptr);

        // 3. extute the external recording logic (no longer decided by RHI what to draw)
        if (recordCallback) {
            recordCallback(imageIndex);
        }

        // 4. submit draw command buffer and signal the renderFinishedSemaphore when done
        vk::PipelineStageFlags waitStages = vk::PipelineStageFlagBits::eColorAttachmentOutput;
        vk::SubmitInfo submitInfo;
        submitInfo.setWaitSemaphores(*vlkSync->GetPresentCompleteSemaphore());
        submitInfo.setWaitDstStageMask(waitStages);
        submitInfo.setCommandBuffers(
            *commandBuffer.As<CVulkanRHICommandBuffer>()->GetVKCommandBuffer());
        submitInfo.setSignalSemaphores(*vlkSync->GetRenderFinishedSemaphore());

        queue.submit(submitInfo, *vlkSync->GetDrawFence());

        // 5. present the image, waiting on the renderFinishedSemaphore to ensure rendering is
        // complete
        vk::PresentInfoKHR presentInfo;
        presentInfo.setWaitSemaphores(*vlkSync->GetRenderFinishedSemaphore());
        presentInfo.setSwapchains(*m_Swapchain->GetVKSwapchain());
        presentInfo.setPImageIndices(&imageIndex);

        result = queue.presentKHR(presentInfo);

    } catch (const vk::OutOfDateKHRError& e) {
        m_Swapchain->RecreateSwapchain();
    }
}

void CVulkanRHI::TransitionImageLayout(const TRef<IRHICommandBuffer> cmd, uint32 imageIndex,
                                       vk::ImageLayout old_layout, vk::ImageLayout new_layout,
                                       vk::AccessFlags2 src_access_mask,
                                       vk::AccessFlags2 dst_access_mask,
                                       vk::PipelineStageFlags2 src_stage_mask,
                                       vk::PipelineStageFlags2 dst_stage_mask) {
    if (old_layout == new_layout)
        return;

    auto vlkCmd = &cmd.As<CVulkanRHICommandBuffer>()->GetVKCommandBuffer();

    vk::ImageMemoryBarrier2 barrier;

    barrier.srcStageMask = src_stage_mask;
    barrier.srcAccessMask = src_access_mask;
    barrier.dstStageMask = dst_stage_mask;
    barrier.dstAccessMask = dst_access_mask;
    barrier.oldLayout = old_layout;
    barrier.newLayout = new_layout;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = m_Swapchain->GetVKImages()[imageIndex];
    barrier.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = 1;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;

    vk::DependencyInfo dependency_info;
    dependency_info.dependencyFlags = {};
    dependency_info.imageMemoryBarrierCount = 1;
    dependency_info.pImageMemoryBarriers = &barrier;

    vlkCmd->pipelineBarrier2(dependency_info);
}
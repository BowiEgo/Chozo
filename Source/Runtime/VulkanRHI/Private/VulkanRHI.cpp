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
    if (!ChozoUtils::Vulkan::EnableValidationLayers) return;

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

void CVulkanRHI::DrawFrame(const TRef<IRHICommandBuffer>& cmd,
                           const TRef<IRHISyncObject>& syncObject, RecordCallback recordCallback) {
    const auto queue = m_Device->GetGraphicsQueue();
    const auto vkSync = syncObject.As<CVulkanRHISyncObject>();
    vk::CommandBuffer vkCmd = cmd.As<CVulkanRHICommandBuffer>()->GetVKCommandBuffer();
    auto vkSwapchain = m_Swapchain->GetVKSwapchain();

    // 1. CPU waits for GPU to ensure resource safety
    vkSync->WaitAndResetFence(m_Device);

    try {
        // 2. accuireNextImage
        m_ImageIndex = m_Swapchain->AcquireNextImage(vkSync);

        // 3. extute the external recording logic (no longer decided by RHI what to draw)
        if (recordCallback) {
            recordCallback(m_ImageIndex);
        }

        // 4. submit draw command buffer and signal the renderFinishedSemaphore when done
        vk::PipelineStageFlags waitStages = vk::PipelineStageFlagBits::eColorAttachmentOutput;
        vk::SubmitInfo submitInfo;

        vk::Semaphore waitSemaphore = vkSync->GetPresentCompleteSemaphore();
        vk::Semaphore signalSemaphore = vkSync->GetRenderFinishedSemaphore();
        vk::Fence drawFence = vkSync->GetDrawFence();

        submitInfo.setWaitSemaphores(waitSemaphore)
            .setWaitDstStageMask(waitStages)
            .setCommandBuffers(vkCmd)
            .setSignalSemaphores(signalSemaphore);

        queue.submit({ submitInfo }, drawFence);

        // 5. present the image, waiting on the renderFinishedSemaphore to ensure rendering is
        // complete
        vk::PresentInfoKHR presentInfo;
        presentInfo.setWaitSemaphores(signalSemaphore)
            .setSwapchains(vkSwapchain)
            .setPImageIndices(&m_ImageIndex);

        vk::Result result = queue.presentKHR(presentInfo);

    } catch (const vk::OutOfDateKHRError& e) {
        m_Swapchain->RecreateSwapchain();
    }
}

void CVulkanRHI::BeginRendering(const TRef<IRHICommandBuffer>& cmd,
                                const TRef<IRHITexture2D>& target, bool bClear) {
    m_Target = target.As<CVulkanRHITexture2D>();
    auto oldLayout = m_Target->GetCurrentLayout();
    auto newLayout = vk::ImageLayout::eColorAttachmentOptimal;

    auto vkCmd = cmd.As<CVulkanRHICommandBuffer>()->GetVKCommandBuffer();
    auto swapchain = m_Swapchain.As<CVulkanRHISwapchain>();
    FExtent2D targetSize = target->GetSize();

    vk::Extent2D extent(targetSize.Width, targetSize.Height);
    vk::ClearValue clearColor = vk::ClearColorValue(0.1f, 0.1f, 0.1f, 1.0f);

    // Check layout and transition if necessary
    TransitionTextureLayout(cmd, m_Target, oldLayout, newLayout);

    // Setup rendering attachment
    auto colorAttachmentInfo = m_Target->GetColorAttachmentInfo(clearColor, bClear);

    // Begin rendering...
    vk::RenderingInfo renderingInfo;
    renderingInfo.setRenderArea(vk::Rect2D({ 0, 0 }, extent))
        .setLayerCount(1)
        .setColorAttachmentCount(1)
        .setPColorAttachments(&colorAttachmentInfo); // Explicit pointer pass

    vkCmd.beginRendering(renderingInfo);
}

void CVulkanRHI::EndRendering(const TRef<IRHICommandBuffer>& cmd) {
    vk::CommandBuffer vkCmd = cmd.As<CVulkanRHICommandBuffer>()->GetVKCommandBuffer();
    vkCmd.endRendering();

    // After rendering, transition the swapchain image to PRESENT_SRC
    if (m_Target == m_Swapchain->GetColorAttachment(m_ImageIndex)) {
        auto oldLayout = m_Target->GetCurrentLayout();
        auto newLayout = vk::ImageLayout::ePresentSrcKHR;
        TransitionTextureLayout(cmd, m_Target, oldLayout, newLayout);
    }
}

void CVulkanRHI::PrepareTextureForSampling(const TRef<IRHICommandBuffer>& cmd,
                                           const TRef<IRHITexture2D>& texture) {
    auto vkTexture = texture.As<CVulkanRHITexture2D>();

    // Perform the barrier here while we have access to the command buffer.
    if (vkTexture->GetCurrentLayout() != vk::ImageLayout::eShaderReadOnlyOptimal) {
        TransitionTextureLayout(cmd, vkTexture, vkTexture->GetCurrentLayout(),
                                vk::ImageLayout::eShaderReadOnlyOptimal);
    }
}

void CVulkanRHI::TransitionTextureLayout(const TRef<CVulkanRHICommandBuffer>& cmd,
                                         TRef<CVulkanRHITexture2D>& texture,
                                         vk::ImageLayout oldLayout, vk::ImageLayout newLayout) {
    if (oldLayout == newLayout) return;

    vk::CommandBuffer vkCmd = cmd->GetVKCommandBuffer();
    auto vkImage = texture->GetVKImage();

    vk::ImageMemoryBarrier2 barrier;
    barrier.setOldLayout(oldLayout)
        .setNewLayout(newLayout)
        .setSrcQueueFamilyIndex(vk::QueueFamilyIgnored)
        .setDstQueueFamilyIndex(vk::QueueFamilyIgnored)
        .setImage(vkImage)
        .setSubresourceRange(
            vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1));

    // Automatically deduce stages and access masks based on layouts
    SetupBarrierSync(barrier, oldLayout, newLayout);

    vk::DependencyInfo depInfo;
    depInfo.setImageMemoryBarriers(barrier);

    vkCmd.pipelineBarrier2(depInfo);

    texture->SetCurrentLayout(newLayout);
}

// This function automatically sets srcAccessMask, dstAccessMask, srcStageMask, and dstStageMask
// based on the old and new layouts. For simplicity, we handle common cases here.
void CVulkanRHI::SetupBarrierSync(vk::ImageMemoryBarrier2& barrier, vk::ImageLayout oldLayout,
                                  vk::ImageLayout newLayout) {
    // Default to All Commands if no specific match is found (Safe but slow).
    barrier.setSrcStageMask(vk::PipelineStageFlagBits2::eAllCommands)
        .setSrcAccessMask(vk::AccessFlagBits2::eMemoryWrite)
        .setDstStageMask(vk::PipelineStageFlagBits2::eAllCommands)
        .setDstAccessMask(vk::AccessFlagBits2::eMemoryWrite | vk::AccessFlagBits2::eMemoryRead);

    // 1. From Undefined/Pre-initialized to something
    if (oldLayout == vk::ImageLayout::eUndefined) {
        barrier.setSrcStageMask(vk::PipelineStageFlagBits2::eNone)
            .setSrcAccessMask(vk::AccessFlagBits2::eNone);
    }
    // 2. From RenderTarget (Color Attachment)
    else if (oldLayout == vk::ImageLayout::eColorAttachmentOptimal) {
        barrier.setSrcStageMask(vk::PipelineStageFlagBits2::eColorAttachmentOutput)
            .setSrcAccessMask(vk::AccessFlagBits2::eColorAttachmentWrite);
    }

    // 3. To Present Source (The 1000001002 case)
    if (newLayout == vk::ImageLayout::ePresentSrcKHR) {
        barrier.setDstStageMask(vk::PipelineStageFlagBits2::eBottomOfPipe)
            .setDstAccessMask(vk::AccessFlagBits2::eNone);
    }
    // 4. To Shader Read Only (Combined Image Sampler)
    else if (newLayout == vk::ImageLayout::eShaderReadOnlyOptimal) {
        barrier.setDstStageMask(vk::PipelineStageFlagBits2::eFragmentShader)
            .setDstAccessMask(vk::AccessFlagBits2::eShaderRead);
    }
    // 5. To RenderTarget (Color Attachment)
    else if (newLayout == vk::ImageLayout::eColorAttachmentOptimal) {
        barrier.setDstStageMask(vk::PipelineStageFlagBits2::eColorAttachmentOutput)
            .setDstAccessMask(vk::AccessFlagBits2::eColorAttachmentWrite);
    } else {
        // Handle other layout transitions as needed...
        CZ_LOG(LogVulkanRHI, Warning, "Unsupported layout transition from {0} to {1}",
               (uint32)oldLayout, (uint32)newLayout);
    }
}

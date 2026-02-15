#include "VulkanGraphicsContext.h"

#include "RHIDevice.h"
#include "RHISwapchain.h"
#include "VulkanRHI.h"

DEFINE_LOG_CATEGORY(LogVulkanGraphicsContext);

extern "C" {
VULKAN_RHI_API IGraphicsContext* CreateVulkanGraphicsContext(const FRHIWindowInfo& windowInfo) {
    return new CVulkanGraphicsContext(windowInfo);
}
}

CVulkanGraphicsContext::CVulkanGraphicsContext(const FRHIWindowInfo& windowInfo)
    : IGraphicsContext(windowInfo) {
    Init();
}

CVulkanGraphicsContext::~CVulkanGraphicsContext() {
    CZ_LOG(LogVulkanGraphicsContext, Trace, "Pipeline ref count before clear: {}",
           m_Pipeline->GetRefCount());
}

void CVulkanGraphicsContext::Init() {
    FRHICreateInfo RHIInfo;
    RHIInfo.NativeWindow = m_WindowInfo.NativeWindow;
    RHIInfo.RequiredExtensions = m_WindowInfo.RequiredExtensions;

    m_RHI = CreateScope<CVulkanRHI>(RHIInfo);

    if (m_RHI) {
        CZ_LOG(LogGraphicsContext, Info, "RHI created.");

        FRHIDeviceCreateInfo deviceInfo;
        deviceInfo.AppName = "ChozoEngine";
        deviceInfo.AppVersion = 1;
        m_RHI->CreateDevice(deviceInfo);

        FRHISwapchainCreateInfo swapchainInfo;
        swapchainInfo.FrameBufferSize = m_WindowInfo.FrameBufferSize;
        swapchainInfo.NativeWindow = m_WindowInfo.NativeWindow;
        TRef<IRHISwapchain> swapchain = m_RHI->CreateSwapchain(swapchainInfo);
    }
}

void CVulkanGraphicsContext::SetPipeline(const TRef<IRHIPipeline> pipeline) {
    m_Pipeline = pipeline;
}

void CVulkanGraphicsContext::TransitionImageLayout(uint32 imageIndex, vk::ImageLayout old_layout,
                                                   vk::ImageLayout new_layout,
                                                   vk::AccessFlags2 src_access_mask,
                                                   vk::AccessFlags2 dst_access_mask,
                                                   vk::PipelineStageFlags2 src_stage_mask,
                                                   vk::PipelineStageFlags2 dst_stage_mask) {
    auto swapchain = m_RHI->GetSwapchain().As<CVulkanRHISwapchain>();

    vk::ImageMemoryBarrier2 barrier;

    barrier.srcStageMask = src_stage_mask;
    barrier.srcAccessMask = src_access_mask;
    barrier.dstStageMask = dst_stage_mask;
    barrier.dstAccessMask = dst_access_mask;
    barrier.oldLayout = old_layout;
    barrier.newLayout = new_layout;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = swapchain->GetVKImages()[m_ImageIndex];
    barrier.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = 1;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;

    vk::DependencyInfo dependency_info;
    dependency_info.dependencyFlags = {};
    dependency_info.imageMemoryBarrierCount = 1;
    dependency_info.pImageMemoryBarriers = &barrier;

    m_CurrentVKCmdBuffer->pipelineBarrier2(dependency_info);
}

void CVulkanGraphicsContext::RecordCommandBuffer(const TRef<IRHICommandBuffer> commandBuffer,
                                                 uint32 imageIndex) {
    auto swapchain = m_RHI->GetSwapchain().As<CVulkanRHISwapchain>();
    m_CurrentVKCmdBuffer = &commandBuffer.As<CVulkanRHICommandBuffer>()->GetVKCommandBuffer();
    m_ImageIndex = imageIndex;

    const auto& swapChainImageViews = swapchain->GetVKImageViews();
    auto swapChainExtent = swapchain->GetVKExtent();
    auto graphicsPipeline = &m_Pipeline.As<CVulkanRHIPipeline>()->GetVKPipeline();

    m_CurrentVKCmdBuffer->begin({});
    // Before starting rendering, transition the swapchain image to COLOR_ATTACHMENT_OPTIMAL
    TransitionImageLayout(imageIndex, vk::ImageLayout::eUndefined,
                          vk::ImageLayout::eColorAttachmentOptimal,
                          {}, // srcAccessMask (no need to wait for previous operations)
                          vk::AccessFlagBits2::eColorAttachmentWrite,         // dstAccessMask
                          vk::PipelineStageFlagBits2::eColorAttachmentOutput, // srcStage
                          vk::PipelineStageFlagBits2::eColorAttachmentOutput  // dstStage
    );
    vk::ClearValue clearColor = vk::ClearColorValue(0.02f, 0.02f, 0.02f, 1.0f);

    vk::RenderingAttachmentInfo attachmentInfo;
    attachmentInfo.imageView = swapChainImageViews[imageIndex];
    attachmentInfo.imageLayout = vk::ImageLayout::eColorAttachmentOptimal;
    attachmentInfo.loadOp = vk::AttachmentLoadOp::eClear;
    attachmentInfo.storeOp = vk::AttachmentStoreOp::eStore;
    attachmentInfo.clearValue = clearColor;

    vk::RenderingInfo renderingInfo;
    renderingInfo.renderArea = vk::Rect2D({0, 0}, swapChainExtent);
    renderingInfo.layerCount = 1;
    renderingInfo.colorAttachmentCount = 1;
    renderingInfo.pColorAttachments = &attachmentInfo;

    m_CurrentVKCmdBuffer->beginRendering(renderingInfo);
    m_CurrentVKCmdBuffer->bindPipeline(vk::PipelineBindPoint::eGraphics, *graphicsPipeline);
    m_CurrentVKCmdBuffer->setViewport(
        0, vk::Viewport(0.0f, 0.0f, static_cast<float>(swapChainExtent.width),
                        static_cast<float>(swapChainExtent.height), 0.0f, 1.0f));
    m_CurrentVKCmdBuffer->setScissor(0, vk::Rect2D(vk::Offset2D(0, 0), swapChainExtent));
    m_CurrentVKCmdBuffer->draw(3, 1, 0, 0);
    m_CurrentVKCmdBuffer->endRendering();
    // After rendering, transition the swapchain image to PRESENT_SRC
    TransitionImageLayout(imageIndex, vk::ImageLayout::eColorAttachmentOptimal,
                          vk::ImageLayout::ePresentSrcKHR,
                          vk::AccessFlagBits2::eColorAttachmentWrite,         // srcAccessMask
                          {},                                                 // dstAccessMask
                          vk::PipelineStageFlagBits2::eColorAttachmentOutput, // srcStage
                          vk::PipelineStageFlagBits2::eBottomOfPipe           // dstStage
    );
    m_CurrentVKCmdBuffer->end();
}

void CVulkanGraphicsContext::DrawFrame(const TRef<IRHICommandBuffer> commandBuffer,
                                       const TRef<IRHISyncObject> syncObject) {
    auto RHIDevice = (m_RHI->GetDevice()).As<CVulkanRHIDevice>();
    const auto& device = RHIDevice->GetLogicalDevice();
    const auto& queue = RHIDevice->GetGraphicsQueue();
    const auto& drawFence = syncObject.As<CVulkanRHISyncObject>()->GetDrawFence();
    const auto& renderFinishedSemaphore =
        syncObject.As<CVulkanRHISyncObject>()->GetRenderFinishedSemaphore();
    const auto& presentCompleteSemaphore =
        syncObject.As<CVulkanRHISyncObject>()->GetPresentCompleteSemaphore();
    const auto& swapchain = m_RHI->GetSwapchain().As<CVulkanRHISwapchain>()->GetVKSwapchain();
    queue.waitIdle(); // NOTE: for simplicity, wait for the queue to be idle before starting the
                      // frame In the next chapter you see how to use multiple frames in flight and
                      // fences to sync

    try {
        auto [result, imageIndex] =
            swapchain.acquireNextImage(UINT64_MAX, *presentCompleteSemaphore, nullptr);
        RecordCommandBuffer(commandBuffer, imageIndex);

        device.resetFences(*drawFence);
        vk::PipelineStageFlags waitDestinationStageMask(
            vk::PipelineStageFlagBits::eColorAttachmentOutput);

        vk::SubmitInfo submitInfo;
        submitInfo.setWaitSemaphores(*presentCompleteSemaphore);
        submitInfo.setWaitDstStageMask(waitDestinationStageMask);
        vk::CommandBuffer cmd = *commandBuffer.As<CVulkanRHICommandBuffer>()->GetVKCommandBuffer();
        submitInfo.setCommandBuffers(cmd);
        submitInfo.setSignalSemaphores(*renderFinishedSemaphore);

        queue.submit(submitInfo, *drawFence);

        result = device.waitForFences(*drawFence, vk::True, UINT64_MAX);
        if (result != vk::Result::eSuccess) {
            CZ_LOG(LogVulkanGraphicsContext, Error, "failed to wait for fence!");
        }

        vk::PresentInfoKHR presentInfoKHR;
        presentInfoKHR.setWaitSemaphores(*renderFinishedSemaphore);
        presentInfoKHR.setSwapchains(*swapchain);
        presentInfoKHR.setPImageIndices(&imageIndex);

        result = queue.presentKHR(presentInfoKHR);
    } catch (const vk::OutOfDateKHRError& e) {
        CZ_LOG(LogVulkanGraphicsContext, Warning, "Swapchain out of date, recreating...");
        m_RHI->GetSwapchain().As<CVulkanRHISwapchain>()->RecreateSwapchain();
    } catch (const std::exception& e) {
        CZ_LOG(LogVulkanGraphicsContext, Error, "Render error: {0}", e.what());
    }
}

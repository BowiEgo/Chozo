#include "VulkanAPI.h"

#include "VulkanCommandBuffer.h"

DEFINE_LOG_CATEGORY(LogVulkanAPI);

CVulkanAPI::CVulkanAPI() {}

CVulkanAPI::~CVulkanAPI() { CZ_LOG(LogVulkanAPI, Trace, "VulkanAPI destroying..."); }

void CVulkanAPI::DrawFrame_Internal(IRHIContext* ctx, const TRef<IRHICommandList>& cmdBuffer,
                                    TRef<IRHISyncObject>& syncObject,
                                    RecordCallback recordCallback) {
    auto currentFrame = ctx->GetCurrentFrameIndex();
    auto device = ctx->GetDevice();
    auto swapchain = ctx->GetSwapchain().As<CVulkanSwapchain>();
    auto queue = device.As<CVulkanDevice>()->GetGraphicsQueue();
    auto vkSync = syncObject.As<CVulkanSyncObject>();
    vk::CommandBuffer vkCmdBuffer = cmdBuffer.As<CVulkanCommandBuffer>()->GetVKCommandBuffer();

    if (swapchain->RecreateIfNeeded()) {
        vkSync->RecreateSemaphores(device);
        return;
    }
    auto vkSwapchain = swapchain->GetVKSwapchain();

    // 1. CPU waits for GPU to ensure resource safety
    vkSync->WaitAndResetFence(device);

    // 2. accuireNextImage
    uint32 inFlightIndex = currentFrame % swapchain->GetImageCount();
    vk::Semaphore acquireWaitSem = swapchain->GetImageAvailableSemaphore(inFlightIndex);

    uint32 imgIdx = swapchain->AcquireNextImage(acquireWaitSem);
    ctx->SetCurrentImageIndex(imgIdx);
    if (imgIdx == INVALID_IMAGE_INDEX) return;

    // 3. extute the external recording logic (no longer decided by RHI what to draw)
    if (recordCallback) {
        recordCallback(imgIdx);
    }

    // 4. submit draw command buffer and signal the renderFinishedSemaphore when done
    vk::PipelineStageFlags waitStages = vk::PipelineStageFlagBits::eColorAttachmentOutput;
    vk::SubmitInfo submitInfo;

    vk::Semaphore imageSigSem = swapchain->GetRenderFinishedSemaphore(imgIdx);
    vk::Fence fence = vkSync->GetVKFence();

    submitInfo.setWaitSemaphores(acquireWaitSem)
        .setWaitDstStageMask(waitStages)
        .setCommandBuffers(vkCmdBuffer)
        .setSignalSemaphores(imageSigSem);

    queue.submit({ submitInfo }, fence);

    // 5. present the image, waiting on the renderFinishedSemaphore to ensure rendering is
    // complete
    vk::PresentInfoKHR presentInfo;
    presentInfo.setWaitSemaphores(imageSigSem).setSwapchains(vkSwapchain).setPImageIndices(&imgIdx);

    vk::Result result;
    try {
        result = queue.presentKHR(presentInfo);
    } catch (const vk::OutOfDateKHRError& e) {
        result = vk::Result::eErrorOutOfDateKHR;
    }

    if (result == vk::Result::eErrorOutOfDateKHR || result == vk::Result::eSuboptimalKHR) {
        swapchain->Recreate();
    }
}

void CVulkanAPI::BeginRendering_Internal(const IRHIContext* ctx,
                                         const TRef<IRHICommandList>& cmdBuffer, bool bClear) {
    auto target = ctx->GetTarget().As<CVulkanTexture2D>();
    auto oldLayout = target->GetCurrentLayout();
    auto newLayout = vk::ImageLayout::eColorAttachmentOptimal;

    auto vkCmdBuffer = cmdBuffer.As<CVulkanCommandBuffer>()->GetVKCommandBuffer();
    auto swapchain = ctx->GetSwapchain().As<CVulkanSwapchain>();
    FExtent2D targetSize = target->GetSize();

    vk::Extent2D extent(targetSize.Width, targetSize.Height);
    vk::ClearValue clearColor = vk::ClearColorValue(0.1f, 0.1f, 0.1f, 1.0f);

    // Check layout and transition if necessary
    ChozoUtils::Vulkan::TransitionTextureLayout(vkCmdBuffer, target->GetVKImage(), oldLayout,
                                                newLayout);
    target->SetCurrentLayout(newLayout);

    // Setup rendering attachment
    auto colorAttachmentInfo = target->GetColorAttachmentInfo(clearColor, bClear);

    // Begin rendering...
    vk::RenderingInfo renderingInfo;
    renderingInfo.setRenderArea(vk::Rect2D({ 0, 0 }, extent))
        .setLayerCount(1)
        .setColorAttachmentCount(1)
        .setPColorAttachments(&colorAttachmentInfo); // Explicit pointer pass

    vkCmdBuffer.beginRendering(renderingInfo);
}

void CVulkanAPI::EndRendering_Internal(const IRHIContext* ctx,
                                       const TRef<IRHICommandList>& cmdBuffer) {
    auto swapchain = ctx->GetSwapchain().As<CVulkanSwapchain>();
    auto target = ctx->GetTarget().As<CVulkanTexture2D>();
    auto vkCmdBuffer = cmdBuffer.As<CVulkanCommandBuffer>()->GetVKCommandBuffer();

    vkCmdBuffer.endRendering();
    uint32 imgIdx = ctx->GetCurrentImageIndex();

    // After rendering, transition the swapchain image to PRESENT_SRC
    if (target == swapchain->GetColorAttachment(imgIdx)) {
        auto oldLayout = target->GetCurrentLayout();
        auto newLayout = vk::ImageLayout::ePresentSrcKHR;
        ChozoUtils::Vulkan::TransitionTextureLayout(vkCmdBuffer, target->GetVKImage(), oldLayout,
                                                    newLayout);
        target->SetCurrentLayout(newLayout);
    }
}

void CVulkanAPI::PrepareTextureForSampling_Internal(const IRHIContext* ctx,
                                                    const TRef<IRHICommandList>& cmdBuffer,
                                                    const TRef<IRHITexture2D>& texture) {
    auto vkCmdBuffer = cmdBuffer.As<CVulkanCommandBuffer>()->GetVKCommandBuffer();
    auto vulkanTexture = texture.As<CVulkanTexture2D>();
    auto oldLayout = vulkanTexture->GetCurrentLayout();
    auto newLayout = vk::ImageLayout::eShaderReadOnlyOptimal;

    // Perform the barrier here while we have access to the command buffer.
    if (oldLayout != vk::ImageLayout::eShaderReadOnlyOptimal) {
        ChozoUtils::Vulkan::TransitionTextureLayout(vkCmdBuffer, vulkanTexture->GetVKImage(),
                                                    oldLayout, newLayout);
        vulkanTexture->SetCurrentLayout(newLayout);
    }
}

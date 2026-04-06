#include "VulkanAPI.h"

#include "VulkanCommandBuffer.h"
#include "VulkanImage.h"

DEFINE_LOG_CATEGORY(LogVulkanAPI);

CVulkanAPI::CVulkanAPI() {}

CVulkanAPI::~CVulkanAPI() { CZ_LOG(LogVulkanAPI, Trace, "VulkanAPI destroying..."); }

// void CVulkanAPI::DrawFrame_Internal(IRHIContext* ctx, const TRef<IRHICommandList>& cmdBuffer,
//                                     TRef<IRHISyncObject>& syncObject,
//                                     RecordCallback recordCallback) {
//     auto currentFrame             = ctx->GetCurrentFrameIndex();
//     auto device                   = ctx->GetDevice().As<CVulkanDevice>();
//     auto swapchain                = ctx->GetSwapchain().As<CVulkanSwapchain>();
//     auto queue                    = device->GetGraphicsQueue();
//     auto vkSync                   = syncObject.As<CVulkanSyncObject>();
//     vk::CommandBuffer vkCmdBuffer = cmdBuffer.As<CVulkanCommandBuffer>()->GetVKCommandBuffer();

//     if (vkSync->WasJustRecreated()) {
//         CZ_LOG(LogVulkanAPI, Trace,
//                "Semaphores were just recreated, skipping one frame to stabilize");
//         vkSync->ClearRecreatedFlag();
//         return;
//     }

//     if (swapchain->RecreateIfNeeded()) {
//         vkSync->RecreateSemaphores(device);
//         return;
//     }

//     auto vkSwapchain = swapchain->GetVKSwapchain();

//     // 1. CPU waits for GPU to ensure resource safety
//     vk::Result waitResult = vkSync->WaitAndResetFence(device, UINT32_MAX);
//     if (waitResult != vk::Result::eSuccess) {
//         CZ_LOG(LogVulkanAPI, Error, "Failed to wait for fence: {}", vk::to_string(waitResult));
//         vkSync->RecreateSemaphores(device);
//         return;
//     }

//     // 2. accuireNextImage
//     uint32 inFlightIndex         = currentFrame % swapchain->GetImageCount();
//     vk::Semaphore acquireWaitSem = swapchain->GetImageAvailableSemaphore(inFlightIndex);

//     uint32 imgIdx        = INVALID_IMAGE_INDEX;
//     int retryCount       = 0;
//     const int maxRetries = 3;

//     while (retryCount < maxRetries) {
//         imgIdx = swapchain->AcquireNextImage(acquireWaitSem);

//         if (imgIdx != INVALID_IMAGE_INDEX) {
//             break;
//         }

//         CZ_LOG(LogVulkanAPI, Warning, "Failed to acquire image (attempt {}/{})", retryCount + 1,
//                maxRetries);
//         retryCount++;

//         std::this_thread::sleep_for(std::chrono::milliseconds(1));
//     }

//     if (imgIdx == INVALID_IMAGE_INDEX) {
//         CZ_LOG(LogVulkanAPI, Error, "Failed to acquire image after {} attempts", maxRetries);
//         return;
//     }

//     ctx->SetCurrentImageIndex(imgIdx);

//     // 3. extute the external recording logic (no longer decided by RHI what to draw)
//     if (recordCallback) {
//         recordCallback(imgIdx);
//     }

//     // 4. submit draw command buffer and signal the renderFinishedSemaphore when done
//     vk::PipelineStageFlags waitStages = vk::PipelineStageFlagBits::eColorAttachmentOutput;
//     vk::SubmitInfo submitInfo;

//     vk::Semaphore imageSigSem = swapchain->GetRenderFinishedSemaphore(imgIdx);
//     vk::Fence fence           = vkSync->GetVKFence();

//     submitInfo.setWaitSemaphores(acquireWaitSem)
//         .setWaitDstStageMask(waitStages)
//         .setCommandBuffers(vkCmdBuffer)
//         .setSignalSemaphores(imageSigSem);

//     try {
//         queue.submit({ submitInfo }, fence);
//     } catch (const vk::Error& e) {
//         CZ_LOG(LogVulkanAPI, Error, "Submit failed: {}", e.what());
//         vkSync->RecreateSemaphores(device);
//         return;
//     }

//     // 5. present the image, waiting on the renderFinishedSemaphore to ensure rendering is
//     // complete
//     vk::PresentInfoKHR presentInfo;
//     presentInfo.setWaitSemaphores(imageSigSem).setSwapchains(vkSwapchain).setPImageIndices(&imgIdx);

//     vk::Result result;
//     try {
//         result = queue.presentKHR(presentInfo);
//     } catch (const vk::OutOfDateKHRError& e) {
//         result = vk::Result::eErrorOutOfDateKHR;
//     } catch (const vk::Error& e) {
//         CZ_LOG(LogVulkanAPI, Error, "Present failed: {}", e.what());
//         result = vk::Result::eErrorUnknown;
//     }

//     if (result == vk::Result::eErrorOutOfDateKHR) {
//         CZ_LOG(LogVulkanAPI, Info, "Swapchain out of date, will recreate");
//         swapchain->MarkNeedsRecreation();
//     } else if (result == vk::Result::eSuboptimalKHR) {
//         CZ_LOG(LogVulkanAPI, Warning, "Swapchain suboptimal, may need recreation");
//         swapchain->MarkNeedsRecreation();
//     } else if (result != vk::Result::eSuccess) {
//         CZ_LOG(LogVulkanAPI, Error, "Present failed with unexpected result: {}",
//                vk::to_string(result));
//     }
// }

// void CVulkanAPI::BeginRendering_Internal(const IRHIContext* ctx,
//                                          const TRef<IRHICommandList>& cmdBuffer, bool bClear) {
//     auto target      = ctx->GetTarget().As<CVulkanTexture2D>();
//     auto targetImage = target->GetImage().As<CVulkanImage>();
//     auto oldLayout   = targetImage->GetCurrentLayout();
//     auto newLayout   = vk::ImageLayout::eColorAttachmentOptimal;

//     auto vkCmdBuffer     = cmdBuffer.As<CVulkanCommandBuffer>()->GetVKCommandBuffer();
//     auto swapchain       = ctx->GetSwapchain().As<CVulkanSwapchain>();
//     FExtent2D targetSize = target->GetSize();

//     vk::Extent2D extent(targetSize.Width, targetSize.Height);
//     vk::ClearValue clearColor = vk::ClearColorValue(0.1f, 0.1f, 0.1f, 1.0f);

//     // Check layout and transition if necessary
//     ChozoUtils::Vulkan::TransitionImageLayout(vkCmdBuffer, targetImage->GetVKImage(), oldLayout,
//                                               newLayout);
//     targetImage->SetCurrentLayout(newLayout);

//     // Setup rendering attachment
//     auto colorAttachmentInfo = target->GetColorAttachmentInfo(clearColor, bClear);

//     // Begin rendering...
//     vk::RenderingInfo renderingInfo;
//     renderingInfo.setRenderArea(vk::Rect2D({ 0, 0 }, extent))
//         .setLayerCount(1)
//         .setColorAttachmentCount(1)
//         .setPColorAttachments(&colorAttachmentInfo); // Explicit pointer pass

//     vkCmdBuffer.beginRendering(renderingInfo);
// }

// void CVulkanAPI::EndRendering_Internal(const IRHIContext* ctx,
//                                        const TRef<IRHICommandList>& cmdBuffer) {
//     auto swapchain   = ctx->GetSwapchain().As<CVulkanSwapchain>();
//     auto target      = ctx->GetTarget().As<CVulkanTexture2D>();
//     auto targetImage = target->GetImage().As<CVulkanImage>();
//     auto vkCmdBuffer = cmdBuffer.As<CVulkanCommandBuffer>()->GetVKCommandBuffer();

//     vkCmdBuffer.endRendering();
//     uint32 imgIdx = ctx->GetCurrentImageIndex();

//     // After rendering, transition the swapchain image to PRESENT_SRC
//     auto targetVKImage = targetImage->GetVKImage();
//     auto swapchainVKImage =
//         swapchain->GetColorAttachment(imgIdx)->GetImage().As<CVulkanImage>()->GetVKImage();

//     uint64_t h1 = (uint64_t)(VkImage)targetVKImage;
//     uint64_t h2 = (uint64_t)(VkImage)swapchainVKImage;

//     if (targetVKImage == swapchainVKImage) {
//         auto oldLayout = targetImage->GetCurrentLayout();
//         auto newLayout = vk::ImageLayout::ePresentSrcKHR;
//         ChozoUtils::Vulkan::TransitionImageLayout(vkCmdBuffer, targetVKImage, oldLayout,
//         newLayout); targetImage->SetCurrentLayout(newLayout);
//     }
// }

void CVulkanAPI::BeginRendering_Internal(const IRHIContext* ctx,
                                         const TRef<IRHICommandList>& cmdBuffer, bool bClear) {
    const auto& targets = ctx->GetRenderTargets();
    if (targets.empty()) return;

    auto vkCmdBuffer = cmdBuffer.As<CVulkanCommandBuffer>()->GetVKCommandBuffer();

    FExtent2D firstSize = targets[0]->GetSize();
    vk::Extent2D extent(firstSize.Width, firstSize.Height);
    vk::ClearValue clearValue = vk::ClearColorValue(0.1f, 0.1f, 0.1f, 1.0f);

    // Setup rendering attachment
    std::vector<vk::RenderingAttachmentInfo> colorAttachmentInfos;
    colorAttachmentInfos.reserve(targets.size());

    for (const auto& target : targets) {
        colorAttachmentInfos.push_back(target.As<CVulkanTexture2D>()->GetColorAttachmentInfo(
            clearValue, bClear)); // TODO: target owns clearValue and get from it
    }

    vk::RenderingInfo renderingInfo;
    renderingInfo.setRenderArea(vk::Rect2D({ 0, 0 }, extent))
        .setLayerCount(1)
        .setColorAttachmentCount(static_cast<uint32_t>(colorAttachmentInfos.size()))
        .setPColorAttachments(colorAttachmentInfos.data());

    // Optional: Handle DepthAttachment
    // renderingInfo.setPDepthAttachment(&depthAttachmentInfo);

    vkCmdBuffer.beginRendering(renderingInfo);
}

void CVulkanAPI::DrawFrame_Internal(IRHIContext* ctx, const TRef<IRHICommandList>& cmdBuffer,
                                    TRef<IRHISyncObject>& syncObject,
                                    RecordCallback recordCallback) {
    auto currentFrame             = ctx->GetCurrentFrameIndex();
    auto device                   = ctx->GetDevice().As<CVulkanDevice>();
    auto swapchain                = ctx->GetSwapchain().As<CVulkanSwapchain>();
    auto queue                    = device->GetGraphicsQueue();
    auto vkSync                   = syncObject.As<CVulkanSyncObject>();
    vk::CommandBuffer vkCmdBuffer = cmdBuffer.As<CVulkanCommandBuffer>()->GetVKCommandBuffer();

    if (vkSync->WasJustRecreated()) {
        CZ_LOG(LogVulkanAPI, Trace,
               "Semaphores were just recreated, skipping one frame to stabilize");
        vkSync->ClearRecreatedFlag();
        return;
    }

    if (swapchain->RecreateIfNeeded()) {
        vkSync->RecreateSemaphores(device);
        return;
    }

    auto vkSwapchain = swapchain->GetVKSwapchain();

    // 1. CPU waits for GPU to ensure resource safety
    vk::Result waitResult = vkSync->WaitAndResetFence(device, UINT32_MAX);
    if (waitResult != vk::Result::eSuccess) {
        CZ_LOG(LogVulkanAPI, Error, "Failed to wait for fence: {}", vk::to_string(waitResult));
        vkSync->RecreateSemaphores(device);
        return;
    }

    // 2. accuireNextImage
    uint32 inFlightIndex         = currentFrame % swapchain->GetImageCount();
    vk::Semaphore acquireWaitSem = swapchain->GetImageAvailableSemaphore(inFlightIndex);

    uint32 imgIdx        = INVALID_IMAGE_INDEX;
    int retryCount       = 0;
    const int maxRetries = 3;

    while (retryCount < maxRetries) {
        imgIdx = swapchain->AcquireNextImage(acquireWaitSem);

        if (imgIdx != INVALID_IMAGE_INDEX) {
            break;
        }

        CZ_LOG(LogVulkanAPI, Warning, "Failed to acquire image (attempt {}/{})", retryCount + 1,
               maxRetries);
        retryCount++;

        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }

    if (imgIdx == INVALID_IMAGE_INDEX) {
        CZ_LOG(LogVulkanAPI, Error, "Failed to acquire image after {} attempts", maxRetries);
        return;
    }

    ctx->SetCurrentImageIndex(imgIdx);

    // 3. extute the external recording logic (no longer decided by RHI what to draw)
    if (recordCallback) {
        recordCallback(imgIdx);
    }

    // 4. submit draw command buffer and signal the renderFinishedSemaphore when done
    vk::PipelineStageFlags waitStages = vk::PipelineStageFlagBits::eColorAttachmentOutput;
    vk::SubmitInfo submitInfo;

    vk::Semaphore imageSigSem = swapchain->GetRenderFinishedSemaphore(imgIdx);
    vk::Fence fence           = vkSync->GetVKFence();

    submitInfo.setWaitSemaphores(acquireWaitSem)
        .setWaitDstStageMask(waitStages)
        .setCommandBuffers(vkCmdBuffer)
        .setSignalSemaphores(imageSigSem);

    try {
        queue.submit({ submitInfo }, fence);
    } catch (const vk::Error& e) {
        CZ_LOG(LogVulkanAPI, Error, "Submit failed: {}", e.what());
        vkSync->RecreateSemaphores(device);
        return;
    }

    // 5. present the image, waiting on the renderFinishedSemaphore to ensure rendering is
    // complete
    vk::PresentInfoKHR presentInfo;
    presentInfo.setWaitSemaphores(imageSigSem).setSwapchains(vkSwapchain).setPImageIndices(&imgIdx);

    vk::Result result;
    try {
        result = queue.presentKHR(presentInfo);
    } catch (const vk::OutOfDateKHRError& e) {
        result = vk::Result::eErrorOutOfDateKHR;
    } catch (const vk::Error& e) {
        CZ_LOG(LogVulkanAPI, Error, "Present failed: {}", e.what());
        result = vk::Result::eErrorUnknown;
    }

    if (result == vk::Result::eErrorOutOfDateKHR) {
        CZ_LOG(LogVulkanAPI, Info, "Swapchain out of date, will recreate");
        swapchain->MarkNeedsRecreation();
    } else if (result == vk::Result::eSuboptimalKHR) {
        CZ_LOG(LogVulkanAPI, Warning, "Swapchain suboptimal, may need recreation");
        swapchain->MarkNeedsRecreation();
    } else if (result != vk::Result::eSuccess) {
        CZ_LOG(LogVulkanAPI, Error, "Present failed with unexpected result: {}",
               vk::to_string(result));
    }
}

void CVulkanAPI::EndRendering_Internal(const IRHIContext* ctx,
                                       const TRef<IRHICommandList>& cmdBuffer) {
    auto vkCmdBuffer = cmdBuffer.As<CVulkanCommandBuffer>()->GetVKCommandBuffer();
    vkCmdBuffer.endRendering();
}

void CVulkanAPI::TransitionImageLayout_Internal(const IRHIContext* ctx,
                                                const TRef<IRHICommandList>& cmdBuffer,
                                                const TRef<IRHIImage> image,
                                                const EImageLayout newLayout) {
    auto vkCmdBuffer = cmdBuffer.As<CVulkanCommandBuffer>()->GetVKCommandBuffer();
    auto vkImage     = image.As<CVulkanImage>()->GetVKImage();
    auto vkOldLayout = image.As<CVulkanImage>()->GetCurrentLayout();
    auto vkNewLayout = ChozoUtils::Vulkan::ToVkImageLayout(newLayout);

    ChozoUtils::Vulkan::TransitionImageLayout(vkCmdBuffer, vkImage, vkOldLayout, vkNewLayout);
    image.As<CVulkanImage>()->SetCurrentLayout(vkNewLayout);
}

void CVulkanAPI::PrepareTextureForSampling_Internal(const IRHIContext* ctx,
                                                    const TRef<IRHICommandList>& cmdBuffer,
                                                    const TRef<IRHITexture2D>& texture) {
    auto vkCmdBuffer = cmdBuffer.As<CVulkanCommandBuffer>()->GetVKCommandBuffer();
    auto image       = texture->GetImage().As<CVulkanImage>();
    auto oldLayout   = image->GetCurrentLayout();
    auto newLayout   = vk::ImageLayout::eShaderReadOnlyOptimal;

    // Perform the barrier here while we have access to the command buffer.
    if (oldLayout != vk::ImageLayout::eShaderReadOnlyOptimal) {
        ChozoUtils::Vulkan::TransitionImageLayout(vkCmdBuffer, image->GetVKImage(), oldLayout,
                                                  newLayout);
        image->SetCurrentLayout(newLayout);
    }
}

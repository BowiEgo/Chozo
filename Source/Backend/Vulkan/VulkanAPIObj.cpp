#include "VulkanAPIObj.hpp"
#include "VulkanCommandBufferObj.hpp"
#include "VulkanFenceObj.hpp"
#include "VulkanGraphicsContextObj.hpp"
#include "VulkanImageObj.hpp"
#include "VulkanSemaphoreObj.hpp"
#include "VulkanTextureObj.hpp"

namespace CZ {

void VulkanAPIObj::BeginRendering(CommandList cmdList, std::vector<Texture>& targets, bool bClear,
                                  uint32_t faceIndex) {
    if (targets.empty()) return;

    VkCommandBuffer vkCmdBuffer = cmdList.As<VulkanCommandBufferObj>()->GetVkCommandBuffer();

    Extent2D firstSize = targets[0]->GetSize();
    VkExtent2D extent{ firstSize.Width, firstSize.Height };

    VkClearValue clearValue{};
    clearValue.color.float32[0] = 0.1f;
    clearValue.color.float32[1] = 0.1f;
    clearValue.color.float32[2] = 0.1f;
    clearValue.color.float32[3] = 1.0f;

    std::vector<VkRenderingAttachmentInfo> colorAttachmentInfos;
    colorAttachmentInfos.reserve(targets.size());

    for (auto& target : targets) {
        auto tex  = target.As<VulkanTextureObj>();
        auto info = tex->GetColorAttachmentInfo(clearValue, bClear, faceIndex);
        colorAttachmentInfos.push_back(info);
    }

    VkRenderingInfo renderingInfo{};
    renderingInfo.sType                = VK_STRUCTURE_TYPE_RENDERING_INFO;
    renderingInfo.renderArea           = VkRect2D{ { 0, 0 }, extent };
    renderingInfo.layerCount           = 1;
    renderingInfo.colorAttachmentCount = static_cast<uint32_t>(colorAttachmentInfos.size());
    renderingInfo.pColorAttachments    = colorAttachmentInfos.data();

    // 如有深度附件可在此设置：
    // renderingInfo.pDepthAttachment = &depthAttachmentInfo;

    vkCmdBeginRendering(vkCmdBuffer, &renderingInfo);
}

void VulkanAPIObj::DrawFrame(CommandList cmdList, RecordCallback recordCallback) {
    auto currentFrameIdx = m_GraphicsContext->GetCurrentFrameIndex();

    auto vkGraphicsCtx = m_GraphicsContext.As<VulkanGraphicsContextObj>();

    auto device    = vkGraphicsCtx->m_DeviceObj;
    auto swapchain = vkGraphicsCtx->m_SwapchainObj;

    auto vkQueue     = device->GetGraphicsQueue();
    auto vkSwapchain = swapchain->GetVkSwapchain();

    auto fence      = swapchain->GetFence(currentFrameIdx);
    VkFence vkFence = fence.As<VulkanFenceObj>()->GetVKFence();

    VkCommandBuffer vkCmdBuffer = cmdList.As<VulkanCommandBufferObj>()->GetVkCommandBuffer();

    // if (vkSync->WasJustRecreated()) {
    //     CZ_BACKEND_LOG(Trace,
    //            "Semaphores were just recreated, skipping one frame to stabilize");
    //     vkSync->ClearRecreatedFlag();
    //     return;
    // }

    // if (swapchain->RecreateIfNeeded()) {
    //     vkSync->RecreateSemaphores(device);
    //     return;
    // }

    // 1. CPU wait GPU make resources safety
    bool waitSuccess = fence->WaitAndReset(UINT32_MAX);
    if (!waitSuccess) {
        CZ_BACKEND_LOG(Error, "Failed to wait for fence");
        // vkSync->RecreateSemaphores(device);
        return;
    }

    // 2. Acquire next available image
    Semaphore acquireWaitSem     = swapchain->GetImageAvailableSemaphore(currentFrameIdx);
    VkSemaphore vkAcquireWaitSem = acquireWaitSem.As<VulkanSemaphoreObj>()->GetVkSemaphore();

    uint32_t imgIdx      = INVALID_IMAGE_INDEX;
    int retryCount       = 0;
    const int maxRetries = 3;

    while (retryCount < maxRetries) {
        imgIdx = swapchain->AcquireNextImageIndex(vkAcquireWaitSem);
        if (imgIdx != INVALID_IMAGE_INDEX) break;

        CZ_BACKEND_LOG(Warning, "Failed to acquire image (attempt {}/{})", retryCount + 1,
                       maxRetries);
        retryCount++;
        // std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }

    if (imgIdx == INVALID_IMAGE_INDEX) {
        CZ_BACKEND_LOG(Error, "Failed to acquire image after {} attempts", maxRetries);
        return;
    }

    swapchain->SetCurrentImageIndex(imgIdx);

    // 3. 执行外部录制回调（RHI 不再决定绘制内容）
    if (recordCallback) {
        recordCallback(imgIdx);
    }

    // 4. 提交绘制命令缓冲区，并在完成时发出 renderFinishedSemaphore
    Semaphore imageSigSem     = swapchain->GetRenderFinishedSemaphore(imgIdx);
    VkSemaphore vkImageSigSem = imageSigSem.As<VulkanSemaphoreObj>()->GetVkSemaphore();

    VkPipelineStageFlags waitStages = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    submitInfo.waitSemaphoreCount   = 1;
    submitInfo.pWaitSemaphores      = &vkAcquireWaitSem;
    submitInfo.pWaitDstStageMask    = &waitStages;
    submitInfo.commandBufferCount   = 1;
    submitInfo.pCommandBuffers      = &vkCmdBuffer;
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores    = &vkImageSigSem;

    VkResult submitResult = vkQueueSubmit(vkQueue, 1, &submitInfo, vkFence);
    if (submitResult != VK_SUCCESS) {
        CZ_BACKEND_LOG(Error, "Submit failed: {}", VulkanUtils::VkResultToString(submitResult));
        // acquireWaitSem.As<VulkanSemaphoreObj>()->Recreate();
        return;
    }

    // 5. 呈现图像，等待 renderFinishedSemaphore 以确保渲染结束
    VkPresentInfoKHR presentInfo{};
    presentInfo.sType              = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores    = &vkImageSigSem;
    presentInfo.swapchainCount     = 1;
    presentInfo.pSwapchains        = &vkSwapchain;
    presentInfo.pImageIndices      = &imgIdx;

    VkResult presentResult = vkQueuePresentKHR(vkQueue, &presentInfo);

    if (presentResult == VK_ERROR_OUT_OF_DATE_KHR) {
        CZ_BACKEND_LOG(Info, "Swapchain out of date, will recreate");
        swapchain->MarkNeedsRecreation();
    } else if (presentResult == VK_SUBOPTIMAL_KHR) {
        CZ_BACKEND_LOG(Warning, "Swapchain suboptimal, may need recreation");
        swapchain->MarkNeedsRecreation();
    } else if (presentResult != VK_SUCCESS) {
        CZ_BACKEND_LOG(Error, "Present failed with unexpected result: {}",
                       VulkanUtils::VkResultToString(presentResult));
    }
}

void VulkanAPIObj::EndRendering(CommandList cmdList) {
    VkCommandBuffer vkCmdBuffer = cmdList.As<VulkanCommandBufferObj>()->GetVkCommandBuffer();
    vkCmdEndRendering(vkCmdBuffer);
}

void VulkanAPIObj::TransitionImageLayout(CommandList cmdList, Image image,
                                         const ImageLayout newLayout, uint32_t baseArrayLayer) {
    // CZ_BACKEND_LOG(Info, "TransitionImageLayout");

    auto imageObj = image.As<VulkanImageObj>();

    auto vkCmdBuffer = cmdList.As<VulkanCommandBufferObj>()->GetVkCommandBuffer();
    auto vkImage     = imageObj->GetVkImage();
    auto vkOldLayout = imageObj->GetVkImageLayout();
    auto vkNewLayout = VulkanUtils::ToVkImageLayout(newLayout);

    VulkanUtils::TransitionImageLayout(vkCmdBuffer, vkImage, vkOldLayout, vkNewLayout,
                                       baseArrayLayer);
    imageObj->SetVkImageLayout(vkNewLayout);
}

} // namespace CZ
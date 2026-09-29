#include <vector>

#include <cstdint>

#include "VulkanAPIObj.hpp"
#include "VulkanCommandBufferObj.hpp"
#include "VulkanFenceObj.hpp"
#include "VulkanGraphicsContextObj.hpp"
#include "VulkanImageObj.hpp"
#include "VulkanSemaphoreObj.hpp"
#include "VulkanTextureObj.hpp"

namespace CZ {

void VulkanAPIObj::BeginRendering(CommandList cmdList, std::vector<Texture>& targets, bool bClear,
                                  uint32_t faceIndex, Texture depthTarget) {
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

    // Depth attachment: reuse the colour helper for the image view / load-store policy, then
    // override the two fields that differ for a depth image (layout and clear value).
    if (depthTarget) {
        // The depth image was never transitioned out of VK_IMAGE_LAYOUT_UNDEFINED: the colour path
        // only fills the attachment struct and relies on the swapchain side for its barrier, so the
        // depth image needs its own. Transitioning from UNDEFINED each frame is legal and also
        // discards stale depth contents, so no separate clear pass is required.
        VkImageMemoryBarrier depthBarrier{};
        depthBarrier.sType               = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        depthBarrier.srcAccessMask       = 0;
        depthBarrier.dstAccessMask       = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT |
                                           VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT;
        depthBarrier.oldLayout           = VK_IMAGE_LAYOUT_UNDEFINED;
        depthBarrier.newLayout           = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;
        depthBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        depthBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        depthBarrier.image =
            depthTarget.As<VulkanTextureObj>()->GetImage().As<VulkanImageObj>()->GetVkImage();
        depthBarrier.subresourceRange =
            VkImageSubresourceRange{ VK_IMAGE_ASPECT_DEPTH_BIT, 0, 1, 0, 1 };
        vkCmdPipelineBarrier(vkCmdBuffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                             VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT, 0, 0, nullptr, 0, nullptr,
                             1, &depthBarrier);
    }

    VkRenderingAttachmentInfo depthAttachmentInfo{};
    if (depthTarget) {
        depthAttachmentInfo = depthTarget.As<VulkanTextureObj>()->GetColorAttachmentInfo(
            clearValue, bClear, faceIndex);
        depthAttachmentInfo.imageLayout             = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;
        depthAttachmentInfo.clearValue.depthStencil = VkClearDepthStencilValue{ 1.0f, 0 };
        renderingInfo.pDepthAttachment              = &depthAttachmentInfo;
    }

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

    //            "Semaphores were just recreated, skipping one frame to stabilize");

    // 1. CPU wait GPU make resources safety
    bool waitSuccess = fence->WaitAndReset(UINT32_MAX);
    if (!waitSuccess) {
        CZ_BACKEND_LOG(Error, "Failed to wait for fence");
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

    auto imageObj = image.As<VulkanImageObj>();

    auto vkCmdBuffer = cmdList.As<VulkanCommandBufferObj>()->GetVkCommandBuffer();
    auto vkImage     = imageObj->GetVkImage();
    auto vkOldLayout = imageObj->GetVkImageLayout();
    auto vkNewLayout = VulkanUtils::ToVkImageLayout(newLayout);

    VulkanUtils::TransitionImageLayout(vkCmdBuffer, vkImage, vkOldLayout, vkNewLayout,
                                       baseArrayLayer);
    imageObj->SetVkImageLayout(vkNewLayout);
}

VulkanAPIObj::~VulkanAPIObj() {
    if (m_TimestampPool == VK_NULL_HANDLE) {
        return;
    }
    if (auto* vkDev = m_GraphicsContext->GetDevice().As<VulkanDeviceObj>()) {
        vkDestroyQueryPool(vkDev->GetLogicalDevice(), m_TimestampPool, nullptr);
    }
    m_TimestampPool = VK_NULL_HANDLE;
}

void VulkanAPIObj::BeginGPUTiming(CommandList cmdList) {
    if (!m_TimestampChecked) {
        m_TimestampChecked = true;
        auto* vkDev        = m_GraphicsContext->GetDevice().As<VulkanDeviceObj>();
        if (vkDev) {
            VkPhysicalDeviceProperties props{};
            vkGetPhysicalDeviceProperties(vkDev->GetPhysicalDevice(), &props);
            m_TimestampPeriod = props.limits.timestampPeriod;

            uint32_t familyCount = 0;
            vkGetPhysicalDeviceQueueFamilyProperties(vkDev->GetPhysicalDevice(), &familyCount,
                                                     nullptr);
            m_TimestampSupported = props.limits.timestampComputeAndGraphics && familyCount > 0 &&
                                   m_TimestampPeriod > 0.0f;

            VkQueryPoolCreateInfo queryInfo{};
            queryInfo.sType      = VK_STRUCTURE_TYPE_QUERY_POOL_CREATE_INFO;
            queryInfo.queryType  = VK_QUERY_TYPE_TIMESTAMP;
            queryInfo.queryCount = kFramesInFlight * 2;
            if (m_TimestampSupported &&
                vkCreateQueryPool(vkDev->GetLogicalDevice(), &queryInfo, nullptr,
                                  &m_TimestampPool) != VK_SUCCESS) {
                m_TimestampSupported = false;
            }
        }
    }

    if (!m_TimestampSupported) {
        return;
    }

    const uint32_t slot = m_FrameIndex % kFramesInFlight;
    VkCommandBuffer cmd = cmdList.As<VulkanCommandBufferObj>()->GetVkCommandBuffer();

    // The slot was last used kFramesInFlight frames ago, so that submission has completed:
    // resetting it from the command buffer is safe and needs no hostQueryReset feature.
    vkCmdResetQueryPool(cmd, m_TimestampPool, slot * 2, 2);
    vkCmdWriteTimestamp(cmd, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, m_TimestampPool, slot * 2);
}

void VulkanAPIObj::EndGPUTiming(CommandList cmdList) {
    if (!m_TimestampSupported) {
        return;
    }

    const uint32_t slot = m_FrameIndex % kFramesInFlight;
    VkCommandBuffer cmd = cmdList.As<VulkanCommandBufferObj>()->GetVkCommandBuffer();
    vkCmdWriteTimestamp(cmd, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, m_TimestampPool, slot * 2 + 1);

    // Read back the slot written kFramesInFlight frames ago, with availability and never WAIT.
    auto* vkDev = m_GraphicsContext->GetDevice().As<VulkanDeviceObj>();
    if (vkDev && m_FrameIndex >= kFramesInFlight) {
        uint64_t results[4]   = { 0, 0, 0, 0 }; // two queries, each (value, availability)
        const VkResult result = vkGetQueryPoolResults(
            vkDev->GetLogicalDevice(), m_TimestampPool, slot * 2, 2, sizeof(results), results,
            2 * sizeof(uint64_t), VK_QUERY_RESULT_64_BIT | VK_QUERY_RESULT_WITH_AVAILABILITY_BIT);
        if (result == VK_SUCCESS && results[1] != 0 && results[3] != 0 && results[2] > results[0]) {
            m_GpuTiming.FrameSeconds = static_cast<float>(
                static_cast<double>(results[2] - results[0]) * m_TimestampPeriod * 1e-9);
            m_GpuTiming.bValid = true;
        }
    }

    ++m_FrameIndex;
}

} // namespace CZ

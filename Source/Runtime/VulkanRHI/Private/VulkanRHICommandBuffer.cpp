#include "VulkanRHICommandBuffer.h"
#include "VulkanRHICommandPool.h"
#include "VulkanRHIPipeline.h"

DEFINE_LOG_CATEGORY(LogVulkanRHICommandBuffer);

CVulkanRHICommandBuffer::CVulkanRHICommandBuffer(const FRHICommandBufferCreateInfo& info,
                                                 const TRef<CVulkanRHIDevice> device)
    : IRHICommandBuffer(info), m_Device(WeakRef(device)) {
    vk::CommandBufferAllocateInfo allocInfo;
    allocInfo.commandPool = m_Info.CommandPool.As<CVulkanRHICommandPool>()->GetHandle();
    allocInfo.level = vk::CommandBufferLevel::ePrimary;
    allocInfo.commandBufferCount = 1;

    vk::raii::CommandBuffers cmdBuffers(m_Device->GetLogicalDevice(), allocInfo);
    m_Handle = std::move(cmdBuffers.front());
}

CVulkanRHICommandBuffer::~CVulkanRHICommandBuffer() {
    CZ_LOG(LogVulkanRHICommandBuffer, Trace, "VulkanRHICommandBuffer destroying...");
}

void CVulkanRHICommandBuffer::SetViewport(const FRHIViewport& vp) {
    vk::Viewport v(vp.x, vp.y, vp.width, vp.height, vp.minDepth, vp.maxDepth);
    // [Note] 0 is the first viewport index
    m_Handle.setViewport(0, v);
}

void CVulkanRHICommandBuffer::SetScissor(const FRHIScissor& sc) {
    vk::Rect2D s({sc.x, sc.y}, {sc.width, sc.height});
    m_Handle.setScissor(0, s);
}

void CVulkanRHICommandBuffer::Draw(uint32_t vertexCount, uint32_t instanceCount,
                                   uint32_t firstVertex, uint32_t firstInstance) {
    // [Note] Ensure a pipeline is bound before this call to avoid the previous error
    m_Handle.draw(vertexCount, instanceCount, firstVertex, firstInstance);
}

void CVulkanRHICommandBuffer::BindPipeline(TRef<IRHIPipeline> pipeline) {
    auto vlkPipeline = *pipeline.As<CVulkanRHIPipeline>()->GetVKPipeline();

    m_Handle.bindPipeline(vk::PipelineBindPoint::eGraphics, vlkPipeline);
}

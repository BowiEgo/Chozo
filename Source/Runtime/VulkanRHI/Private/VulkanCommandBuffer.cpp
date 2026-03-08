#include "VulkanCommandBuffer.h"

#include "VulkanCommandPool.h"
#include "VulkanPipeline.h"

DEFINE_LOG_CATEGORY(LogVulkanCommandBuffer);

CVulkanCommandBuffer::CVulkanCommandBuffer(const TRef<CVulkanCommandPool>& cmdPool)
    : m_CommandPool(cmdPool) {
    Init();
}

CVulkanCommandBuffer::~CVulkanCommandBuffer() {
    // CZ_LOG(LogVulkanCommandBuffer, Trace, "VulkanCommandBuffer destroying...");
}

void CVulkanCommandBuffer::Init() {
    auto device = m_CommandPool->GetDevice().lock();
    if (!device) {
        CZ_LOG(LogVulkanCommandBuffer, Error,
               "Device is no longer valid during CommandBuffer creation!");
        return;
    }

    const vk::raii::Device& raiiDevice = device->GetRAIILogicalDevice();

    vk::CommandBufferAllocateInfo allocInfo;
    allocInfo.commandPool = m_CommandPool->GetRAIICommandPool();
    allocInfo.level = vk::CommandBufferLevel::ePrimary;
    allocInfo.commandBufferCount = 1;

    vk::raii::CommandBuffers cmdBuffers(raiiDevice, allocInfo);
    m_Handle = std::move(cmdBuffers.front());
}

void CVulkanCommandBuffer::SetViewport(const FRHIViewport& vp) {
    vk::Viewport v(vp.x, vp.y, vp.width, vp.height, vp.minDepth, vp.maxDepth);
    // [Note] 0 is the first viewport index
    m_Handle.setViewport(0, v);
}

void CVulkanCommandBuffer::SetScissor(const FRHIScissor& sc) {
    vk::Rect2D s({ sc.x, sc.y }, { sc.width, sc.height });
    m_Handle.setScissor(0, s);
}

void CVulkanCommandBuffer::Draw(uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex,
                                uint32_t firstInstance) {
    // [Note] Ensure a pipeline is bound before this call to avoid the previous error
    m_Handle.draw(vertexCount, instanceCount, firstVertex, firstInstance);
}

void CVulkanCommandBuffer::BindPipeline(TRef<IRHIPipeline> pipeline) {
    auto& vlkPipeline = pipeline.As<CVulkanPipeline>()->GetRAIIPipeline();

    m_Handle.bindPipeline(vk::PipelineBindPoint::eGraphics, vlkPipeline);
}

#include "VulkanCommandBuffer.h"

#include "VulkanBuffer.h"
#include "VulkanCommandPool.h"
#include "VulkanDescriptorSet.h"
#include "VulkanImage.h"
#include "VulkanSampler.h"
#include "VulkanSetLayout.h"
#include "VulkanUtils.h"

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

    vk::CommandBufferAllocateInfo allocInfo{};
    allocInfo.commandPool        = m_CommandPool->GetRAIICommandPool();
    allocInfo.level              = vk::CommandBufferLevel::ePrimary;
    allocInfo.commandBufferCount = 1;

    vk::raii::CommandBuffers cmdBuffers(raiiDevice, allocInfo);
    m_VKHandle = std::move(cmdBuffers.front());
}

void CVulkanCommandBuffer::SetViewport(const FRHIViewport& vp) {
    vk::Viewport v(vp.x, vp.y, vp.width, vp.height, vp.minDepth, vp.maxDepth);
    // [Note] 0 is the first viewport index
    m_VKHandle.setViewport(0, v);
}

void CVulkanCommandBuffer::SetScissor(const FRHIScissor& sc) {
    vk::Rect2D s({ sc.x, sc.y }, { sc.width, sc.height });
    m_VKHandle.setScissor(0, s);
}

void CVulkanCommandBuffer::SetPolygonMode(EPolygonMode mode) {
    auto device = m_CommandPool->GetDevice().lock();

    vk::CommandBuffer vkBuffer = GetVKCommandBuffer();
    vk::PolygonMode vkMode     = ChozoUtils::Vulkan::GetVulkanPolygonMode(mode);

    device->GetDynamicState3Functions().vkCmdSetPolygonModeEXT(vkBuffer,
                                                               static_cast<VkPolygonMode>(vkMode));
}

void CVulkanCommandBuffer::BindPipeline(TRef<IRHIPipeline> pipeline) {
    m_CurrentPipeline = pipeline.As<CVulkanPipeline>();
    auto& vlkPipeline = m_CurrentPipeline->GetRAIIPipeline();

    SetPolygonMode(pipeline->GetPolygonMode());

    m_VKHandle.bindPipeline(vk::PipelineBindPoint::eGraphics, vlkPipeline);
}

void CVulkanCommandBuffer::PushConstants(const void* data, uint32_t size, uint32_t offset) {
    PushConstants_Internal(vk::ShaderStageFlags(VK_SHADER_STAGE_VERTEX_BIT), data, size, offset);
}

void CVulkanCommandBuffer::PushConstants(const void* data, const FPushConstantRange& range) {
    PushConstants_Internal(ChozoUtils::Vulkan::ToVkStageFlags(range.StageFlags), data, range.Size,
                           range.Offset);
}

void CVulkanCommandBuffer::PushConstants_Internal(vk::ShaderStageFlags stageFlags, const void* data,
                                                  uint32_t size, uint32_t offset) {
    auto vkCommandBuffer              = GetVKCommandBuffer();
    vk::PipelineLayout pipelineLayout = m_CurrentPipeline->GetPipelineLayout();
    if (!pipelineLayout) {
        CZ_LOG(LogVulkan, Error, "Invalid pipeline layout");
        return;
    }

    vkCommandBuffer.pushConstants(pipelineLayout, stageFlags, offset, size, data);
}

void CVulkanCommandBuffer::BindVertexBuffer(TRef<IRHIBuffer> vertexBuffer, int binding) {
    auto vkBuffer = vertexBuffer.As<CVulkanBuffer>();
    if (!vkBuffer) {
        CZ_LOG(LogVulkanCommandBuffer, Error, "Invalid buffer type for Vertex Buffer binding");
        return;
    }

    if (!HasFlag(vkBuffer->GetUsage(), EBufferUsage::VertexBuffer)) {
        CZ_LOG(LogVulkanCommandBuffer, Warning,
               "Binding non-vertex buffer as vertex buffer (flags: 0x%x)",
               static_cast<uint32>(vkBuffer->GetUsage()));
    }

    std::array<vk::Buffer, 1> buffers     = { vkBuffer->GetVKBuffer() };
    std::array<vk::DeviceSize, 1> offsets = { 0 };

    m_VKHandle.bindVertexBuffers(binding, buffers, offsets);

    // CZ_LOG(LogVulkanCommandBuffer, Trace, "Bound vertex buffer: {}",
    //    (void*)vkBuffer->GetVKBuffer());
}

void CVulkanCommandBuffer::BindIndexBuffer(TRef<IRHIBuffer> indexBuffer) {
    auto vkBuffer = indexBuffer.As<CVulkanBuffer>();
    if (!vkBuffer) {
        CZ_LOG(LogVulkanCommandBuffer, Error, "Invalid buffer type for Index Buffer binding");
        return;
    }

    if (!HasFlag(vkBuffer->GetUsage(), EBufferUsage::IndexBuffer)) {
        CZ_LOG(LogVulkanCommandBuffer, Warning,
               "Binding non-index buffer as index buffer (flags: 0x%x)",
               static_cast<uint32>(vkBuffer->GetUsage()));
    }

    m_VKHandle.bindIndexBuffer(vkBuffer->GetVKBuffer(), 0, vk::IndexType::eUint32);

    // CZ_LOG(LogVulkanCommandBuffer, Trace, "Bound index buffer: {}",
    // (void*)vkBuffer->GetVKBuffer());
}

void CVulkanCommandBuffer::DrawIndexed(uint32 indexCount) {
    m_VKHandle.drawIndexed(indexCount, 1, 0, 0, 0);

    // CZ_LOG(LogVulkanCommandBuffer, Trace, "DrawIndexed: {} indices", indexCount);
}

void CVulkanCommandBuffer::DrawIndexed(uint32 indexCount, uint32 instanceCount, uint32 firstIndex,
                                       int32_t vertexOffset, uint32 firstInstance) {
    m_VKHandle.drawIndexed(indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);

    // CZ_LOG(LogVulkanCommandBuffer, Trace, "DrawIndexed: {} indices, {} instances", indexCount,
    //        instanceCount);
}

void CVulkanCommandBuffer::Draw(uint32 vertexCount, uint32 instanceCount, uint32 firstVertex,
                                uint32 firstInstance) {
    // [Note] Ensure a pipeline is bound before this call to avoid the previous error
    m_VKHandle.draw(vertexCount, instanceCount, firstVertex, firstInstance);
}

void CVulkanCommandBuffer::BindDescriptorSets(int set, TRef<IRHIDescriptorSet> descSet) {
    m_VKHandle.bindDescriptorSets(vk::PipelineBindPoint::eGraphics,
                                  m_CurrentPipeline->GetPipelineLayout(), set,
                                  { descSet.As<CVulkanDescriptorSet>()->GetVKHandle() }, {});
}
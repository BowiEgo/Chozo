#include "VulkanCommandBuffer.h"

#include "VulkanBuffer.h"
#include "VulkanCommandPool.h"

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
    m_CurrentPipeline = pipeline.As<CVulkanPipeline>();
    auto& vlkPipeline = m_CurrentPipeline->GetRAIIPipeline();

    m_Handle.bindPipeline(vk::PipelineBindPoint::eGraphics, vlkPipeline);
}

void CVulkanCommandBuffer::BindUniformBuffer(TRef<IRHIBuffer> buffer, int set, int binding) {
    auto device = m_CommandPool->GetDevice().lock();
    auto vkCommandBuffer = GetVKCommandBuffer();
    vk::PipelineLayout currentPipelineLayout = m_CurrentPipeline->GetPipelineLayout();

    auto vkBuffer = buffer.As<CVulkanBuffer>();
    if (!vkBuffer) {
        CZ_LOG(LogVulkan, Error, "Invalid buffer type for Uniform Buffer binding");
        return;
    }

    if (!HasFlag(vkBuffer->GetUsage(), EBufferUsage::UniformBuffer)) {
        CZ_LOG(LogVulkan, Warning, "Binding non-uniform buffer as uniform buffer");
    }

    vk::DescriptorSetLayout layout =
        device->GetDescriptorSetLayout(EDescriptorLayoutType::UniformBuffer);
    vk::DescriptorSet descSet = GetOrCreateDescriptorSet(set, layout);

    vk::DescriptorBufferInfo bufferInfo;
    bufferInfo.setBuffer(vkBuffer->GetVKBuffer()).setOffset(0).setRange(vkBuffer->GetSize());

    vk::WriteDescriptorSet descriptorWrite;
    descriptorWrite.setDstSet(descSet)
        .setDstBinding(binding)
        .setDescriptorCount(1)
        .setDescriptorType(vk::DescriptorType::eUniformBuffer)
        .setPBufferInfo(&bufferInfo);

    device->GetLogicalDevice().updateDescriptorSets({ descriptorWrite }, nullptr);

    vkCommandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, currentPipelineLayout, set,
                                       1, &descSet, 0, nullptr);

    m_BoundDescriptorSets[set] = descSet;
}

vk::DescriptorSet CVulkanCommandBuffer::GetOrCreateDescriptorSet(int set,
                                                                 vk::DescriptorSetLayout layout) {
    auto device = m_CommandPool->GetDevice().lock();

    auto it = m_DescriptorSetCache.find(set);
    if (it != m_DescriptorSetCache.end()) {
        return it->second;
    }

    vk::DescriptorSetAllocateInfo allocInfo;
    allocInfo.setDescriptorPool(device->GetGlobalDescriptorPool())
        .setDescriptorSetCount(1)
        .setPSetLayouts(&layout);

    auto descSet = device->GetLogicalDevice().allocateDescriptorSets(allocInfo)[0];
    m_DescriptorSetCache[set] = descSet;

    return descSet;
}
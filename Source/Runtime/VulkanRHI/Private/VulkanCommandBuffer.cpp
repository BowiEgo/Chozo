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

    vk::CommandBufferAllocateInfo allocInfo;
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

void CVulkanCommandBuffer::BindTexture(IRHITexture* texture, int set, int binding) {
    // auto vkCommandBuffer       = GetVKCommandBuffer();
    // auto currentPipelineLayout = m_CurrentPipeline->GetPipelineLayout();

    // TRef<IRHISetLayout> layout = m_CurrentPipeline->GetSetLayout(set);

    // vk::DescriptorSet descSet = vk::DescriptorSet(
    //     reinterpret_cast<VkDescriptorSet>(texture->GetDescriptorSet(layout, binding)));
    // std::array<vk::DescriptorSet, 1> descSets = { descSet };

    // vkCommandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, currentPipelineLayout,
    // set,
    //                                    descSets, nullptr);

    // m_BoundDescriptorSets[set] = descSet;
}

void CVulkanCommandBuffer::PushConstants(const void* data, uint32_t size, uint32_t offset) {
    PushConstants(VK_SHADER_STAGE_VERTEX_BIT, data, size, offset);
}

void CVulkanCommandBuffer::PushConstants(VkShaderStageFlags stageFlags, const void* data,
                                         uint32_t size, uint32_t offset) {
    auto vkCommandBuffer              = GetVKCommandBuffer();
    vk::PipelineLayout pipelineLayout = m_CurrentPipeline->GetPipelineLayout();
    if (!pipelineLayout) {
        CZ_LOG(LogVulkan, Error, "Invalid pipeline layout");
        return;
    }

    vkCommandBuffer.pushConstants(pipelineLayout, vk::ShaderStageFlags(stageFlags), offset, size,
                                  data);
}

void CVulkanCommandBuffer::BindUniformBuffer(TRef<IRHIBuffer> buffer, int set, int binding) {
    // auto device                              = m_CommandPool->GetDevice().lock();
    // auto vkCommandBuffer                     = GetVKCommandBuffer();
    // vk::PipelineLayout currentPipelineLayout = m_CurrentPipeline->GetPipelineLayout();

    // auto vkBuffer = buffer.As<CVulkanBuffer>();
    // if (!vkBuffer) {
    //     CZ_LOG(LogVulkan, Error, "Invalid buffer type for Uniform Buffer binding");
    //     return;
    // }

    // if (!HasFlag(vkBuffer->GetUsage(), EBufferUsage::UniformBuffer)) {
    //     CZ_LOG(LogVulkan, Warning, "Binding non-uniform buffer as uniform buffer");
    // }

    // vk::DescriptorSetLayout layout =
    //     m_CurrentPipeline->GetSetLayout(set).As<CVulkanSetLayout>()->GetVKHandle();
    // vk::DescriptorSet descSet = GetOrCreateDescriptorSet(set, layout);

    // vk::DescriptorBufferInfo bufferInfo;
    // bufferInfo.setBuffer(vkBuffer->GetVKBuffer()).setOffset(0).setRange(vkBuffer->GetSize());

    // vk::WriteDescriptorSet descriptorWrite;
    // descriptorWrite.setDstSet(descSet)
    //     .setDstBinding(binding)
    //     .setDescriptorCount(1)
    //     .setDescriptorType(vk::DescriptorType::eUniformBuffer)
    //     .setPBufferInfo(&bufferInfo);

    // device->GetLogicalDevice().updateDescriptorSets({ descriptorWrite }, nullptr);

    // vkCommandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, currentPipelineLayout,
    // set,
    //                                    1, &descSet, 0, nullptr);

    // m_BoundDescriptorSets[set] = descSet;
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

// vk::DescriptorSet CVulkanCommandBuffer::GetOrCreateDescriptorSet(int set,
//                                                                  vk::DescriptorSetLayout layout)
//                                                                  {
//     auto device = m_CommandPool->GetDevice().lock();

//     auto it = m_DescriptorSetCache.find(set);
//     if (it != m_DescriptorSetCache.end()) {
//         return it->second;
//     }

//     vk::DescriptorSetAllocateInfo allocInfo;
//     allocInfo.setDescriptorPool(device->GetGlobalDescriptorPool())
//         .setDescriptorSetCount(1)
//         .setPSetLayouts(&layout);

//     auto descSet              = device->GetLogicalDevice().allocateDescriptorSets(allocInfo)[0];
//     m_DescriptorSetCache[set] = descSet;

//     return descSet;
// }

// void CVulkanCommandBuffer::BeginDescriptorSet(int set, TRef<IRHISetLayout> setLayout) {
//     if (m_DescriptorSetCaches.find(set) == m_DescriptorSetCaches.end()) {
//         vk::DescriptorSetAllocateInfo allocInfo;
//         allocInfo.setDescriptorPool(m_Device->GetGlobalDescriptorPool())
//             .setDescriptorSetCount(1)
//             .setPSetLayouts(&setLayout.As<CVulkanSetLayout>()->GetRawHandle());
//         auto descSet = m_Device->GetLogicalDevice().allocateDescriptorSets(allocInfo)[0];
//         m_DescriptorSetCaches[set] = { descSet, setLayout };
//     }
// }

// void CVulkanCommandBuffer::UpdateBuffer(int set, int binding, TRef<IRHIBuffer> buffer) {
//     auto& data = m_DescriptorSetCaches[set];
//     vk::WriteDescriptorSet write;
//     write.setDstSet(data.descSet)
//         .setDstBinding(binding)
//         .setDescriptorCount(1)
//         .setDescriptorType(vk::DescriptorType::eUniformBuffer)
//         .setPBufferInfo(&buffer.As<CVulkanBuffer>()->GetVKBufferInfo());
//     m_Device->GetLogicalDevice().updateDescriptorSets({ write }, nullptr);
// }

// void CVulkanCommandBuffer::UpdateImage(int set, int binding,
//                                        const vk::DescriptorImageInfo& imageInfo) {
//     auto& data = m_DescriptorSetCaches[set];
//     vk::WriteDescriptorSet write;
//     write.setDstSet(data.descSet)
//         .setDstBinding(binding)
//         .setDescriptorCount(1)
//         .setDescriptorType(vk::DescriptorType::eCombinedImageSampler)
//         .setPImageInfo(&imageInfo);
//     m_Device->GetLogicalDevice().updateDescriptorSets({ write }, nullptr);
// }

// void CVulkanCommandBuffer::FlushDescriptorSets() {
//     for (auto& [set, data] : m_DescriptorSetCaches) {
//         m_VKHandle.bindDescriptorSets(vk::PipelineBindPoint::eGraphics,
//                                       m_CurrentPipeline->GetPipelineLayout(), set, { data.descSet
//                                       },
//                                       {});
//     }
// }

void CVulkanCommandBuffer::BindDescriptorSets(int set, TRef<IRHIDescriptorSet> descSet) {
    m_VKHandle.bindDescriptorSets(vk::PipelineBindPoint::eGraphics,
                                  m_CurrentPipeline->GetPipelineLayout(), set,
                                  { descSet.As<CVulkanDescriptorSet>()->GetVKHandle() }, {});
}
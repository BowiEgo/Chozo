#include "VulkanCommandBufferObj.hpp"

#include "VulkanCommandPoolObj.hpp"
#include "VulkanDescriptorSetObj.hpp"
#include "VulkanDeviceObj.hpp"
#include "VulkanGraphicsBufferObj.hpp"
#include "VulkanPipelineObj.hpp"

#include <Core/Log/LogMacros.hpp>
#include <Runtime/RenderCore/Camera/CameraManager.hpp>

namespace CZ {

VulkanCommandBufferObj::~VulkanCommandBufferObj() {
    // if (m_VkCommandBuffer == VK_NULL_HANDLE) return;

    auto deviceObj = m_CmdPoolObj->m_DeviceObj;

    if (!deviceObj) {
        CZ_BACKEND_LOG(Error, "Device is no longer valid during CommandBuffer creation!");
        return;
    }

    VkDevice logicalDevice = deviceObj->GetLogicalDevice();

    vkFreeCommandBuffers(logicalDevice, m_CmdPoolObj->GetVkCommandPool(), 1, &m_VkCommandBuffer);

    m_VkCommandBuffer = VK_NULL_HANDLE;
}

// ---- Public ----

void VulkanCommandBufferObj::Begin() {
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    VkResult result = vkBeginCommandBuffer(m_VkCommandBuffer, &beginInfo);
    if (result != VK_SUCCESS) {
        CZ_BACKEND_LOG(Error, "vkBeginCommandBuffer failed: {}",
                       VulkanUtils::VkResultToString(result));
    }
}

void VulkanCommandBufferObj::SetViewport(const RenderViewport& vp) {
    VkViewport viewport{ vp.x, vp.y, vp.width, vp.height, vp.minDepth, vp.maxDepth };
    vkCmdSetViewport(m_VkCommandBuffer, 0, 1, &viewport);
}

void VulkanCommandBufferObj::SetScissor(const RenderScissor& sc) {
    VkRect2D scissor{ { sc.x, sc.y }, { sc.width, sc.height } };
    vkCmdSetScissor(m_VkCommandBuffer, 0, 1, &scissor);
}

void VulkanCommandBufferObj::SetPolygonMode(PolygonMode mode) {
    auto deviceObj = m_CmdPoolObj->m_DeviceObj;

    if (!deviceObj) {
        CZ_BACKEND_LOG(Error, "Device is no longer valid during CommandBuffer creation!");
        return;
    }

    VkCommandBuffer vkBuffer = m_VkCommandBuffer;
    VkPolygonMode vkMode     = static_cast<VkPolygonMode>(VulkanUtils::GetVulkanPolygonMode(mode));

    deviceObj->GetDynamicState3Functions().vkCmdSetPolygonModeEXT(vkBuffer, vkMode);
}

void VulkanCommandBufferObj::BindPipeline(Pipeline pipeline) {
    m_CurrentPipeline = pipeline;

    auto vkPipeline = m_CurrentPipeline.As<VulkanPipelineObj>()->GetVKPipeline();

    SetPolygonMode(pipeline->GetPolygonMode());

    vkCmdBindPipeline(m_VkCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vkPipeline);
}

// void VulkanCommandBufferObj::BindMaterial(Material material) {
//     auto deviceObj = m_CmdPoolObj->m_DeviceObj;

//     BindPipeline(material->GetPipeline());
//     SetPolygonMode(material->GetPolygonMode());
//     BindDescriptorSets(1, material->GetDescriptorSet());

//     auto setLayout                          = material->GetShader()->GetSetLayout(0);
//     std::vector<DescriptorBinding> bindings = {
//         { 0, UniformType::UniformBuffer, cameraBuffer.get(), nullptr },
//     };
//     auto descSet = deviceObj->GetOrCreateDescriptorSet(setLayout, bindings);

//     BindDescriptorSets(0, descSet);
// }

void VulkanCommandBufferObj::BindDescriptorSets(int set, DescriptorSet descSet) {
    auto vkDescSet = descSet.As<VulkanDescriptorSetObj>()->GetVkDescriptorSet();
    VkPipelineLayout pipelineLayout =
        m_CurrentPipeline.As<VulkanPipelineObj>()->GetVKPipelineLayout();

    vkCmdBindDescriptorSets(m_VkCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, set,
                            1, &vkDescSet, 0, nullptr);
}

void VulkanCommandBufferObj::PushConstants(const void* data, uint32 size, uint32 offset) {
    PushConstants(VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, data, size, offset);
}

void VulkanCommandBufferObj::PushConstants(VkShaderStageFlags stageFlags, const void* data,
                                           uint32_t size, uint32_t offset) {
    VkPipelineLayout pipelineLayout =
        m_CurrentPipeline.As<VulkanPipelineObj>()->GetVKPipelineLayout();

    if (!pipelineLayout) {
        CZ_BACKEND_LOG(Error, "Invalid pipeline layout");
        return;
    }

    vkCmdPushConstants(m_VkCommandBuffer, pipelineLayout, stageFlags, offset, size, data);
}

void VulkanCommandBufferObj::BindVertexBuffer(GraphicsBuffer vertexBuffer, int binding) {
    auto vkBufferObj  = vertexBuffer.As<VulkanGraphicsBufferObj>();
    VkBuffer vkBuffer = vkBufferObj->GetVKBuffer();

    if (!vkBufferObj) {
        CZ_BACKEND_LOG(Error, "Invalid buffer type for Vertex Buffer binding");
        return;
    }

    if (!HasFlag(vkBufferObj->GetUsage(), BufferUsage::VertexBuffer)) {
        CZ_BACKEND_LOG(Warning, "Binding non-vertex buffer as vertex buffer (flags: {})",
                       static_cast<uint32>(vkBufferObj->GetUsage()));
    }

    VkDeviceSize offset = 0;
    vkCmdBindVertexBuffers(m_VkCommandBuffer, binding, 1, &vkBuffer, &offset);
}

void VulkanCommandBufferObj::BindIndexBuffer(GraphicsBuffer indexBuffer) {
    auto vkBufferObj  = indexBuffer.As<VulkanGraphicsBufferObj>();
    VkBuffer vkBuffer = vkBufferObj->GetVKBuffer();

    if (!vkBufferObj) {
        CZ_BACKEND_LOG(Error, "Invalid buffer type for Index Buffer binding");
        return;
    }

    if (!HasFlag(vkBufferObj->GetUsage(), BufferUsage::IndexBuffer)) {
        CZ_BACKEND_LOG(Warning, "Binding non-index buffer as index buffer (flags: {})",
                       static_cast<uint32>(vkBufferObj->GetUsage()));
    }

    vkCmdBindIndexBuffer(m_VkCommandBuffer, vkBuffer, 0, VK_INDEX_TYPE_UINT32);
}

void VulkanCommandBufferObj::DrawIndexed(uint32 indexCount) {
    vkCmdDrawIndexed(m_VkCommandBuffer, indexCount, 1, 0, 0, 0);
}

void VulkanCommandBufferObj::DrawIndexed(uint32 indexCount, uint32 instanceCount, uint32 firstIndex,
                                         int32 vertexOffset, uint32 firstInstance) {
    vkCmdDrawIndexed(m_VkCommandBuffer, indexCount, instanceCount, firstIndex, vertexOffset,
                     firstInstance);
}

void VulkanCommandBufferObj::Draw(uint32 vertexCount, uint32 instanceCount, uint32 firstVertex,
                                  uint32 firstInstance) {
    vkCmdDraw(m_VkCommandBuffer, vertexCount, instanceCount, firstVertex, firstInstance);
}

void VulkanCommandBufferObj::Draw(Mesh mesh) {
    if (!mesh->GetVertexBuffer() || !mesh->GetIndexBuffer()) {
        CZ_LOG(LogProceduralMesh, Error, "Buffers not uploaded");
        return;
    }

    BindVertexBuffer(mesh->GetVertexBuffer(), 0);
    BindIndexBuffer(mesh->GetIndexBuffer());
    DrawIndexed(mesh->GetIndexCount());
}

void VulkanCommandBufferObj::End() {
    VkResult result = vkEndCommandBuffer(m_VkCommandBuffer);
    if (result != VK_SUCCESS) {
        CZ_BACKEND_LOG(Error, "vkEndCommandBuffer failed: {}",
                       VulkanUtils::VkResultToString(result));
    }
}

// ---- Private ----

VkResult VulkanCommandBufferObj::Init() {
    VkResult result;

    auto deviceObj         = m_CmdPoolObj->m_DeviceObj;
    VkDevice logicalDevice = deviceObj->GetLogicalDevice();

    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool        = m_CmdPoolObj->GetVkCommandPool();
    allocInfo.level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = 1;

    VkCommandBuffer cmdBuffer = VK_NULL_HANDLE;
    result                    = vkAllocateCommandBuffers(logicalDevice, &allocInfo, &cmdBuffer);
    if (result != VK_SUCCESS) {
        CZ_BACKEND_LOG(Error, "Failed to allocate command buffer: {}",
                       VulkanUtils::VkResultToString(result));
        return result;
    }
    m_VkCommandBuffer = cmdBuffer;

    return result;
}

} // namespace CZ
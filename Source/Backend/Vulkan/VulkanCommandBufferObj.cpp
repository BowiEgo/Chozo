#include "VulkanCommandBufferObj.hpp"

#include "VulkanCommandPoolObj.hpp"
#include "VulkanDeviceObj.hpp"
#include "VulkanPipelineObj.hpp"

#include <Core/Log/LogMacros.hpp>

namespace CZ {

VulkanCommandBufferObj::VulkanCommandBufferObj(VulkanCommandPoolObj* cmdPoolObj)
    : m_CmdPoolObj(cmdPoolObj) {
    auto deviceObj = cmdPoolObj->m_DeviceObj;

    if (!deviceObj) {
        CZ_CORE_LOG(Error, "Device is no longer valid during CommandBuffer creation!");
        return;
    }

    VkDevice logicalDevice = deviceObj->GetLogicalDevice();

    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool        = cmdPoolObj->GetVkCommandPool();
    allocInfo.level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = 1;

    VkCommandBuffer cmdBuffer = VK_NULL_HANDLE;
    VkResult result           = vkAllocateCommandBuffers(logicalDevice, &allocInfo, &cmdBuffer);
    if (result != VK_SUCCESS) {
        CZ_CORE_LOG(Error, "Failed to allocate command buffer: {}",
                    VulkanUtils::VkResultToString(result));
        return;
    }
    m_VkCommandBuffer = cmdBuffer;
}

VulkanCommandBufferObj::~VulkanCommandBufferObj() {
    // if (m_VkCommandBuffer == VK_NULL_HANDLE) return;

    auto deviceObj = m_CmdPoolObj->m_DeviceObj;

    if (!deviceObj) {
        CZ_CORE_LOG(Error, "Device is no longer valid during CommandBuffer creation!");
        return;
    }

    VkDevice logicalDevice = deviceObj->GetLogicalDevice();

    vkFreeCommandBuffers(logicalDevice, m_CmdPoolObj->GetVkCommandPool(), 1, &m_VkCommandBuffer);

    m_VkCommandBuffer = VK_NULL_HANDLE;
}

void VulkanCommandBufferObj::Begin() {
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    VkResult result = vkBeginCommandBuffer(m_VkCommandBuffer, &beginInfo);
    if (result != VK_SUCCESS) {
        CZ_CORE_LOG(Error, "vkBeginCommandBuffer failed: {}",
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
        CZ_CORE_LOG(Error, "Device is no longer valid during CommandBuffer creation!");
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

// void VulkanCommandBufferObj::BindDescriptorSets(int set, DescriptorSet descSet) {
//     auto vkDescSet = descSet.As<CVulkanDescriptorSet>()->GetVKHandle(); // 假设返回
//     VkDescriptorSet VkPipelineLayout layout = m_CurrentPipeline->GetPipelineLayout();

//     vkCmdBindDescriptorSets(m_VkCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, layout, set, 1,
//     &vkDescSet,
//                             0, nullptr);
// }

// void VulkanCommandBufferObj::PushConstants(const void* data, uint32 size, uint32 offset) {
//     PushConstants(VK_SHADER_STAGE_VERTEX_BIT, data, size, offset);
// }

// void VulkanCommandBufferObj::PushConstants(VkShaderStageFlags stageFlags, const void* data,
//                                            uint32_t size, uint32_t offset) {
// VkPipelineLayout pipelineLayout =
//     m_CurrentPipeline->GetPipelineLayout(); // 假设返回 VkPipelineLayout
// if (!pipelineLayout) {
//     CZ_LOG(LogVulkan, Error, "Invalid pipeline layout");
//     return;
// }

// vkCmdPushConstants(m_VKHandle, pipelineLayout, stageFlags, offset, size, data);
// }

// void VulkanCommandBufferObj::BindVertexBuffer(GraphicsBuffer vertexBuffer, int binding) {
// auto vkBuffer = vertexBuffer.As<CVulkanBuffer>();
// if (!vkBuffer) {
//     CZ_LOG(LogVulkanCommandBuffer, Error, "Invalid buffer type for Vertex Buffer binding");
//     return;
// }

// if (!HasFlag(vkBuffer->GetUsage(), EBufferUsage::VertexBuffer)) {
//     CZ_LOG(LogVulkanCommandBuffer, Warning,
//            "Binding non-vertex buffer as vertex buffer (flags: 0x%x)",
//            static_cast<uint32>(vkBuffer->GetUsage()));
// }

// VkBuffer buffer     = vkBuffer->GetVKBuffer();
// VkDeviceSize offset = 0;
// vkCmdBindVertexBuffers(m_VKHandle, binding, 1, &buffer, &offset);
// }

// void VulkanCommandBufferObj::BindIndexBuffer(GraphicsBuffer indexBuffer) {
//     auto vkBuffer = indexBuffer.As<CVulkanBuffer>();
//     if (!vkBuffer) {
//         CZ_LOG(LogVulkanCommandBuffer, Error, "Invalid buffer type for Index Buffer binding");
//         return;
//     }

//     if (!HasFlag(vkBuffer->GetUsage(), EBufferUsage::IndexBuffer)) {
//         CZ_LOG(LogVulkanCommandBuffer, Warning,
//                "Binding non-index buffer as index buffer (flags: 0x%x)",
//                static_cast<uint32>(vkBuffer->GetUsage()));
//     }

//     vkCmdBindIndexBuffer(m_VKHandle, vkBuffer->GetVKBuffer(), 0, VK_INDEX_TYPE_UINT32);
// }

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

void VulkanCommandBufferObj::End() {
    VkResult result = vkEndCommandBuffer(m_VkCommandBuffer);
    if (result != VK_SUCCESS) {
        CZ_CORE_LOG(Error, "vkEndCommandBuffer failed: {}", VulkanUtils::VkResultToString(result));
    }
}

} // namespace CZ
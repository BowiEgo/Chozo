#include "VulkanCommandBufferObj.h"

#include "VulkanCommandPoolObj.h"
#include "VulkanDeviceObj.h"
#include "VulkanUtils.h"

#include <Core/Log/LogMacros.h>

namespace CZ {

VulkanCommandBufferObj::VulkanCommandBufferObj(VulkanCommandPoolObj* cmdPoolObj) {
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

VulkanCommandBufferObj::~VulkanCommandBufferObj() {}

void VulkanCommandBufferObj::Begin() {}

void VulkanCommandBufferObj::SetViewport(const RenderViewport& viewport) {}

void VulkanCommandBufferObj::SetScissor(const RenderScissor& scissor) {}

void VulkanCommandBufferObj::SetPolygonMode(PolygonMode mode) {}

// void VulkanCommandBufferObj::BindPipeline(Pipeline pipeline) { m_Obj->BindPipeline(pipeline); }

// void VulkanCommandBufferObj::BindDescriptorSets(int set, DescriptorSet descSet) {
// m_Obj->BindDescriptorSets(set, decsSet); }

void VulkanCommandBufferObj::PushConstants(const void* data, uint32 size, uint32 offset) {}

// void VulkanCommandBufferObj::BindVertexBuffer(GraphicsBuffer vertexBuffer, int binding) {
// m_Obj->BindVertexBuffer(vertexBuffer, binding); }

// void VulkanCommandBufferObj::BindIndexBuffer(GraphicsBuffer indexBuffer) {
// m_Obj->BindIndexBuffer(indexBuffer); }

void VulkanCommandBufferObj::DrawIndexed(uint32 indexCount) {}

void VulkanCommandBufferObj::DrawIndexed(uint32 indexCount, uint32 instanceCount, uint32 firstIndex,
                                         int32 vertexOffset, uint32 firstInstance) {}

void VulkanCommandBufferObj::Draw(uint32 vertexCount, uint32 instanceCount, uint32 firstVertex,
                                  uint32 firstInstance) {}

void VulkanCommandBufferObj::End() {}

} // namespace CZ
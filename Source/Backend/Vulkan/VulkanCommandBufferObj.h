#pragma once

#include "VulkanCommandPoolObj.h"

#include "../Source/Runtime/RHI/CommandListObj.h"

#include <Runtime/RHI/CommandList.h>

#include <vulkan/vulkan_core.h>

namespace CZ {

class VulkanCommandPoolObj;

class VulkanCommandBufferObj : public CommandListObj {
public:
    VulkanCommandBufferObj(VulkanCommandPoolObj* cmdPoolObj);
    ~VulkanCommandBufferObj() override;

    void Begin() override;

    void SetViewport(const RenderViewport& viewport) override;

    void SetScissor(const RenderScissor& scissor) override;

    void SetPolygonMode(PolygonMode mode) override;

    //  void BindPipeline(Pipeline pipeline) override;

    //  void BindDescriptorSets(int set, DescriptorSet descSet) override;

    void PushConstants(const void* data, uint32 size, uint32 offset) override;

    //  void BindVertexBuffer(GraphicsBuffer vertexBuffer, int binding) override;

    //  void BindIndexBuffer(GraphicsBuffer indexBuffer) override;

    void DrawIndexed(uint32 indexCount) override;

    void DrawIndexed(uint32 indexCount, uint32 instanceCount, uint32 firstIndex, int32 vertexOffset,
                     uint32 firstInstance) override;

    void Draw(uint32 vertexCount, uint32 instanceCount, uint32 firstVertex,
              uint32 firstInstance) override;

    void End() override;

    VkCommandBuffer GetVkCommandBuffer() const { return m_VkCommandBuffer; }

private:
    VkCommandBuffer m_VkCommandBuffer;
};

} // namespace CZ
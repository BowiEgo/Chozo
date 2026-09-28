#pragma once
#include <cstdint>

#include "VulkanCommandPoolObj.hpp"

#include <Runtime/RHI/CommandList.hpp>

#include <vulkan/vulkan_core.h>

namespace CZ {

class VulkanCommandPoolObj;

class VulkanCommandBufferObj : public CommandListObj {
public:
    VulkanCommandBufferObj(VulkanCommandPoolObj* cmdPoolObj) : m_CmdPoolObj(cmdPoolObj) {};
    ~VulkanCommandBufferObj() override;

    static Scope<VulkanCommandBufferObj> Create(VulkanCommandPoolObj* cmdPoolObj) {
        if (!cmdPoolObj) return nullptr;

        auto obj = CZ_CREATE_SCOPE(MEMORY_USAGE_RENDER, VulkanCommandBufferObj, cmdPoolObj);

        VkResult res = obj->Init();
        if (res != VK_SUCCESS) return nullptr;

        return obj;
    }

    void Begin() override;

    void SetViewport(const RenderViewport& vp) override;

    void SetScissor(const RenderScissor& sc) override;

    void SetPolygonMode(PolygonMode mode) override;

    void BindPipeline(Pipeline pipeline) override;

    void BindDescriptorSets(int set, DescriptorSet descSet) override;

    void PushConstants(const void* data, uint32 size, uint32 offset) override;

    void BindVertexBuffer(GraphicsBuffer vertexBuffer, int binding) override;

    void BindIndexBuffer(GraphicsBuffer indexBuffer) override;

    void DrawIndexed(uint32 indexCount) override;

    void DrawIndexed(uint32 indexCount, uint32 instanceCount, uint32 firstIndex, int32 vertexOffset,
                     uint32 firstInstance) override;

    void Draw(uint32 vertexCount, uint32 instanceCount, uint32 firstVertex,
              uint32 firstInstance) override;

    void Draw(Mesh mesh) override;

    void End() override;

    VkCommandBuffer GetVkCommandBuffer() const { return m_VkCommandBuffer; }

private:
    VkResult Init();
    void PushConstants(VkShaderStageFlags stageFlags, const void* data, uint32_t size,
                       uint32_t offset);

    VkCommandBuffer m_VkCommandBuffer;

    VulkanCommandPoolObj* m_CmdPoolObj;
};

} // namespace CZ
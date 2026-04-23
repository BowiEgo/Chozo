#pragma once

#include "RHICommandList.h"

#include "VulkanCommandPool.h"
#include "VulkanDevice.h"
#include "VulkanPipeline.h"

#include "VulkanRHIExport.h"

DECLARE_LOG_CATEGORY_EXTERN(LogVulkanCommandBuffer, Info);

class VULKAN_RHI_API CVulkanCommandBuffer : public IRHICommandList {
public:
    CVulkanCommandBuffer(const TRef<CVulkanCommandPool>& cmdPool);
    virtual ~CVulkanCommandBuffer();

    virtual void Begin() override { m_VKHandle.begin({}); }
    virtual void SetViewport(const FRHIViewport& viewport) override;
    virtual void SetScissor(const FRHIScissor& scissor) override;
    virtual void SetPolygonMode(EPolygonMode mode) override;
    virtual void BindPipeline(TRef<IRHIPipeline> pipeline) override;
    virtual void BindDescriptorSets(int set, TRef<IRHIDescriptorSet> descSet) override;
    virtual void PushConstants(const void* data, uint32_t size, uint32_t offset) override;
    virtual void BindVertexBuffer(TRef<IRHIBuffer> vertexBuffer, int binding) override;
    virtual void BindIndexBuffer(TRef<IRHIBuffer> indexBuffer) override;
    virtual void DrawIndexed(uint32 indexCount) override;
    virtual void DrawIndexed(uint32 indexCount, uint32 instanceCount, uint32 firstIndex,
                             int32_t vertexOffset, uint32 firstInstance) override;
    virtual void Draw(uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex,
                      uint32_t firstInstance) override;
    virtual void End() override { m_VKHandle.end(); }

    // void BeginDescriptorSet(int set, TRef<IRHISetLayout> setLayout) override;
    // void UpdateBuffer(int set, int binding, const vk::DescriptorBufferInfo& bufferInfo) override;
    // void UpdateImage(int set, int binding, const vk::DescriptorImageInfo& imageInfo) override;
    // void FlushDescriptorSets();

    const vk::CommandBuffer GetVKCommandBuffer() const { return *m_VKHandle; }

private:
    void Init();

    vk::DescriptorSet GetOrCreateDescriptorSet(int set, vk::DescriptorSetLayout layout);

    void PushConstants(VkShaderStageFlags stageFlags, const void* data, uint32_t size,
                       uint32_t offset);

private:
    TRef<CVulkanCommandPool> m_CommandPool;
    TRef<CVulkanPipeline> m_CurrentPipeline;

    vk::raii::CommandBuffer m_VKHandle = nullptr;

    std::unordered_map<int, vk::DescriptorSet> m_DescriptorSetCache;
    std::unordered_map<int, vk::DescriptorSet> m_BoundDescriptorSets;
};

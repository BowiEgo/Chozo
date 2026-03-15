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

    virtual void Begin() override { m_Handle.begin({}); }
    virtual void SetViewport(const FRHIViewport& viewport) override;
    virtual void SetScissor(const FRHIScissor& scissor) override;
    virtual void Draw(uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex,
                      uint32_t firstInstance) override;
    virtual void End() override { m_Handle.end(); }
    virtual void BindPipeline(TRef<IRHIPipeline> pipeline) override;
    virtual void BindUniformBuffer(TRef<IRHIBuffer> buffer, int set, int binding) override;

    const vk::CommandBuffer GetVKCommandBuffer() const { return *m_Handle; }

private:
    void Init();
    vk::DescriptorSet GetOrCreateDescriptorSet(int set, vk::DescriptorSetLayout layout);

private:
    TRef<CVulkanCommandPool> m_CommandPool;
    TRef<CVulkanPipeline> m_CurrentPipeline;

    vk::raii::CommandBuffer m_Handle = nullptr;

    std::unordered_map<int, vk::DescriptorSet> m_DescriptorSetCache;
    std::unordered_map<int, vk::DescriptorSet> m_BoundDescriptorSets;
};

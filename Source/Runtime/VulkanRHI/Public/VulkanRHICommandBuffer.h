#pragma once

#include "RHICommandBuffer.h"
#include "VulkanRHIDevice.h"
#include "VulkanRHIExport.h"

DECLARE_LOG_CATEGORY_EXTERN(LogVulkanRHICommandBuffer, Info);

class VULKAN_RHI_API CVulkanRHICommandBuffer : public IRHICommandBuffer {
public:
    CVulkanRHICommandBuffer(const FRHICommandBufferCreateInfo& info,
                            const TRef<CVulkanRHIDevice>& device);
    virtual ~CVulkanRHICommandBuffer();

    virtual void Begin() { m_Handle.begin({}); }
    virtual void SetViewport(const FRHIViewport& viewport) override;
    virtual void SetScissor(const FRHIScissor& scissor) override;
    virtual void Draw(uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex,
                      uint32_t firstInstance) override;
    virtual void End() { m_Handle.end(); }
    virtual void BindPipeline(TRef<IRHIPipeline> pipeline) override;

    const vk::CommandBuffer GetVKCommandBuffer() const { return *m_Handle; }

private:
    void Init();

private:
    WeakRef<CVulkanRHIDevice> m_Device;

    vk::raii::CommandBuffer m_Handle = nullptr;
};

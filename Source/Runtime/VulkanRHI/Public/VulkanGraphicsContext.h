#pragma once

#include "CoreMinimal.h"
#include "GraphicsContext.h"
#include "VulkanRHIExport.h"
#include "VulkanRHIPipeline.h"

DECLARE_LOG_CATEGORY_EXTERN(LogVulkanGraphicsContext, Info);

class VULKAN_RHI_API CVulkanGraphicsContext : public IGraphicsContext {
public:
    CVulkanGraphicsContext(const FRHIWindowInfo& windowInfo);
    ~CVulkanGraphicsContext();

    virtual void Init() override;
    // virtual void SwapBuffers();

    virtual void SetPipeline(const TRef<IRHIPipeline> pipeline) override;
    virtual void RecordCommandBuffer(const TRef<IRHICommandBuffer> commandBuffer,
                                     const uint32 imageIndex) override;
    virtual void DrawFrame(const TRef<IRHICommandBuffer> commandBuffer,
                           const TRef<IRHISyncObject> syncObject) override;

private:
    void TransitionImageLayout(uint32 imageIndex, vk::ImageLayout old_layout,
                               vk::ImageLayout new_layout, vk::AccessFlags2 src_access_mask,
                               vk::AccessFlags2 dst_access_mask,
                               vk::PipelineStageFlags2 src_stage_mask,
                               vk::PipelineStageFlags2 dst_stage_mask);

private:
    vk::raii::CommandBuffer* m_CurrentVKCmdBuffer = nullptr;
    vk::RenderingInfo m_RenderingInfo;
    uint32 m_ImageIndex;
};

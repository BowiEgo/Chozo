#pragma once

#include "RHICommandList.h"
#include "VulkanRHICommandBuffer.h"
#include "VulkanRHIExport.h"

class VULKAN_RHI_API CVulkanRHICommandList : public IRHICommandList {
public:
    CVulkanRHICommandList(const FRHICommandListCreateInfo& info,
                          const TRef<CVulkanRHICommandBuffer> commandBuffer);
    virtual ~CVulkanRHICommandList() = default;

    virtual void Begin() override;
    virtual void End() override;

private:
    TRef<CVulkanRHICommandBuffer> m_CommandBuffer;
};

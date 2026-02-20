#include "VulkanRHICommandList.h"

CVulkanRHICommandList::CVulkanRHICommandList(const FRHICommandListCreateInfo& info,
                                             const TRef<CVulkanRHICommandBuffer> commandBuffer)
    : IRHICommandList(info), m_CommandBuffer(commandBuffer) {}

void CVulkanRHICommandList::Begin() { m_CommandBuffer->Begin(); }

void CVulkanRHICommandList::End() {}

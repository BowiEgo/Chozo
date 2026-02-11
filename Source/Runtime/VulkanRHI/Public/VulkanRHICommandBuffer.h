#pragma once

#include "RHICommandBuffer.h"
#include "VulkanRHIDevice.h"
#include "VulkanRHIExport.h"

DECLARE_LOG_CATEGORY_EXTERN(LogVulkanRHICommandBuffer, Info);

class VULKAN_RHI_API CVulkanRHICommandBuffer : public IRHICommandBuffer {
public:
    CVulkanRHICommandBuffer(const FRHICommandBufferCreateInfo& info,
                            const TRef<CVulkanRHIDevice> device);
    virtual ~CVulkanRHICommandBuffer();

    vk::raii::CommandBuffer& GetVKCommandBuffer() { return m_ActiveCmdHandle; }

private:
    WeakRef<CVulkanRHIDevice> m_Device;

    vk::raii::CommandBuffer m_ActiveCmdHandle = nullptr;
};

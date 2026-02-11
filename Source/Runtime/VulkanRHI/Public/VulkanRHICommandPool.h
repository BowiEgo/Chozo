#pragma once

#include "RHICommandPool.h"
#include "VulkanRHICommandList.h"
#include "VulkanRHIDevice.h"
#include "VulkanRHIExport.h"

class VULKAN_RHI_API CVulkanRHICommandPool : public IRHICommandPool {
public:
    CVulkanRHICommandPool(const FRHICommandPoolCreateInfo& info,
                          const TRef<CVulkanRHIDevice> device);
    virtual ~CVulkanRHICommandPool() = default;

    virtual TRef<IRHICommandList>
        AllocateCommandList(const FRHICommandListCreateInfo& info,
                            const TRef<IRHICommandBuffer> cmdBuffer) override;

    vk::raii::CommandPool& GetHandle() { return m_Handle; }

private:
    WeakRef<CVulkanRHIDevice> m_Device;
    TRef<CVulkanRHICommandList> m_CommandList;

    vk::raii::CommandPool m_Handle = nullptr;
};

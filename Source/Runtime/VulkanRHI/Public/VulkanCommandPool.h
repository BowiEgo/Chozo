#pragma once

#include "RHICommandPool.h"

#include "VulkanRHIExport.h"

class CVulkanDevice;

class VULKAN_RHI_API CVulkanCommandPool : public IRHICommandPool {
public:
    CVulkanCommandPool(const TRef<CVulkanDevice>& device, const FCommandPoolSpecification& spec);
    virtual ~CVulkanCommandPool() = default;

    virtual TRef<IRHICommandList> AllocateCommandBuffer() override;

    WeakRef<CVulkanDevice> GetDevice() { return m_Device; }
    vk::CommandPool GetVKCommandPool() { return *m_Handle; }
    vk::raii::CommandPool& GetRAIICommandPool() { return m_Handle; }

private:
    void Init();

private:
    WeakRef<CVulkanDevice> m_Device;

    vk::raii::CommandPool m_Handle = nullptr;
};

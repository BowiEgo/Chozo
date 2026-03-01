#pragma once

#include "RHICommandPool.h"

#include "VulkanExport.h"

class CVulkanDevice;

class VULKAN_API CVulkanCommandPool : public IRHICommandPool {
public:
    CVulkanCommandPool(const FCommandPoolSpecification& spec, const TRef<CVulkanDevice>& device);
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

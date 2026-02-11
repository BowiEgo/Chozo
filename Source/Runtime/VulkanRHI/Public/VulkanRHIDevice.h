#pragma once

#include "RHIDevice.h"
#include "RHIPipeline.h"
#include "RHIShader.h"
#include "RHISyncObject.h"
#include "VulkanRHIExport.h"

DECLARE_LOG_CATEGORY_EXTERN(LogVulkanRHIDevice, Info);

class VULKAN_RHI_API CVulkanRHIDevice : public IRHIDevice {
public:
    CVulkanRHIDevice(const FRHIDeviceCreateInfo& info, const vk::raii::Instance& instance,
                     const vk::raii::SurfaceKHR& surface);
    virtual ~CVulkanRHIDevice();

    virtual TRef<IRHIShader> CreateShader(const FRHIShaderCreateInfo& info,
                                          const std::vector<uint32_t>* binary) const override;
    virtual TRef<IRHIPipeline> CreatePipeline(const FRHIPipelineCreateInfo& info) const override;
    virtual TRef<IRHISyncObject> CreateSyncObject() const override;
    virtual void WaitIdle() override;

private:
    void PickPhysicalDevice(const vk::raii::Instance& instance);
    void CreateLogicalDevice(const vk::raii::SurfaceKHR& surface);

public:
    const vk::raii::PhysicalDevice& GetPhysicalDevice() const { return m_PhysicalDevice; }
    const vk::raii::Device& GetLogicalDevice() const { return m_LogicalDevice; }
    const vk::raii::Queue& GetGraphicsQueue() const { return m_GraphicsQueue; }
    const uint32 GetGraphicsQueueIndex() const { return m_GraphicsQueueIndex; }

private:
    std::vector<const char*> m_RequiredDeviceExtension = {vk::KHRSwapchainExtensionName};
    vk::raii::PhysicalDevice m_PhysicalDevice = nullptr;
    vk::raii::Device m_LogicalDevice = nullptr;

    vk::raii::Queue m_GraphicsQueue = nullptr;
    vk::raii::Queue m_PresentQueue = nullptr;
    vk::raii::Queue m_ComputeQueue = nullptr;
    uint32 m_GraphicsQueueIndex;
};

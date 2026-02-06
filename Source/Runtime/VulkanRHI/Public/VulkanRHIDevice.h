#pragma once

#include "RHIDevice.h"
#include "RHIShader.h"
#include "VulkanRHIExport.h"

DECLARE_LOG_CATEGORY_EXTERN(LogVulkanRHIDevice, Info);

class VULKAN_RHI_API CVulkanRHIDevice : public IRHIDevice {
public:
    CVulkanRHIDevice(const vk::raii::Instance& instance,
                     const vk::raii::SurfaceKHR& surface,
                     const FRHIDeviceCreateInfo& info);
    virtual ~CVulkanRHIDevice() = default;

    virtual TRef<IRHIShader>
        CreateShader(const FRHIShaderCreateInfo& info) override;

    virtual void WaitIdle() override {};

private: // TODO: Remove
    void PickPhysicalDevice();

private:
    void Init();

public:
    const vk::raii::Instance& GetVKInstance() const { return m_Instance; }

    const vk::raii::PhysicalDevice& GetVKPhysicalDevice() const {
        return m_PhysicalDevice;
    }
    const vk::raii::Device& GetVKLogicalDevice() const {
        return m_LogicalDevice;
    }

private:
    const vk::raii::Instance& m_Instance;
    const vk::raii::SurfaceKHR& m_Surface;

    vk::raii::PhysicalDevice m_PhysicalDevice = nullptr;
    vk::raii::Device m_LogicalDevice = nullptr;

    vk::raii::Queue m_GraphicsQueue = nullptr;
    vk::raii::Queue m_PresentQueue = nullptr;
    vk::raii::Queue m_ComputeQueue = nullptr;
};
#pragma once

#include "RHIDevice.h"
#include "RHIShader.h"
#include "VulkanRHIExport.h"

DECLARE_LOG_CATEGORY_EXTERN(LogVulkanRHIDevice, Info);

class VULKAN_RHI_API CVulkanRHIDevice : public IRHIDevice {
public:
    CVulkanRHIDevice(const FRHIDeviceCreateInfo& info);
    virtual ~CVulkanRHIDevice() = default;

    virtual TRef<IRHIShader>
        CreateShader(const FRHIShaderCreateInfo& info) override;

    virtual void WaitIdle() override {};

private: // TODO: Remove
    void CreateVKInstance();
    void SetupDebugMessenger();
    void CreateVKSurface();
    void PickPhysicalDevice();

private:
    void Init();

public:
    const vk::raii::Device& GetVKDevice() const { return m_LogicalDevice; }

private: // TODO: Remove
    vk::raii::Context m_Context;
    vk::raii::Instance m_Instance = nullptr;
    vk::raii::DebugUtilsMessengerEXT m_DebugMessenger = nullptr;
    vk::raii::SurfaceKHR m_Surface = nullptr;
    vk::raii::PhysicalDevice m_PhysicalDevice = nullptr;

private:
    vk::raii::Device m_LogicalDevice = nullptr;
    vk::raii::Queue m_GraphicsQueue = nullptr;
    vk::raii::Queue m_PresentQueue = nullptr;
    vk::raii::Queue m_ComputeQueue = nullptr;
};
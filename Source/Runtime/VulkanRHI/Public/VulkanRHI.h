#pragma once

#include "RHI.h"
#include "VulkanRHIDevice.h"
#include "VulkanRHIExport.h"
#include "VulkanRHISwapchain.h"

DECLARE_LOG_CATEGORY_EXTERN(LogVulkanRHI, Info);

class CVulkanRHI : public IRHI {

public:
    CVulkanRHI(const FRHICreateInfo& info);
    virtual ~CVulkanRHI() = default;

    virtual TRef<IRHIDevice>
        CreateDevice(const FRHIDeviceCreateInfo& info) override {
        return TRef<CVulkanRHIDevice>::Create(m_Instance, m_Surface, info);
    }

    // [Note] Surface creation is triggered here but delegated to the platform
    virtual TRef<IRHISwapchain>
        CreateSwapchain(const FRHISwapchainCreateInfo& info) override {
        return TRef<CVulkanRHISwapchain>::Create(m_Instance, m_Surface, info);
    }

    virtual TRef<IRHIDevice> GetDevice() const override { return m_Device; };

private:
    void Init();
    void CreateVKInstance();
    void SetupVKDebugMessenger();
    void CreateVKSurface();

private:
    FRHICreateInfo m_Data;
    // [Note] Vulkan context and instance (Global to the RHI module)
    vk::raii::Context m_Context;
    vk::raii::Instance m_Instance = nullptr;
    vk::raii::DebugUtilsMessengerEXT m_DebugMessenger =
        nullptr; // Only in Debug
    vk::raii::SurfaceKHR m_Surface = nullptr;

    TRef<IRHIDevice> m_Device;
};
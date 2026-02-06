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

    virtual void CreateDevice(const FRHIDeviceCreateInfo& info) override {
        m_Device = TRef<CVulkanRHIDevice>::Create(info, m_Instance, m_Surface);
    }

    // [Note] Surface creation is triggered here but delegated to the platform
    virtual void CreateSwapchain(const FRHISwapchainCreateInfo& info) override {
        m_Swapchain =
            TRef<CVulkanRHISwapchain>::Create(info, m_Surface, m_Device);
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

    TRef<CVulkanRHIDevice> m_Device;
    TRef<CVulkanRHISwapchain> m_Swapchain;
};
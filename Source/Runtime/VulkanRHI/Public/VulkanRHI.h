#pragma once

#include "RHI.h"
#include "VulkanRHICommandList.h"
#include "VulkanRHICommandPool.h"
#include "VulkanRHIDevice.h"
#include "VulkanRHIExport.h"
#include "VulkanRHISwapchain.h"
#include "VulkanRHISyncObject.h"

DECLARE_LOG_CATEGORY_EXTERN(LogVulkanRHI, Info);

class CVulkanRHI : public IRHI {

public:
    CVulkanRHI(const FRHICreateInfo& info);
    virtual ~CVulkanRHI();

private:
    void Init();
    void CreateVKInstance();
    void SetupVKDebugMessenger();
    void CreateVKSurface();
    void CreateCommandPool();

public:
    virtual TRef<IRHIDevice> CreateDevice(const FRHIDeviceCreateInfo& info) override {
        m_Device = TRef<CVulkanRHIDevice>::Create(info, m_Instance, m_Surface);
        CreateCommandPool();
        return m_Device;
    }

    // [Note] Surface creation is triggered here but delegated to the platform
    virtual TRef<IRHISwapchain> CreateSwapchain(const FRHISwapchainCreateInfo& info) override {
        m_Swapchain = CreateRef<CVulkanRHISwapchain>(info, m_Surface, m_Device);
        return m_Swapchain;
    }

    virtual TRef<IRHISyncObject> CreateSyncObject() override {
        m_SyncObject = CreateRef<CVulkanRHISyncObject>(m_Device);
        return m_SyncObject;
    }

    virtual TRef<IRHICommandBuffer> CreateCommandBuffer() override {
        FRHICommandBufferCreateInfo info;
        info.CommandPool = m_MainCommandPool;
        return CreateRef<CVulkanRHICommandBuffer>(info, m_Device);
    }

public:
    virtual TRef<IRHIDevice> GetDevice() const override { return m_Device; };
    virtual TRef<IRHISwapchain> GetSwapchain() const override { return m_Swapchain; };

private:
    FRHICreateInfo m_Info;
    // [Note] Vulkan context and instance (Global to the RHI module)
    vk::raii::Context m_Context;
    vk::raii::Instance m_Instance = nullptr;
    vk::raii::DebugUtilsMessengerEXT m_DebugMessenger = nullptr; // Only in Debug
    vk::raii::SurfaceKHR m_Surface = nullptr;

    TRef<CVulkanRHIDevice> m_Device;
    TRef<CVulkanRHISwapchain> m_Swapchain;
    TRef<CVulkanRHISyncObject> m_SyncObject;
    TRef<CVulkanRHICommandPool> m_MainCommandPool;
    std::vector<TRef<CVulkanRHICommandList>> m_FrameCommandLists;
};

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

private:
    void TransitionImageLayout(const TRef<IRHICommandBuffer> cmd, uint32 imageIndex,
                               vk::ImageLayout old_layout, vk::ImageLayout new_layout,
                               vk::AccessFlags2 src_access_mask, vk::AccessFlags2 dst_access_mask,
                               vk::PipelineStageFlags2 src_stage_mask,
                               vk::PipelineStageFlags2 dst_stage_mask);

public:
    virtual void BeginRenderingToSwapchain(const TRef<IRHICommandBuffer> commandBuffer,
                                           uint32 imageIndex, bool bClear) override;
    virtual void EndRendering(const TRef<IRHICommandBuffer> cmd) override;

    // virtual void RecordCommandBuffer(const TRef<IRHICommandBuffer> commandBuffer,
    //                                  const TRef<IRHIPipeline> pipeline,
    //                                  const uint32 imageIndex) override;

    virtual void DrawFrame(const TRef<IRHICommandBuffer> commandBuffer,
                           const TRef<IRHISyncObject> syncObject,
                           RecordCallback recordCallback) override;

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
        return CreateRef<CVulkanRHISyncObject>(m_Device);
    }

    virtual TRef<IRHICommandBuffer> CreateCommandBuffer() override {
        FRHICommandBufferCreateInfo info;
        info.CommandPool = m_MainCommandPool;
        return CreateRef<CVulkanRHICommandBuffer>(info, m_Device);
    }

public:
    virtual TRef<IRHIDevice> GetDevice() const override { return m_Device; }
    virtual TRef<IRHISwapchain> GetSwapchain() const override { return m_Swapchain; }
    virtual TRef<IRHICommandPool> GetCommandPool() const override { return m_MainCommandPool; }

    const vk::raii::Instance& GetVKInstance() const { return m_Instance; }
    const vk::raii::SurfaceKHR& GetVKSurface() const { return m_Surface; }

private:
    FRHICreateInfo m_Info;
    // [Note] Vulkan context and instance (Global to the RHI module)
    vk::raii::Context m_Context;
    vk::raii::Instance m_Instance = nullptr;
    vk::raii::DebugUtilsMessengerEXT m_DebugMessenger = nullptr; // Only in Debug
    vk::raii::SurfaceKHR m_Surface = nullptr;

    uint32 m_ImageIndex;
    bool m_IsFirstRenderingInFrame = true;

    TRef<CVulkanRHIDevice> m_Device;
    TRef<CVulkanRHISwapchain> m_Swapchain;
    TRef<CVulkanRHICommandPool> m_MainCommandPool;
    std::vector<TRef<CVulkanRHICommandList>> m_FrameCommandLists;
};

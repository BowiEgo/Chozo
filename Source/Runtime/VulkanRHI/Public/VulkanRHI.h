#pragma once

#include "RHI.h"
#include "VulkanRHICommandList.h"
#include "VulkanRHICommandPool.h"
#include "VulkanRHIDevice.h"
#include "VulkanRHIExport.h"
#include "VulkanRHIFrameBuffer.h"
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
    void TransitionTextureLayout(const TRef<CVulkanRHICommandBuffer>& cmd,
                                 TRef<CVulkanRHITexture2D>& texture, vk::ImageLayout oldLayout,
                                 vk::ImageLayout newLayout);
    void SetupBarrierSync(vk::ImageMemoryBarrier2& barrier, vk::ImageLayout oldLayout,
                          vk::ImageLayout newLayout);

public:
    virtual void DrawFrame(const TRef<IRHICommandBuffer>& cmd, TRef<IRHISyncObject>& syncObject,
                           uint32 currentFrame, RecordCallback recordCallback) override;

    virtual void BeginRendering(const TRef<IRHICommandBuffer>& cmd,
                                const TRef<IRHITexture2D>& target, bool bClear) override;

    virtual void EndRendering(const TRef<IRHICommandBuffer>& cmd) override;

    virtual void PrepareTextureForSampling(const TRef<IRHICommandBuffer>& cmd,
                                           const TRef<IRHITexture2D>& texture) override;

    virtual TRef<IRHIDevice> CreateDevice(const FRHIDeviceCreateInfo& info) override {
        m_Device = TRef<CVulkanRHIDevice>::Create(info, m_Instance, m_Surface);
        CreateCommandPool();
        return m_Device;
    }

    // [Note] Surface creation is triggered here but delegated to the platform
    virtual TRef<IRHISwapchain> CreateSwapchain(const FRHISwapchainSpecification& spec) override {
        m_Swapchain = CreateRef<CVulkanRHISwapchain>(spec, m_Device, m_Surface);
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

    virtual TRef<IRHIFrameBuffer>
        CreateFrameBuffer(const FFrameBufferSpecification& spec) override {
        return CreateRef<CVulkanRHIFrameBuffer>(spec, m_Device);
    }

    virtual TRef<IRHITexture2D> CreateTexture2D(const FTextureSpecification& spec) override {
        return CreateRef<CVulkanRHITexture2D>(spec, WeakRef(m_Device));
    }

public:
    virtual TRef<IRHIDevice> GetDevice() const override { return m_Device; }
    virtual TRef<IRHISwapchain> GetSwapchain() const override { return m_Swapchain; }
    virtual TRef<IRHICommandPool> GetCommandPool() const override { return m_MainCommandPool; }

    const vk::Instance GetVKInstance() const { return *m_Instance; }
    const vk::SurfaceKHR GetVKSurface() const { return *m_Surface; }

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
    TRef<CVulkanRHITexture2D> m_Target;
};

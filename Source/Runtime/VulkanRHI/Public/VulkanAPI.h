#pragma once

#include "RHIAPI.h"

#include "VulkanBuffer.h"
#include "VulkanCore.h"
#include "VulkanDevice.h"
#include "VulkanFrameBuffer.h"
#include "VulkanPipeline.h"
#include "VulkanShader.h"
#include "VulkanSwapchain.h"
#include "VulkanSyncObject.h"
#include "VulkanTexture2D.h"
#include "VulkanTextureCubemap.h"

#include "VulkanRHIExport.h"

DECLARE_LOG_CATEGORY_EXTERN(LogVulkanAPI, Info);

class VULKAN_RHI_API CVulkanAPI : public IRHIAPI {
public:
    CVulkanAPI();
    virtual ~CVulkanAPI() override;

    static void Init(const std::vector<const char*>& windowRequiredExtensions,
                     const void* nativeWindowHandle) {
        CVulkanAPI* api = new CVulkanAPI();
        api->m_Vulkan   = new CVulkan(windowRequiredExtensions, nativeWindowHandle);
        if (!IRHIAPI::s_Instance) IRHIAPI::s_Instance = api;
    }

    static vk::Instance GetVKInstance() {
        return static_cast<CVulkanAPI*>(IRHIAPI::s_Instance)->m_Vulkan->GetVKInstance();
    }
    static vk::SurfaceKHR GetVKSurface() {
        return static_cast<CVulkanAPI*>(IRHIAPI::s_Instance)->m_Vulkan->GetVKSurface();
    }

    virtual TRef<IRHIDevice> CreateDevice_Internal(const IRHIContext* ctx,
                                                   const FDeviceSpecification& spec) override {
        const auto& vkInstance = m_Vulkan->GetVKRAIIInstance();
        const auto& vkSurface  = m_Vulkan->GetVKRAIISurface();

        auto device = CreateRef<CVulkanDevice>(ctx, spec, vkInstance, vkSurface);
        device->Init();

        return device;
    }

    virtual TRef<IRHISwapchain>
        CreateSwapchain_Internal(const IRHIContext* ctx,
                                 const FSwapchainSpecification& spec) override {
        auto RHIDevice        = ctx->GetDevice().As<CVulkanDevice>();
        const auto& vkSurface = m_Vulkan->GetVKRAIISurface();
        return CreateRef<CVulkanSwapchain>(spec, RHIDevice, vkSurface);
    }

    virtual TRef<IRHISyncObject> CreateSyncObject_Internal() override {
        auto RHIDevice = m_Context->GetDevice().As<CVulkanDevice>();

        return CreateRef<CVulkanSyncObject>(RHIDevice);
    }

    virtual TRef<IRHIPipeline>
        CreatePipeline_Internal(const FPipelineSpecification& spec) override {
        auto RHIDevice = m_Context->GetDevice().As<CVulkanDevice>();
        return CreateRef<CVulkanPipeline>(spec, RHIDevice);
    }

    virtual TRef<IRHIFrameBuffer>
        CreateFrameBuffer_Internal(const FFrameBufferSpecification& spec) override {
        auto RHIDevice = m_Context->GetDevice().As<CVulkanDevice>();
        return CreateRef<CVulkanFrameBuffer>(spec, RHIDevice);
    }

    virtual TRef<IRHIShader> CreateShader_Internal(const FRHIShaderSpecification& spec,
                                                   const std::vector<uint32_t>* binary,
                                                   const FShaderReflection reflection) override {
        auto RHIDevice = m_Context->GetDevice().As<CVulkanDevice>();
        return CreateRef<CVulkanShader>(spec, RHIDevice, binary, reflection);
    }

    virtual TScope<IRHITexture> CreateTexture_Internal(const FTextureSpecification& spec) override {
        auto RHIDevice = m_Context->GetDevice().As<CVulkanDevice>();
        return RHIDevice->CreateTexture(spec);
    }

    virtual TScope<IRHITexture> CreateTexture_Internal(const FTextureSpecification& spec,
                                                       TScope<IRHIImage> ownedImage) override {
        auto RHIDevice = m_Context->GetDevice().As<CVulkanDevice>();
        return RHIDevice->CreateTexture(spec, std::move(ownedImage));
    }

    virtual TScope<IRHITexture> CreateTexture_Internal(const FTextureSpecification& spec,
                                                       IRHIImage* borrowedImage) override {
        auto RHIDevice = m_Context->GetDevice().As<CVulkanDevice>();
        return RHIDevice->CreateTexture(spec, borrowedImage);
    }

    virtual TScope<IRHITexture> CreateTexture_Internal(const FTextureSpecification& spec,
                                                       FBuffer& data) override {
        auto RHIDevice = m_Context->GetDevice().As<CVulkanDevice>();
        return RHIDevice->CreateTexture(spec, data);
    }

    virtual TRef<IRHIBuffer> CreateBuffer_Internal(const FBufferSpecification& spec) override {
        auto RHIDevice = m_Context->GetDevice();
        return CreateRef<CVulkanBuffer>(WeakRef(RHIDevice), spec);
    }

    virtual TRef<IRHIBuffer> CreateBuffer_Internal(const FBufferSpecification& spec,
                                                   FBuffer& data) override {
        auto RHIDevice = m_Context->GetDevice();
        return CreateRef<CVulkanBuffer>(WeakRef(RHIDevice), spec, data);
    }

    virtual void DrawFrame_Internal(const TRef<IRHICommandList>& cmdBuffer,
                                    TRef<IRHISyncObject>& syncObject,
                                    RecordCallback recordCallback) override;
    virtual void BeginRendering_Internal(const TRef<IRHICommandList>& cmdBuffer, bool bClear,
                                         uint32_t faceIndex = 0, uint32_t mip = 0) override;
    virtual void EndRendering_Internal(const TRef<IRHICommandList>& cmdBuffer) override;
    virtual void TransitionImageLayout_Internal(const TRef<IRHICommandList>& cmdBuffer,
                                                const IRHIImage* image,
                                                const EImageLayout newLayout,
                                                uint32_t baseArrayLayer,
                                                uint32_t layerCount) override;

private:
    CVulkan* m_Vulkan;
};

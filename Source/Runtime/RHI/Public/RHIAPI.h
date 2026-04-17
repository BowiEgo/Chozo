#pragma once

#include "Buffer.h"
#include "RHIBuffer.h"
#include "RHICommandList.h"
#include "RHIContext.h"
#include "RHIDevice.h"
#include "RHIExport.h"
#include "RHIFrameBuffer.h"
#include "RHIPipeline.h"
#include "RHISwapchain.h"
#include "RHISyncObject.h"
#include "RHITexture.h"
#include "Ref.h"

DECLARE_LOG_CATEGORY_EXTERN(LogRHIAPI, Info);

using RecordCallback = std::function<void(uint32)>;

class RHI_API IRHIAPI {
public:
    virtual ~IRHIAPI();

    static void SetContext(IRHIContext* ctx) { s_Instance->m_Context = ctx; }

    static TRef<IRHIDevice> CreateDevice(const IRHIContext* ctx, const FDeviceSpecification& spec) {
        return s_Instance->CreateDevice_Internal(ctx, spec);
    }

    static TRef<IRHISwapchain> CreateSwapchain(const IRHIContext* ctx,
                                               const FSwapchainSpecification& spec) {
        return s_Instance->CreateSwapchain_Internal(ctx, spec);
    }

    static TRef<IRHISyncObject> CreateSyncObject() {
        return s_Instance->CreateSyncObject_Internal();
    }

    static TRef<IRHIFrameBuffer> CreateFrameBuffer(const FFrameBufferSpecification& spec) {
        return s_Instance->CreateFrameBuffer_Internal(spec);
    }

    static TRef<IRHIPipeline> CreatePipeline(const FPipelineSpecification& spec) {
        return s_Instance->CreatePipeline_Internal(spec);
    }

    static TRef<IRHIShader> CreateShader(const FRHIShaderSpecification& spec,
                                         const std::vector<uint32_t>* binary,
                                         const FShaderReflection reflection) {
        return s_Instance->CreateShader_Internal(spec, binary, reflection);
    }

    static TScope<IRHITexture> CreateTexture(const FTextureSpecification& spec) {
        return s_Instance->CreateTexture_Internal(spec);
    }

    static TScope<IRHITexture> CreateTexture(const FTextureSpecification& spec,
                                             TScope<IRHIImage> ownedImage) {
        return s_Instance->CreateTexture_Internal(spec, std::move(ownedImage));
    }

    static TScope<IRHITexture> CreateTexture(const FTextureSpecification& spec,
                                             IRHIImage* borrowedImage) {
        return s_Instance->CreateTexture_Internal(spec, borrowedImage);
    }

    static TScope<IRHITexture> CreateTexture(const FTextureSpecification& spec, FBuffer& data) {
        return s_Instance->CreateTexture_Internal(spec, data);
    }

    static TRef<IRHIBuffer> CreateBuffer(const FBufferSpecification& spec) {
        return s_Instance->CreateBuffer_Internal(spec);
    }

    static TRef<IRHIBuffer> CreateBuffer(const FBufferSpecification& spec, FBuffer& data) {
        return s_Instance->CreateBuffer_Internal(spec, data);
    }

    static void DrawFrame(const TRef<IRHICommandList>& cmdList, TRef<IRHISyncObject>& syncObject,
                          RecordCallback recordCallback) {
        return s_Instance->DrawFrame_Internal(cmdList, syncObject, recordCallback);
    }

    static void BeginRendering(const TRef<IRHICommandList>& cmdList, bool bClear,
                               uint32_t faceIndex = 0) {
        return s_Instance->BeginRendering_Internal(cmdList, bClear, faceIndex);
    }

    static void EndRendering(const TRef<IRHICommandList>& cmdList) {
        return s_Instance->EndRendering_Internal(cmdList);
    }

    static void TransitionImageLayout(const TRef<IRHICommandList>& cmdList, const IRHIImage* image,
                                      const EImageLayout newLayout, uint32_t baseArrayLayer = 0,
                                      uint32_t layerCount = 1) {
        return s_Instance->TransitionImageLayout_Internal(cmdList, image, newLayout, baseArrayLayer,
                                                          layerCount);
    }

protected:
    virtual TRef<IRHIDevice> CreateDevice_Internal(const IRHIContext* ctx,
                                                   const FDeviceSpecification& spec)          = 0;
    virtual TRef<IRHISwapchain> CreateSwapchain_Internal(const IRHIContext* ctx,
                                                         const FSwapchainSpecification& spec) = 0;
    virtual TRef<IRHISyncObject> CreateSyncObject_Internal()                                  = 0;
    virtual TRef<IRHIFrameBuffer>
        CreateFrameBuffer_Internal(const FFrameBufferSpecification& spec)                  = 0;
    virtual TRef<IRHIShader> CreateShader_Internal(const FRHIShaderSpecification& spec,
                                                   const std::vector<uint32_t>* binary,
                                                   const FShaderReflection reflection)     = 0;
    virtual TRef<IRHIPipeline> CreatePipeline_Internal(const FPipelineSpecification& spec) = 0;
    virtual TScope<IRHITexture> CreateTexture_Internal(const FTextureSpecification& spec)  = 0;
    virtual TScope<IRHITexture> CreateTexture_Internal(const FTextureSpecification& spec,
                                                       TScope<IRHIImage> ownedImage)       = 0;
    virtual TScope<IRHITexture> CreateTexture_Internal(const FTextureSpecification& spec,
                                                       IRHIImage* borrowedImage)           = 0;
    virtual TScope<IRHITexture> CreateTexture_Internal(const FTextureSpecification& spec,
                                                       FBuffer& data)                      = 0;

    virtual TRef<IRHIBuffer> CreateBuffer_Internal(const FBufferSpecification& spec) = 0;

    virtual TRef<IRHIBuffer> CreateBuffer_Internal(const FBufferSpecification& spec,
                                                   FBuffer& data) = 0;

    virtual void DrawFrame_Internal(const TRef<IRHICommandList>& cmdList,
                                    TRef<IRHISyncObject>& syncObject,
                                    RecordCallback recordCallback)           = 0;
    virtual void BeginRendering_Internal(const TRef<IRHICommandList>& cmdList, bool bClear,
                                         uint32_t faceIndex)                 = 0;
    virtual void EndRendering_Internal(const TRef<IRHICommandList>& cmdList) = 0;

    virtual void TransitionImageLayout_Internal(const TRef<IRHICommandList>& cmdList,
                                                const IRHIImage* image,
                                                const EImageLayout newLayout,
                                                uint32_t baseArrayLayer, uint32_t layerCount) = 0;

protected:
    static IRHIAPI* s_Instance;
    IRHIContext* m_Context;
};

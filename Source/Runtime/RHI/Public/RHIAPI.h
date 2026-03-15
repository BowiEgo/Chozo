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
#include "RHITexture2D.h"
#include "Ref.h"

DECLARE_LOG_CATEGORY_EXTERN(LogRHIAPI, Info);

using RecordCallback = std::function<void(uint32)>;

class RHI_API IRHIAPI {
public:
    virtual ~IRHIAPI();

    static TRef<IRHIDevice> CreateDevice(const IRHIContext* ctx, const FDeviceSpecification& spec) {
        return s_Instance->CreateDevice_Internal(ctx, spec);
    }

    static TRef<IRHISwapchain> CreateSwapchain(const IRHIContext* ctx,
                                               const FSwapchainSpecification& spec) {
        return s_Instance->CreateSwapchain_Internal(ctx, spec);
    }

    static TRef<IRHISyncObject> CreateSyncObject(const IRHIContext* ctx) {
        return s_Instance->CreateSyncObject_Internal(ctx);
    }

    static TRef<IRHIFrameBuffer> CreateFrameBuffer(const IRHIContext* ctx,
                                                   const FFrameBufferSpecification& spec) {
        return s_Instance->CreateFrameBuffer_Internal(ctx, spec);
    }

    static TRef<IRHIPipeline> CreatePipeline(const IRHIContext* ctx,
                                             const FPipelineSpecification& spec) {
        return s_Instance->CreatePipeline_Internal(ctx, spec);
    }

    static TRef<IRHIShader> CreateShader(const IRHIContext* ctx, const FShaderSpecification& spec,
                                         const std::vector<uint32_t>* binary) {
        return s_Instance->CreateShader_Internal(ctx, spec, binary);
    }

    static TRef<IRHITexture2D> CreateTexture2D(const IRHIContext* ctx,
                                               const FTexture2DSpecification& spec) {
        return s_Instance->CreateTexture2D_Internal(ctx, spec);
    }

    static TRef<IRHITexture2D> CreateTexture2D(const IRHIContext* ctx,
                                               const FTexture2DSpecification& spec, FBuffer& data) {
        return s_Instance->CreateTexture2D_Internal(ctx, spec, data);
    }

    static TRef<IRHIBuffer> CreateBuffer(const IRHIContext* ctx, const FBufferSpecification& spec) {
        return s_Instance->CreateBuffer_Internal(ctx, spec);
    }

    static TRef<IRHIBuffer> CreateBuffer(const IRHIContext* ctx, const FBufferSpecification& spec,
                                         FBuffer& data) {
        return s_Instance->CreateBuffer_Internal(ctx, spec, data);
    }

    static void DrawFrame(IRHIContext* ctx, const TRef<IRHICommandList>& cmdList,
                          TRef<IRHISyncObject>& syncObject, RecordCallback recordCallback) {
        return s_Instance->DrawFrame_Internal(ctx, cmdList, syncObject, recordCallback);
    }

    static void BeginRendering(const IRHIContext* ctx, const TRef<IRHICommandList>& cmdList,
                               bool bClear) {
        return s_Instance->BeginRendering_Internal(ctx, cmdList, bClear);
    }

    static void EndRendering(const IRHIContext* ctx, const TRef<IRHICommandList>& cmdList) {
        return s_Instance->EndRendering_Internal(ctx, cmdList);
    }

    static void PrepareTextureForSampling(const IRHIContext* ctx,
                                          const TRef<IRHICommandList>& cmdList,
                                          const TRef<IRHITexture2D>& texture) {
        return s_Instance->PrepareTextureForSampling_Internal(ctx, cmdList, texture);
    }

protected:
    virtual TRef<IRHIDevice> CreateDevice_Internal(const IRHIContext* ctx,
                                                   const FDeviceSpecification& spec) = 0;
    virtual TRef<IRHISwapchain> CreateSwapchain_Internal(const IRHIContext* ctx,
                                                         const FSwapchainSpecification& spec) = 0;
    virtual TRef<IRHISyncObject> CreateSyncObject_Internal(const IRHIContext* ctx) = 0;
    virtual TRef<IRHIFrameBuffer>
        CreateFrameBuffer_Internal(const IRHIContext* ctx,
                                   const FFrameBufferSpecification& spec) = 0;
    virtual TRef<IRHIShader> CreateShader_Internal(const IRHIContext* ctx,
                                                   const FShaderSpecification& spec,
                                                   const std::vector<uint32_t>* binary) = 0;
    virtual TRef<IRHIPipeline> CreatePipeline_Internal(const IRHIContext* ctx,
                                                       const FPipelineSpecification& spec) = 0;
    virtual TRef<IRHITexture2D> CreateTexture2D_Internal(const IRHIContext* ctx,
                                                         const FTexture2DSpecification& spec) = 0;
    virtual TRef<IRHITexture2D> CreateTexture2D_Internal(const IRHIContext* ctx,
                                                         const FTexture2DSpecification& spec,
                                                         FBuffer& data) = 0;

    virtual TRef<IRHIBuffer> CreateBuffer_Internal(const IRHIContext* ctx,
                                                   const FBufferSpecification& spec) = 0;

    virtual TRef<IRHIBuffer> CreateBuffer_Internal(const IRHIContext* ctx,
                                                   const FBufferSpecification& spec,
                                                   FBuffer& data) = 0;

    virtual void DrawFrame_Internal(IRHIContext* ctx, const TRef<IRHICommandList>& cmdList,
                                    TRef<IRHISyncObject>& syncObject,
                                    RecordCallback recordCallback) = 0;
    virtual void BeginRendering_Internal(const IRHIContext* ctx,
                                         const TRef<IRHICommandList>& cmdList, bool bClear) = 0;
    virtual void EndRendering_Internal(const IRHIContext* ctx,
                                       const TRef<IRHICommandList>& cmdList) = 0;
    virtual void PrepareTextureForSampling_Internal(const IRHIContext* ctx,
                                                    const TRef<IRHICommandList>& cmdList,
                                                    const TRef<IRHITexture2D>& texture) = 0;

protected:
    static IRHIAPI* s_Instance;
};

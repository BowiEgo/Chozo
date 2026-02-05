#pragma once

#include "Ref.h"

class RHI_API IRHIDevice : public FRefCounted {
public:
    virtual ~IRHIDevice() = default;

    // virtual TRef<IRHIBuffer> CreateBuffer(const FRHIBufferDesc &desc) = 0;
    // virtual TRef<IRHITexture> CreateTexture(const FRHITextureDesc &desc) = 0;
    // virtual TRef<IRHIPipelineState>
    //     CreatePipelineState(const FRHIPipelineStateDesc &desc) = 0;

    // virtual TRef<IRHICommandContext> GetDefaultContext() = 0;

    // virtual ERendererAPI GetAPIType() const = 0;
};
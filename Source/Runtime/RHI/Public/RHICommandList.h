#pragma once

#include "RHICommandPool.h"
#include "RHIExport.h"
#include "RHIPipeline.h"
#include "Ref.h"

DECLARE_LOG_CATEGORY_EXTERN(LogRHICommandBuffer, Info);

struct FRHIViewport {
    float x, y, width, height, minDepth, maxDepth;
};

struct FRHIScissor {
    int32_t x, y;
    uint32_t width, height;
};

class RHI_API IRHICommandList : public FRefCounted {
public:
    IRHICommandList();
    virtual ~IRHICommandList();

    virtual void Begin() = 0;
    virtual void SetViewport(const FRHIViewport& viewport) = 0;
    virtual void SetScissor(const FRHIScissor& scissor) = 0;
    virtual void Draw(uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex,
                      uint32_t firstInstance) = 0;
    virtual void End() = 0;
    virtual void BindPipeline(TRef<IRHIPipeline> pipeline) = 0;
};

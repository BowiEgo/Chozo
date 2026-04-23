#pragma once

#include "RHIBuffer.h"
#include "RHICommandPool.h"
#include "RHIDescriptorSet.h"
#include "RHIPipeline.h"
#include "RHITexture.h"
#include "Ref.h"

#include "RHIExport.h"

DECLARE_LOG_CATEGORY_EXTERN(LogRHICommandBuffer, Info);

struct FRHIViewport {
    float x, y, width, height, minDepth, maxDepth;
};

struct FRHIScissor {
    int32 x, y;
    uint32 width, height;
};

class RHI_API IRHICommandList : public FRefCounted {
public:
    IRHICommandList();
    virtual ~IRHICommandList();

    virtual void Begin()                                                         = 0;
    virtual void SetViewport(const FRHIViewport& viewport)                       = 0;
    virtual void SetScissor(const FRHIScissor& scissor)                          = 0;
    virtual void SetPolygonMode(EPolygonMode mode)                               = 0;
    virtual void BindPipeline(TRef<IRHIPipeline> pipeline)                       = 0;
    virtual void BindDescriptorSets(int set, TRef<IRHIDescriptorSet> descSet)    = 0;
    virtual void PushConstants(const void* data, uint32_t size, uint32_t offset) = 0;
    virtual void BindVertexBuffer(TRef<IRHIBuffer> vertexBuffer, int binding)    = 0;
    virtual void BindIndexBuffer(TRef<IRHIBuffer> indexBuffer)                   = 0;
    virtual void DrawIndexed(uint32 indexCount)                                  = 0;
    virtual void DrawIndexed(uint32 indexCount, uint32 instanceCount, uint32 firstIndex,
                             int32_t vertexOffset, uint32 firstInstance)         = 0;
    virtual void Draw(uint32 vertexCount, uint32 instanceCount, uint32 firstVertex,
                      uint32 firstInstance)                                      = 0;
    virtual void End()                                                           = 0;

protected:
    struct FPerSetData {
        void* descSet;
        TRef<IRHISetLayout> layout;
    };

    std::unordered_map<int, FPerSetData> m_DescriptorSetCaches;
};

#pragma once

#include "CoreMinimal.h"
#include "RHI.h"
#include "RHICommandBuffer.h"
#include "RHIExport.h"
#include "RHIPipeline.h"
#include "Scope.h"

DECLARE_LOG_CATEGORY_EXTERN(LogGraphicsContext, Info);

struct FRHIWindowInfo {
    FExtent2D FrameBufferSize;
    void* NativeWindow;
    std::vector<const char*> RequiredExtensions;
};

class RHI_API IGraphicsContext {
public:
    IGraphicsContext(const FRHIWindowInfo& windowInfo);
    ~IGraphicsContext();

    IRHI* GetRHI() const { return m_RHI.get(); }

    virtual void Init() = 0;
    // virtual void SwapBuffers();

    virtual void SetPipeline(const TRef<IRHIPipeline> pipeline) = 0;
    virtual void RecordCommandBuffer(const TRef<IRHICommandBuffer> commandBuffer,
                                     const uint32 imageIndex) = 0;
    virtual void DrawFrame(const TRef<IRHICommandBuffer> commandBuffer,
                           const TRef<IRHISyncObject> syncObject) = 0;

protected:
    FRHIWindowInfo m_WindowInfo;
    TScope<IRHI> m_RHI;
    TRef<IRHIPipeline> m_Pipeline;
};

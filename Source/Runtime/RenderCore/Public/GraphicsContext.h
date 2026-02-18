#pragma once

#include "CoreMinimal.h"
#include "Module.h"
#include "RHI.h"
#include "RHICommandBuffer.h"
#include "RHIPipeline.h"
#include "RHISyncObject.h"
#include "RenderCoreExport.h"
#include "Scope.h"

DECLARE_LOG_CATEGORY_EXTERN(LogGraphicsContext, Info);

struct FRHIWindowInfo {
    FExtent2D FrameBufferSize;
    void* NativeWindow;
    std::vector<const char*> RequiredExtensions;
};

class RENDER_CORE_API CGraphicsContext {
public:
    CGraphicsContext(const FRHIWindowInfo& windowInfo);
    ~CGraphicsContext();

    IRHI* GetRHI() const { return m_RHI.get(); }

    void Init();
    //  void SwapBuffers();

protected:
    CModule m_RHIModule;

    FRHIWindowInfo m_WindowInfo;
    TScope<IRHI> m_RHI;
};

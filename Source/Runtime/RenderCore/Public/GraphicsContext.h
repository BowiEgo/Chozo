#pragma once

#include "CoreMinimal.h"
#include "RHI.h"
#include "RenderCoreExport.h"
#include "RendererWindow.h"
#include "Scope.h"

DECLARE_LOG_CATEGORY_EXTERN(LogGraphicsContext, Info);

class RENDER_CORE_API CGraphicsContext {
public:
    CGraphicsContext(IRendererWindow* windowHandle);
    ~CGraphicsContext();

    IRHI* GetRHI() const { return m_RHI.get(); }

    void Init();
    // void SwapBuffers();

    static CGraphicsContext& Get();

protected:
    static CGraphicsContext* s_Instance;

    IRendererWindow* m_Window;
    TScope<IRHI> m_RHI;
};

#pragma once

#include "CoreMinimal.h"
#include "RendererWindow.h"

using FWindowHandle = void *;

class RENDERCORE_API CGraphicsContext {
public:
    CGraphicsContext(IRendererWindow *windowHandle) : m_Window(windowHandle) {};
    virtual ~CGraphicsContext() = default;

    virtual void Init() = 0;
    virtual void SwapBuffers() = 0;

    virtual void CreateRenderer() = 0; // TODO: Remove

    // English comment: Factory method to create the appropriate context
    static TScope<CGraphicsContext> Create(IRendererWindow *windowHandle);

protected:
    IRendererWindow *m_Window;
};

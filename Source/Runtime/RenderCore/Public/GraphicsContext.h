#pragma once

#include "CoreMinimal.h"
#include "RendererWindow.h"

namespace Chozo {

using WindowHandle = void *;

class RENDERCORE_API GraphicsContext {
public:
    GraphicsContext(IRendererWindow *windowHandle) : m_Window(windowHandle) {};
    virtual ~GraphicsContext() = default;

    virtual void Init() = 0;
    virtual void SwapBuffers() = 0;

    virtual void CreateRenderer() = 0; // TODO: Remove

    // English comment: Factory method to create the appropriate context
    static Scope<GraphicsContext> Create(IRendererWindow *windowHandle);

protected:
    IRendererWindow *m_Window;
};

} // namespace Chozo
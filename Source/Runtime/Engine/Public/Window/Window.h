#pragma once

#include "Core.h"
#include "Platform.h"
#include "RendererWindow.h"
#include "WindowDefinition.h"

namespace Chozo {

using WindowHandle = void *;
using WindowSurface = void *;

class ENGINE_API Window : public IRendererWindow {
public:
    Window(const FWindowDefinition &def) : m_Definition(def) {};
    virtual ~Window() {};

    virtual void Init() = 0;
    virtual void Shutdown() = 0;
    virtual void OnUpdate() = 0;
    virtual void SetVSync(bool enabled) = 0;
    virtual bool ShouldClose() const = 0;

    // from IRendererWindow
    virtual void GetFramebufferSize(int *width, int *height) const override = 0;
    virtual std::vector<const char *>
        GetRequiredExtensions() const override = 0;
    virtual WindowHandle GetWindowWrapper() const override { return m_Window; }
    virtual WindowHandle GetNativeHandle() const = 0;

    unsigned int GetWidth() const { return m_Definition.Width; }
    unsigned int GetHeight() const { return m_Definition.Height; }
    float GetPixelRatio() const { return m_Definition.PixelRatio; }
    bool IsVSync() const { return m_Definition.VSync; }
    float GetDPI() const { return m_Definition.XScale; }

    static Scope<Window> Create(const FWindowDefinition &windowDef);

protected:
    FWindowDefinition m_Definition;
    WindowHandle m_Window{nullptr};
};
} // namespace Chozo
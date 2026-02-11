#pragma once

#include "Core.h"
#include "EngineExport.h"
#include "Platform.h"
#include "RendererWindow.h"
#include "WindowDefinition.h"

class ENGINE_API CWindow : public IRendererWindow {
public:
    CWindow(const FWindowDefinition& def) : m_Definition(def) {};
    virtual ~CWindow() {};

    virtual void Init() = 0;
    virtual void Shutdown() = 0;
    virtual void OnUpdate() = 0;
    virtual void SetVSync(bool enabled) = 0;
    virtual bool ShouldClose() const = 0;

    // from IRendererWindow
    virtual FExtent2D GetFramebufferSize() const override = 0;
    virtual std::vector<const char*> GetRequiredExtensions() const override = 0;
    virtual FWindowHandle GetWindowWrapper() const override { return m_Window; }
    virtual FWindowHandle GetNativeHandle() const = 0;

    unsigned int GetWidth() const { return m_Definition.Width; }
    unsigned int GetHeight() const { return m_Definition.Height; }
    float GetPixelRatio() const { return m_Definition.PixelRatio; }
    bool IsVSync() const { return m_Definition.VSync; }
    float GetDPI() const { return m_Definition.XScale; }

    static TScope<CWindow> Create(const FWindowDefinition& windowDef);

protected:
    FWindowDefinition m_Definition;
    FWindowHandle m_Window{nullptr};
};

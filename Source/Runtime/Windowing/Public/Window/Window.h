#pragma once

#include "Core.h"
#include "Event.h"
#include "Platform.h"
#include "RendererWindow.h"
#include "WindowDefinition.h"
#include "WindowingExport.h"

class WINDOWING_API CWindow : public IRendererWindow {
public:
    CWindow(const FWindowDefinition& def) : m_Definition(def) {};
    virtual ~CWindow() {};

    virtual void Init() = 0;
    virtual void Shutdown() = 0;
    virtual void OnUpdate() = 0;
    virtual void SetVSync(bool enabled) = 0;
    virtual bool ShouldClose() const = 0;
    virtual void SetEventCallback(const FEventCallback& callback) = 0;

    // from IRendererWindow
    virtual FExtent2D GetLogicalSize() const override = 0;
    virtual FExtent2D GetPhysicalSize() const override = 0;
    virtual std::vector<const char*> GetRequiredExtensions() const override = 0;
    virtual FWindowHandle GetWindowWrapper() const override { return m_Window; }
    virtual FWindowHandle GetNativeHandle() const override = 0;

    FExtent2D GetSize() const { return m_Definition.Size; }
    float GetPixelRatio() const { return m_Definition.PixelRatio; }
    bool IsVSync() const { return m_Definition.VSync; }

    static TScope<CWindow> Create(const FWindowDefinition& windowDef);

protected:
    FWindowDefinition m_Definition;
    FWindowHandle m_Window{ nullptr };
};

#pragma once

#include "Core.h"
#include "Event.h"
#include "Platform.h"
#include "RendererWindow.h"
#include "WindowingExport.h"

#include <atomic>

struct WINDOWING_API FWindowDefinition {
    std::string Title;
    FExtent2D Size;
    FExtent2D FrameBufferScale;
    float PixelRatio;
    bool VSync;
    FEventCallback EventCallback;

    FWindowDefinition() : Title("Chozo Engine"), Size({ 1280, 720 }) {}
};

class WINDOWING_API CWindow : public IRendererWindow {
public:
    CWindow(const FWindowDefinition& def) : m_Definition(def) {};
    virtual ~CWindow() {};

    virtual void Init() = 0;
    virtual void Shutdown() = 0;
    virtual void OnUpdate() = 0;
    virtual bool ShouldClose() const = 0;
    virtual void SetEventCallback(const FEventCallback& callback) = 0;

    // from IRendererWindow
    virtual void SetVSync(bool enabled) override {
        if (m_Definition.VSync != enabled) {
            m_Definition.VSync = enabled;
            m_VSyncDirty.store(true);
        }
    }
    virtual bool IsVSyncEnabled() const override { return m_Definition.VSync; }
    virtual bool CheckAndResetVSyncDirty() override { return m_VSyncDirty.exchange(false); }

    virtual FExtent2D GetSize() const override = 0;
    virtual FExtent2D GetFrameBufferSize() const override = 0;
    virtual FExtent2D GetFrameBufferScale() const override = 0;
    virtual std::vector<const char*> GetRequiredExtensions() const override = 0;
    virtual FWindowHandle GetWindowWrapper() const override { return m_Window; }
    virtual FWindowHandle GetNativeHandle() const override = 0;

    float GetPixelRatio() const { return m_Definition.PixelRatio; }
    bool IsVSync() const { return m_Definition.VSync; }

    static TScope<CWindow> Create(const FWindowDefinition& windowDef);

protected:
    FWindowDefinition m_Definition;
    FWindowHandle m_Window{ nullptr };
    std::atomic_bool m_VSyncDirty{ false };
};

#pragma once

#include "Core.h"
#include "Window.h"
#include "WindowingExport.h"

#include <SDL3/SDL.h>

DECLARE_LOG_CATEGORY_EXTERN(LogSDLWindow, Info);

class WINDOWING_API CSDLWindow : public CWindow {
public:
    CSDLWindow(const FWindowDefinition& def) : CWindow(def) {};
    ~CSDLWindow();

    // CWindow interface
    virtual void Init() override;
    virtual void Shutdown() override;
    virtual void OnUpdate() override;
    virtual bool ShouldClose() const override;
    virtual void SetEventCallback(const FEventCallback& callback) override {
        m_Definition.EventCallback = callback;
    }

    // IRendererWindow interface
    virtual FExtent2D GetSize() const override;
    virtual FExtent2D GetFrameBufferSize() const override;
    virtual FExtent2D GetFrameBufferScale() const override { return m_Definition.FrameBufferScale; }
    virtual std::vector<const char*> GetRequiredExtensions() const override;
    virtual FWindowHandle GetNativeHandle() const override;

    SDL_Window* GetSDLWindow() const { return static_cast<SDL_Window*>(m_Window); }

private:
    void CreateSDLWindow();
    void ProcessEvent(const SDL_Event& event);

    bool m_bShouldClose = false;
    static bool s_SDLInitialized;
};

#pragma once

#include "Core.h"
#include "Window.h"
#include "WindowingExport.h"

#include <GLFW/glfw3.h>

DECLARE_LOG_CATEGORY_EXTERN(LogCGLFWWindow, Info);

static bool s_GLFWInitialized = false;

class WINDOWING_API CGLFWWindow : public CWindow {
public:
    CGLFWWindow(const FWindowDefinition& def) : CWindow(def) {};
    ~CGLFWWindow();

    // from Window
    virtual void Init() override;
    virtual void Shutdown() override;
    virtual void OnUpdate() override;
    virtual bool ShouldClose() const override;
    virtual void SetEventCallback(const FEventCallback& callback) override {
        m_Definition.EventCallback = callback;
    }

    // from IRendererWindow
    virtual FExtent2D GetSize() const override;
    virtual FExtent2D GetFrameBufferSize() const override;
    virtual std::vector<const char*> GetRequiredExtensions() const override;
    virtual FWindowHandle GetNativeHandle() const override;

    GLFWwindow* GetGLFWWindow() const { return static_cast<GLFWwindow*>(m_Window); }

private:
    static void OnGLFWError(int error, const char* description);

private:
    void CreateGLFWWindow();
    void SetGLFWCallbacks();
};

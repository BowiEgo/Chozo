#pragma once

#include "Core.h"
#include "Window.h"

#include <GLFW/glfw3.h>

namespace Chozo {

DECLARE_LOG_CATEGORY_EXTERN(LogGenericGLFWWindow, Info);

static bool s_GLFWInitialized = false;

static void GLFWErrorCallback(int error, const char *description) {
    CZ_LOG(LogGenericGLFWWindow, Error, "GLFW Error ({0}):", error,
           description);
}

class ENGINE_API GenericGLFWWindow : public Window {
public:
    GenericGLFWWindow(const FWindowDefinition &def) : Window(def) {};
    ~GenericGLFWWindow();

    // from Window
    virtual void Init() override;
    virtual void Shutdown() override;
    virtual void OnUpdate() override;
    virtual void SetVSync(bool enabled) override;
    virtual bool ShouldClose() const override;

    // from IRendererWindow
    virtual void GetFramebufferSize(int *width, int *height) const override;
    virtual std::vector<const char *> GetRequiredExtensions() const override;
    virtual WindowHandle GetNativeHandle() const override;

    GLFWwindow *GetGLFWWindow() const {
        return static_cast<GLFWwindow *>(m_Window);
    }

private:
    void CreateGLFWWindow();
};
} // namespace Chozo

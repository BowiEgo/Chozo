#include "GenericGLFWWindow.h"

#ifdef CHOZO_PLATFORM_WINDOWS
    #ifndef NOMINMAX
        #define NOMINMAX // 必须在 windows.h 之前，防止 std::min/max 冲突
    #endif
    #define GLFW_EXPOSE_NATIVE_WIN32
    #include <windows.h>
// 注意：SetProcessDpiAwarenessContext 需要包含 ShellScalingApi.h 或特定的 SDK
// 版本
#elif defined(CHOZO_PLATFORM_LINUX)
    #define GLFW_EXPOSE_NATIVE_X11 // 或 WAYLAND
#elif defined(CHOZO_PLATFORM_MACOS)
    #define GLFW_EXPOSE_NATIVE_COCOA
#endif

#include <GLFW/glfw3native.h>

namespace Chozo {

DEFINE_LOG_CATEGORY(LogGenericGLFWWindow);

GenericGLFWWindow::~GenericGLFWWindow() {}

void GenericGLFWWindow::Init() { CreateGLFWWindow(); }

void GenericGLFWWindow::Shutdown() {
    CZ_LOG(GenericGLFWWindow, Trace, "Destroying window {0}",
           m_Definition.Title);

    if (m_Window) {
        glfwDestroyWindow(GetGLFWWindow());
    }

    glfwTerminate();
}

void GenericGLFWWindow::OnUpdate() {
    /* Poll for and process events */
    glfwPollEvents();
}

void GenericGLFWWindow::SetVSync(bool enabled) { m_Definition.VSync = enabled; }

bool GenericGLFWWindow::ShouldClose() const {
    return glfwWindowShouldClose(GetGLFWWindow());
}

void GenericGLFWWindow::GetFramebufferSize(int *width, int *height) const {
    glfwGetFramebufferSize(GetGLFWWindow(), width, height);
}

std::vector<const char *> GenericGLFWWindow::GetRequiredExtensions() const {
    uint32_t glfwExtensionCount = 0;
    const char **glfwExtensions =
        glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    return std::vector<const char *>(glfwExtensions,
                                     glfwExtensions + glfwExtensionCount);
}

WindowHandle GenericGLFWWindow::GetNativeHandle() const {
#ifdef CHOZO_PLATFORM_WINDOWS
    return (void *)glfwGetWin32Window((GLFWwindow *)m_Window);
#elif defined(CHOZO_PLATFORM_LINUX)

    return (void *)glfwGetX11Window(GetGLFWWindow());
#elif defined(CHOZO_PLATFORM_MACOS)

    return (void *)glfwGetCocoaWindow(GetGLFWWindow());
#endif
    return nullptr;
}

void GenericGLFWWindow::CreateGLFWWindow() {
    CZ_LOG(LogGenericGLFWWindow, Trace, "Creating window({1}, {2}) for {0}",
           m_Definition.Title, m_Definition.Width, m_Definition.Height);

    const bool dimensionsInValid =
        m_Definition.Width <= 0 || m_Definition.Height <= 0;
    CZ_ASSERT(!dimensionsInValid, "Invalid window dimensions!");

#ifdef CZ_PLATFORM_WIN
    SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
#endif

    // Initialize GLFW window
    if (!s_GLFWInitialized) {
        const int success = glfwInit();
        CZ_ASSERT(success, "Could not initialize GLFW!");
        glfwSetErrorCallback(GLFWErrorCallback);
        s_GLFWInitialized = true;
    }

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    m_Window = glfwCreateWindow(m_Definition.Width, m_Definition.Height,
                                m_Definition.Title.c_str(), nullptr, nullptr);

    // Set user pointer to access WindowData in callbacks
    glfwSetWindowUserPointer(GetGLFWWindow(), &m_Definition);

    // Pixel ratio
    int windowWidth, windowHeight;
    int fbWidth, fbHeight;
    glfwGetWindowSize(GetGLFWWindow(), &windowWidth, &windowHeight);
    glfwGetFramebufferSize(GetGLFWWindow(), &fbWidth, &fbHeight);
    m_Definition.PixelRatio = (float)fbWidth / (float)windowWidth;

    // DPI Scaling
    float xscale, yscale, factor = 1.0f;
#ifdef CZ_PLATFORM_MACOS
    factor = 0.5f;
#endif
    glfwGetWindowContentScale(GetGLFWWindow(), &xscale, &yscale);
    m_Definition.XScale = xscale * factor;
    m_Definition.YScale = yscale * factor;

    CZ_LOG(LogGenericGLFWWindow, Info,
           "GLFW Window created for Vulkan successfully.");

    SetVSync(false);
}

} // namespace Chozo

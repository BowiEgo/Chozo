#include "GLFWWindow.h"
#include "ApplicationEvent.h"
#include "KeyEvent.h"
#include "MouseEvent.h"

#ifdef CZ_PLATFORM_WINDOWS
    #ifndef NOMINMAX
        #define NOMINMAX // 必须在 windows.h 之前，防止 std::min/max 冲突
    #endif
    #define GLFW_EXPOSE_NATIVE_WIN32
    #include <windows.h>
// 注意：SetProcessDpiAwarenessContext 需要包含 ShellScalingApi.h 或特定的 SDK
// 版本
#elif defined(CZ_PLATFORM_LINUX)
    #define GLFW_EXPOSE_NATIVE_X11 // 或 WAYLAND
#elif defined(CZ_PLATFORM_MACOS)
    #define GLFW_EXPOSE_NATIVE_COCOA
#endif

#include <GLFW/glfw3native.h>

DEFINE_LOG_CATEGORY(LogCGLFWWindow);

CGLFWWindow::~CGLFWWindow() {}

void CGLFWWindow::Init() {
    CreateGLFWWindow();
    SetGLFWCallbacks();
}

void CGLFWWindow::Shutdown() {
    CZ_LOG(CGLFWWindow, Trace, "Destroying window {0}", m_Definition.Title);

    if (m_Window) {
        glfwDestroyWindow(GetGLFWWindow());
    }

    glfwTerminate();
}

void CGLFWWindow::OnUpdate() {
    /* Poll for and process events */
    glfwPollEvents();
}

void CGLFWWindow::SetVSync(bool enabled) { m_Definition.VSync = enabled; }

bool CGLFWWindow::ShouldClose() const { return glfwWindowShouldClose(GetGLFWWindow()); }

FExtent2D CGLFWWindow::GetFramebufferSize() const {
    FExtent2D result;

    int w, h;
    glfwGetFramebufferSize(GetGLFWWindow(), &w, &h);

    result.Width = w;
    result.Height = h;

    return result;
}

std::vector<const char*> CGLFWWindow::GetRequiredExtensions() const {
    uint32_t glfwExtensionCount = 0;
    const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    return std::vector<const char*>(glfwExtensions, glfwExtensions + glfwExtensionCount);
}

FWindowHandle CGLFWWindow::GetNativeHandle() const {
#ifdef CZ_PLATFORM_WINDOWS
    return (void*)glfwGetWin32Window(GetGLFWWindow());
#elif defined(CZ_PLATFORM_LINUX)

    return (void*)glfwGetX11Window(GetGLFWWindow());
#elif defined(CZ_PLATFORM_MACOS)

    return (void*)glfwGetCocoaWindow(GetGLFWWindow());
#endif
    return nullptr;
}

void CGLFWWindow::CreateGLFWWindow() {
    CZ_LOG(LogCGLFWWindow, Trace, "Creating window({1}, {2}) for {0}", m_Definition.Title,
           m_Definition.Width, m_Definition.Height);

    const bool dimensionsInValid = m_Definition.Width <= 0 || m_Definition.Height <= 0;
    CZ_CORE_ASSERT(!dimensionsInValid, "CGLFWWindow: Invalid window dimensions!");

#ifdef CZ_PLATFORM_WIN
    SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
#endif

    // Initialize GLFW window
    if (!s_GLFWInitialized) {
        const int success = glfwInit();
        CZ_CORE_ASSERT(success, "CGLFWWindow: Could not initialize GLFW!");
        glfwSetErrorCallback(CGLFWWindow::OnGLFWError);
        s_GLFWInitialized = true;
    }

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    // glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    m_Window = glfwCreateWindow(m_Definition.Width, m_Definition.Height, m_Definition.Title.c_str(),
                                nullptr, nullptr);

    // Set user pointer to access WindowData in callbacks
    auto glfwWindow = GetGLFWWindow();
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

    CZ_LOG(LogCGLFWWindow, Info, "GLFW Window for Vulkan created.");

    SetVSync(false);
}

void CGLFWWindow::SetGLFWCallbacks() {
    glfwSetWindowContentScaleCallback(
        GetGLFWWindow(), [](GLFWwindow* window, float xscale, float yscale) {
            float factor = 1.0f;
#ifdef CZ_PLATFORM_MACOS
            factor = 0.75f;
#endif

            FWindowDefinition& def = *(FWindowDefinition*)glfwGetWindowUserPointer(window);
            def.XScale = xscale * factor;
            def.YScale = yscale * factor;

            FWindowContentScaledEvent event(def.XScale, def.YScale);
            def.EventCallback(event);
        });

    glfwSetWindowSizeCallback(GetGLFWWindow(), [](GLFWwindow* window, int width, int height) {
        FWindowDefinition& def = *(FWindowDefinition*)glfwGetWindowUserPointer(window);
        def.Width = width;
        def.Height = height;

        FWindowResizedEvent event(width, height);
        def.EventCallback(event);
    });

    glfwSetWindowCloseCallback(GetGLFWWindow(), [](GLFWwindow* window) {
        FWindowDefinition& def = *(FWindowDefinition*)glfwGetWindowUserPointer(window);

        FWindowCloseEvent event;
        def.EventCallback(event);
    });

    glfwSetKeyCallback(
        GetGLFWWindow(), [](GLFWwindow* window, int key, int scancode, int action, int mods) {
            FWindowDefinition& def = *(FWindowDefinition*)glfwGetWindowUserPointer(window);

            // ImGui_ImplGlfw_KeyCallback(window, key, scancode, action, mods);

            switch (action) {
            case GLFW_PRESS: {
                FKeyPressedEvent event((EKeyCode)key, 0);
                def.EventCallback(event);
                break;
            }
            case GLFW_RELEASE: {
                FKeyReleasedEvent event((EKeyCode)key);
                def.EventCallback(event);
                break;
            }
            case GLFW_REPEAT: {
                FKeyPressedEvent event((EKeyCode)key, 1);
                def.EventCallback(event);
                break;
            }
            default:
                break;
                ;
            }
        });

    glfwSetCharCallback(GetGLFWWindow(), [](GLFWwindow* window, uint32_t character) {
        FWindowDefinition& def = *(FWindowDefinition*)glfwGetWindowUserPointer(window);
        FKeyTypedEvent event((EKeyCode)character);
        def.EventCallback(event);
    });

    glfwSetMouseButtonCallback(
        GetGLFWWindow(), [](GLFWwindow* window, int button, int action, int mods) {
            FWindowDefinition& def = *(FWindowDefinition*)glfwGetWindowUserPointer(window);

            switch (action) {
            case GLFW_PRESS: {
                FMouseButtonPressedEvent event((EMouseButton)button);
                def.EventCallback(event);
                break;
            }
            case GLFW_RELEASE: {
                FMouseButtonReleasedEvent event((EMouseButton)button);
                def.EventCallback(event);
                break;
            }
            default:
                break;
                ;
            }
        });

    glfwSetScrollCallback(GetGLFWWindow(), [](GLFWwindow* window, double xOffset, double yOffset) {
        FWindowDefinition& def = *(FWindowDefinition*)glfwGetWindowUserPointer(window);

        FMouseScrolledEvent event((float)xOffset, (float)yOffset);
        def.EventCallback(event);
    });

    glfwSetCursorPosCallback(GetGLFWWindow(), [](GLFWwindow* window, double xPos, double yPos) {
        FWindowDefinition& def = *(FWindowDefinition*)glfwGetWindowUserPointer(window);

        FMouseMovedEvent event((float)xPos, (float)yPos);
        def.EventCallback(event);
    });
}

void CGLFWWindow::OnGLFWError(int error, const char* description) {
    CZ_LOG(LogCGLFWWindow, Error, "GLFW Error ({0}):", error, description);
}

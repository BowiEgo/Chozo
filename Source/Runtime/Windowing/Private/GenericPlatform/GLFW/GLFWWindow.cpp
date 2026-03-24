#include "GLFWWindow.h"
#include "ApplicationEvent.h"
#include "KeyEvent.h"
#include "MouseEvent.h"

#include <GLFW/glfw3native.h>

DEFINE_LOG_CATEGORY(LogGLFWWindow);

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

bool CGLFWWindow::ShouldClose() const { return glfwWindowShouldClose(GetGLFWWindow()); }

FExtent2D CGLFWWindow::GetSize() const {
    FExtent2D result;

    int w, h;
    glfwGetFramebufferSize(GetGLFWWindow(), &w, &h);

    result.Width = w / m_Definition.FrameBufferScale.Width;
    result.Height = h / m_Definition.FrameBufferScale.Height;

    return result;
}

FExtent2D CGLFWWindow::GetFrameBufferSize() const {
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
    const bool dimensionsInValid = m_Definition.Size.Width <= 0 || m_Definition.Size.Height <= 0;
    CZ_CORE_ASSERT(!dimensionsInValid, "CGLFWWindow: Invalid window dimensions!");

#ifdef CZ_PLATFORM_WINDOWS
    SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
#endif

    // Initialize GLFW window
    {
        int major, minor, rev;
        glfwGetVersion(&major, &minor, &rev);
        CZ_LOG(LogGLFWWindow, Trace, "GLFW Version: {}.{}.{} (Need >= 3.3)\n", major, minor, rev);
    }

    if (!s_GLFWInitialized) {
        const int success = glfwInit();
        CZ_CORE_ASSERT(success, "CGLFWWindow: Could not initialize GLFW!");
        glfwSetErrorCallback(CGLFWWindow::OnGLFWError);
        s_GLFWInitialized = true;
    }

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_MAXIMIZED, GLFW_TRUE);
    // glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    // glfwWindowHint(GLFW_SCALE_TO_MONITOR, GLFW_TRUE);
#ifdef __APPLE__
    glfwWindowHint(GLFW_SCALE_FRAMEBUFFER, GLFW_TRUE);
#endif

    m_Window = glfwCreateWindow(m_Definition.Size.Width, m_Definition.Size.Height,
                                m_Definition.Title.c_str(), nullptr, nullptr);

    // Set user pointer to access WindowData in callbacks
    auto glfwWindow = GetGLFWWindow();
    glfwSetWindowUserPointer(GetGLFWWindow(), &m_Definition);

    // Render Scaling
    float xscale, yscale = 1.0f;
    glfwGetWindowContentScale(GetGLFWWindow(), &xscale, &yscale);
    m_Definition.FrameBufferScale.Width = xscale;
    m_Definition.FrameBufferScale.Height = yscale;

    // Pixel ratio
    int w, h;
    auto fbSize = GetFrameBufferSize();
    glfwGetWindowSize(GetGLFWWindow(), &w, &h);
    m_Definition.PixelRatio = (float)fbSize.Width / (float)w;
    m_Definition.Size = GetSize();

    CZ_LOG(LogGLFWWindow, Info, "GLFW Window({1}, {2}) for {0} for Vulkan created.",
           m_Definition.Title, m_Definition.Size.Width, m_Definition.Size.Height);
}

void CGLFWWindow::SetGLFWCallbacks() {
    glfwSetWindowContentScaleCallback(GetGLFWWindow(), [](GLFWwindow* window, float xscale,
                                                          float yscale) {
        FWindowDefinition& def = *(FWindowDefinition*)glfwGetWindowUserPointer(window);
        def.FrameBufferScale.Width = xscale;
        def.FrameBufferScale.Height = yscale;

        FWindowContentScaledEvent event(def.FrameBufferScale.Width, def.FrameBufferScale.Height);
        def.EventCallback(event);
    });

    glfwSetWindowSizeCallback(GetGLFWWindow(), [](GLFWwindow* window, int width, int height) {
        FWindowDefinition& def = *(FWindowDefinition*)glfwGetWindowUserPointer(window);
        def.Size = { (unsigned int)width, (unsigned int)height };

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
                default: break; ;
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
                default: break; ;
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
    CZ_LOG(LogGLFWWindow, Error, "GLFW Error ({0}):", error, description);
}

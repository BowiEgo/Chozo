#include "prxpch.h"
#include "OpenGLContext.h"

#include <Glad/glad.h>
#include <GLFW/glfw3.h>

namespace Chozo {
    OpenGLContext::OpenGLContext(GLFWwindow *windowHandle)
        : m_WindowHandle(windowHandle)
    {
        CZ_CORE_ASSERT(windowHandle, "Window handle is null!");
    }

    void OpenGLContext::Init()
    {
        glfwMakeContextCurrent(m_WindowHandle);
        // Initialize GLAD
        int status = gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
        CZ_CORE_ASSERT(status, "Failed to initialize Glad!");

        CZ_CORE_INFO("OpenGL Info:");
        CZ_CORE_INFO("  Vendor: {0}", (const char*)(glGetString(GL_VENDOR)));
        CZ_CORE_INFO("  Renderer: {0}", (const char*)(glGetString(GL_RENDERER)));
        CZ_CORE_INFO("  Version: {0}", (const char*)(glGetString(GL_VERSION)));
    }

    void OpenGLContext::SwapBuffers()
    {
        glfwSwapBuffers(m_WindowHandle);
    }
}
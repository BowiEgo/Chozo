#include "prxpch.h"
#include "OpenGLContext.h"

#include <Glad/glad.h>
#include <GLFW/glfw3.h>

namespace Parallax {
    OpenGLContext::OpenGLContext(GLFWwindow *windowHandle)
        : m_WindowHandle(windowHandle)
    {
        PRX_CORE_ASSERT(windowHandle, "Window handle is null!");
    }

    void OpenGLContext::Init()
    {
        glfwMakeContextCurrent(m_WindowHandle);
        // Initialize GLAD
        int status = gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
        PRX_CORE_ASSERT(status, "Failed to initialize Glad!");

        PRX_CORE_INFO("OpenGL Info:");
        PRX_CORE_INFO("  Vendor: {0}", (const char*)(glGetString(GL_VENDOR)));
        PRX_CORE_INFO("  Renderer: {0}", (const char*)(glGetString(GL_RENDERER)));
        PRX_CORE_INFO("  Version: {0}", (const char*)(glGetString(GL_VERSION)));
    }

    void OpenGLContext::SwapBuffers()
    {
        glfwSwapBuffers(m_WindowHandle);
    }
}
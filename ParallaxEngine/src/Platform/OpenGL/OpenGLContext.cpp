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
    }

    void OpenGLContext::SwapBuffers()
    {
        glBegin(GL_TRIANGLES);

        glEnd();
        
        glfwSwapBuffers(m_WindowHandle);
    }
}
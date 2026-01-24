#include "GLFWWindow.h"

namespace Chozo
{
    DEFINE_LOG_CATEGORY(GLFWWindow);

    GLFWWindow::GLFWWindow(const FWindowDefinition& windowDef)
        : FWindow(windowDef)
    {
        Init(windowDef);
    }

    GLFWWindow::~GLFWWindow()
    {
        Shutdown();
    }

    void GLFWWindow::Init(const FWindowDefinition& windowDef)
    {
        m_Data.Title = windowDef.Title;
        m_Data.Width = windowDef.Width;
        m_Data.Height = windowDef.Height;

        CZ_LOG(GLFWWindow, Trace, "Creating window({1}, {2}) for {0}", m_Data.Title, m_Data.Width, m_Data.Height);

        // Initialize GLFW window here
    }

    void GLFWWindow::Shutdown()
    {
        // Destroy GLFW window here
    }

    void GLFWWindow::OnUpdate()
    {
        // Update window events here
    }
}
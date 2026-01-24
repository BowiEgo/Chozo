#include "Window.h"

namespace Chozo
{
    FWindow::FWindow(const FWindowDefinition& windowDef)
    {
        Init(windowDef);
    }

    FWindow::~FWindow()
    {
        Shutdown();
    }

    void FWindow::Init(const FWindowDefinition& windowDef)
    {
        m_Data.Title = windowDef.Title;
        m_Data.Width = windowDef.Width;
        m_Data.Height = windowDef.Height;

        // Initialize GLFW window here
    }

    void FWindow::Shutdown()
    {
        // Destroy GLFW window here
    }

    void FWindow::OnUpdate()
    {
        // Update window events here
    }
}
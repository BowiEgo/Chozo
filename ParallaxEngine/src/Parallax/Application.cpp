#include "Application.h"

#include "Parallax/Events/ApplicationEvent.h"
#include "Parallax/Events/KeyEvent.h"
#include "Parallax/Events/MouseEvent.h"

#include "FmtFormatter.h"

#include <GLFW/glfw3.h>

namespace Parallax {
    Application::Application()
    {
        m_Window = std::unique_ptr<Window>(Window::Create());
    }
    
    Application::~Application()
    {
    }

    void Application::Run()
    {
        WindowResizeEvent e(1280, 7200);
        
        PRX_TRACE(e);
        while (m_Running)
        {
            glClearColor(0.2, 0.3, 0.8, 1);
            glClear(GL_COLOR_BUFFER_BIT);
            m_Window->OnUpdate();
        }
    }   
}

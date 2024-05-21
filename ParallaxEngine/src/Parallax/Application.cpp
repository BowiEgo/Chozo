#include "Application.h"

#include "Parallax/Events/ApplicationEvent.h"
#include "Parallax/Events/KeyEvent.h"
#include "Parallax/Events/MouseEvent.h"
#include "Parallax/Log.h"

#include "FmtFormatter.h"

namespace Parallax {
    Application::Application()
    {

    }
    
    Application::~Application()
    {

    }

    void Application::Run()
    {
        WindowResizeEvent e(1280, 7200);
        
        PRX_TRACE(e);
        while (true);
    }   
}

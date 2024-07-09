#include "Window.h"

#ifdef ENGINE_PLATFORM_MACOS
    #include "Platform/Mac/MacWindow.h"
#endif

namespace Chozo {

    Scope<Window> Chozo::Window::Create(const WindowProps &props)
    {
        return std::make_unique<MacWindow>(props);
    }
}
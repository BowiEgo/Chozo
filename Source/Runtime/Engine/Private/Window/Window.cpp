#include "Window.h"
#include "GenericGLFWWindow.h"

namespace Chozo {

Scope<Window> Window::Create(const FWindowDefinition &windowDef) {
    return CreateScope<GenericGLFWWindow>(windowDef);
}
} // namespace Chozo
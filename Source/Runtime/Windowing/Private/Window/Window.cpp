#include "Window.h"
#include "GLFWWindow.h"

TScope<CWindow> CWindow::Create(const FWindowDefinition &windowDef) {
    return CreateScope<CGLFWWindow>(windowDef);
}
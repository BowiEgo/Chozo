#include "Window.h"
#include "SDLWindow.h"

TScope<CWindow> CWindow::Create(const FWindowDefinition& windowDef) {
    return CreateScope<CSDLWindow>(windowDef);
}
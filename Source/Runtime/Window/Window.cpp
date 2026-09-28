#include <Runtime/Window/Window.hpp>

#include <Core/Memory/Memory.hpp>

#include "SDLWindow/SDLWindowObj.hpp"

namespace CZ {

Scope<WindowObj> Window::Create(const WindowSpecifaciton& spec) {
    return CZ_CREATE_SCOPE(MEMORY_USAGE_RUNTIME, SDLWindowObj, spec);
}

} // namespace CZ
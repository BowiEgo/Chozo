#include <Runtime/Window/Window.hpp>

#include <Core/Memory/Memory.hpp>

#include "SDLWindow/SDLWindowObj.hpp"

namespace CZ {

Window Window::Create(const WindowSpecifaciton& spec) {
    return Window(CZ_NEW(MEMORY_USAGE_RUNTIME, SDLWindowObj, spec));
}

DEFINE_HANDLE_DESTROY(WindowObj)

} // namespace CZ
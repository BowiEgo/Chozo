#include "SDLWindowObj.hpp"

#include <Core/Event/AppEvent.hpp>
#include <Core/Event/Input.hpp>
#include <Core/Event/KeyEvent.hpp>
#include <Core/Event/MouseEvent.hpp>
#include <Core/Log/LogMacros.hpp>
#include <Core/Platform/Platform.h>

#include "Core/Memory/Memory.hpp"
#include "SDLInputImpl.hpp"
#include "SDLKeyMap.hpp"
#include <Core/Header/Extent.hpp>

#include <SDL3/SDL_vulkan.h>

namespace CZ {

DEFINE_LOG_CATEGORY_STATIC(LogSDLWindow, Info);

static void ProcessEvent(const SDL_Event& event, bool& shouldClose, SDL_Window* sdlWindow,
                         EventCallback cb) {

    switch (event.type) {
        case SDL_EVENT_QUIT: shouldClose = true; break;
        case SDL_EVENT_WINDOW_CLOSE_REQUESTED: shouldClose = true; break;
        case SDL_EVENT_WINDOW_RESIZED:
        case SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED: {
            int w, h;
            SDL_GetWindowSize(static_cast<SDL_Window*>(sdlWindow), &w, &h);
            WindowResizedEvent e(w, h);
            cb(e);
            break;
        }
        case SDL_EVENT_WINDOW_DISPLAY_SCALE_CHANGED: {
            float scale = SDL_GetWindowDisplayScale(static_cast<SDL_Window*>(sdlWindow));
            WindowContentScaledEvent e(scale, scale);
            EventDispatcher dispatcher(e);
            cb(e);
            break;
        }
        case SDL_EVENT_KEY_DOWN:
        case SDL_EVENT_KEY_UP: {
            KeyCode key = SDLScancodeToEngineKey(event.key.scancode);
            int repeat  = event.key.repeat;
            if (event.type == SDL_EVENT_KEY_DOWN) {
                KeyPressedEvent e(key, repeat);
                cb(e);
                break;

            } else {
                KeyReleasedEvent e(key);
                cb(e);
                break;
            }
            break;
        }
        case SDL_EVENT_TEXT_INPUT: {
            const char* text = event.text.text;
            if (text && text[0] != 0) {
                KeyTypedEvent e((KeyCode)text[0]);
                cb(e);
                break;
            }
            break;
        }
        case SDL_EVENT_MOUSE_BUTTON_DOWN:
        case SDL_EVENT_MOUSE_BUTTON_UP: {
            MouseButton btn = static_cast<MouseButton>(event.button.button);
            if (event.type == SDL_EVENT_MOUSE_BUTTON_DOWN) {
                MouseButtonPressedEvent e(btn);
                cb(e);
                break;
            } else {
                MouseButtonReleasedEvent e(btn);
                cb(e);
                break;
            }
            break;
        }
        case SDL_EVENT_MOUSE_MOTION: {
            float x = (float)event.motion.x;
            float y = (float)event.motion.y;
            MouseMovedEvent e(x, y);
            cb(e);
            break;
        }
        case SDL_EVENT_MOUSE_WHEEL: {
            float xOff = (float)event.wheel.x;
            float yOff = (float)event.wheel.y;
            MouseScrolledEvent e(xOff, yOff);
            cb(e);
            break;
        }
        default: break;
    }
}

SDLWindowObj::SDLWindowObj(const WindowSpecifaciton& spec) : WindowObj(spec) {}

SDLWindowObj::~SDLWindowObj() { Shutdown(); }

void SDLWindowObj::Shutdown() {
    if (m_Window) {
        SDL_DestroyWindow(static_cast<SDL_Window*>(m_Window));
        m_Window = nullptr;
    }
    if (m_BackendInitialized) {
        SDL_Quit();
        m_BackendInitialized = false;
    }
}

bool SDLWindowObj::Init(std::string& err) {
    if (!m_BackendInitialized) {
        if (!SDL_Init(SDL_INIT_VIDEO)) {
            err = SDL_GetError();
            return false;
        }
        m_BackendInitialized = true;
    }

    uint32_t flags = SDL_WINDOW_RESIZABLE;
    if (!m_Spec.IsDecorated) flags |= SDL_WINDOW_BORDERLESS;
    if (m_Spec.IsFullscreen) flags |= SDL_WINDOW_FULLSCREEN;
    if (m_Spec.IsFocused) flags |= SDL_WINDOW_INPUT_FOCUS;
    flags |= SDL_WINDOW_VULKAN;
    flags |= SDL_WINDOW_HIGH_PIXEL_DENSITY;

    m_Window = SDL_CreateWindow(m_Spec.Title.c_str(), m_Spec.Size.Width, m_Spec.Size.Height, flags);

    if (!m_Window) {
        err = SDL_GetError();
        SDL_Quit();
        return false;
    }

    m_InputImpl = CZ_CREATE_SCOPE(MEMORY_USAGE_RUNTIME, SDLInputImpl);
    Input::Init(m_InputImpl.get());

    return true;
}

void SDLWindowObj::OnUpdate() {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        ProcessEventWithPreprocessor(event);
        ProcessEvent(event, m_ShouldClose, static_cast<SDL_Window*>(m_Window),
                     m_Spec.EventCallback);
    }
}

bool SDLWindowObj::ShouldClose() const { return m_ShouldClose; }

Extent2D SDLWindowObj::GetSize() const {
    int w, h;
    SDL_GetWindowSize(static_cast<SDL_Window*>(m_Window), &w, &h);
    return { (uint32_t)w, (uint32_t)h };
}

Extent2D SDLWindowObj::GetFrameBufferSize() const {
    int w, h;
    SDL_GetWindowSizeInPixels(static_cast<SDL_Window*>(m_Window), &w, &h);
    return { (uint32_t)(w), (uint32_t)(h) };
}

Extent2D SDLWindowObj::GetFrameBufferScale() const {
    float scale = SDL_GetWindowDisplayScale(static_cast<SDL_Window*>(m_Window));
    return { (uint32_t)scale, (uint32_t)scale };
}

float SDLWindowObj::GetPixelRatio() const {
    Extent2D size   = GetSize();
    Extent2D fbSize = GetFrameBufferSize();
    return (float)fbSize.Width / (float)size.Width;
}

std::vector<const char*> SDLWindowObj::GetRequiredExtensions(std::string& err) const {
    uint32_t count          = 0;
    const char* const* exts = SDL_Vulkan_GetInstanceExtensions(&count);
    if (exts == nullptr) {
        err = SDL_GetError();
        return {};
    }

    std::vector<const char*> result;
    result.reserve(count);
    for (uint32_t i = 0; i < count; ++i) {
        result.push_back(exts[i]);
    }
    return result;
}

WindowHandle SDLWindowObj::GetNativeHandle() const {
    SDL_PropertiesID props = SDL_GetWindowProperties(static_cast<SDL_Window*>(m_Window));
#ifdef CZ_PLATFORM_WINDOWS
    return (WindowHandle)SDL_GetPointerProperty(props, "SDL.window.win32.hwnd", nullptr);
#elif defined(CZ_PLATFORM_LINUX)
    return (WindowHandle)SDL_GetPointerProperty(props, "SDL.window.x11.window", nullptr);
#elif defined(CZ_PLATFORM_MACOS)
    return (WindowHandle)SDL_GetPointerProperty(props, "SDL.window.cocoa.window", nullptr);
#else
    return nullptr;
#endif
}

} // namespace CZ

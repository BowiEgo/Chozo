#include <Runtime/Window/Window.h>

#include <Core/Event/AppEvent.h>
#include <Core/Event/KeyEvent.h>
#include <Core/Event/MouseEvent.h>
#include <Core/Log/LogMacros.h>
#include <Core/Platform/Platform.h>

#include "SDLKeyMap.h"

#include <SDL3/SDL.h>
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

Window::~Window() { Shutdown(); }

void Window::Shutdown() {
    if (m_Window) {
        SDL_DestroyWindow(static_cast<SDL_Window*>(m_Window));
        m_Window = nullptr;
    }
    if (m_BackendInitialized) {
        SDL_Quit();
        m_BackendInitialized = false;
    }
}

bool Window::Init(std::string& err) {
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

    float scale             = SDL_GetWindowDisplayScale(static_cast<SDL_Window*>(m_Window));
    m_Spec.FrameBufferScale = { (uint32_t)scale, (uint32_t)scale };

    int w, h;
    SDL_GetWindowSize(static_cast<SDL_Window*>(m_Window), &w, &h);
    m_Spec.Size = { (uint32_t)w, (uint32_t)h };

    Extent2D fbSize   = GetFrameBufferSize();
    m_Spec.PixelRatio = (float)fbSize.Width / (float)w;

    return true;
}

void Window::OnUpdate() {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        // ProcessEventWithPreprocessor(event);
        ProcessEvent(event, m_ShouldClose, static_cast<SDL_Window*>(m_Window),
                     m_Spec.EventCallback);
    }
}

bool Window::ShouldClose() const { return m_ShouldClose; }

Extent2D Window::GetSize() const {
    int w, h;
    SDL_GetWindowSize(static_cast<SDL_Window*>(m_Window), &w, &h);
    return { (uint32_t)w, (uint32_t)h };
}

Extent2D Window::GetFrameBufferSize() const {
    int w, h;
    SDL_GetWindowSizeInPixels(static_cast<SDL_Window*>(m_Window), &w, &h);
    return { (uint32_t)(w), (uint32_t)(h) };
}

std::vector<const char*> Window::GetRequiredExtensions(std::string& err) const {
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

WindowHandle Window::GetNativeHandle() const {
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

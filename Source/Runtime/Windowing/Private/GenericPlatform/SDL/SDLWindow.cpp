#include "SDLWindow.h"
#include "ApplicationEvent.h"
#include "KeyEvent.h"
#include "MouseEvent.h"
#include "SDLKeyMap.h"

#include <SDL3/SDL_vulkan.h>

DEFINE_LOG_CATEGORY(LogSDLWindow);

bool CSDLWindow::s_SDLInitialized = false;

CSDLWindow::~CSDLWindow() { Shutdown(); }

void CSDLWindow::Init() { CreateSDLWindow(); }

void CSDLWindow::Shutdown() {
    if (m_Window) {
        SDL_DestroyWindow(GetSDLWindow());
        m_Window = nullptr;
    }
    if (s_SDLInitialized) {
        SDL_Quit();
        s_SDLInitialized = false;
    }
}

void CSDLWindow::CreateSDLWindow() {
    if (!s_SDLInitialized) {
        if (!SDL_Init(SDL_INIT_VIDEO)) {
            CZ_LOG(LogSDLWindow, Error, "SDL_Init failed: %s", SDL_GetError());
            return;
        }
        s_SDLInitialized = true;
    }

    uint32_t flags = SDL_WINDOW_RESIZABLE;
    if (m_Definition.bDecorated) flags |= SDL_WINDOW_BORDERLESS;
    if (m_Definition.bFullscreen) flags |= SDL_WINDOW_FULLSCREEN;
    if (m_Definition.bFocused) flags |= SDL_WINDOW_INPUT_FOCUS;
    flags |= SDL_WINDOW_VULKAN;
    flags |= SDL_WINDOW_HIGH_PIXEL_DENSITY;

    m_Window = SDL_CreateWindow(m_Definition.Title.c_str(), m_Definition.Size.Width,
                                m_Definition.Size.Height, flags);

    if (!m_Window) {
        CZ_LOG(LogSDLWindow, Error, "SDL_CreateWindow failed: %s", SDL_GetError());
        return;
    }

    float scale                   = SDL_GetWindowDisplayScale(GetSDLWindow());
    m_Definition.FrameBufferScale = { (uint32_t)scale, (uint32_t)scale };

    int w, h;
    SDL_GetWindowSize(GetSDLWindow(), &w, &h);
    m_Definition.Size = { (uint32_t)w, (uint32_t)h };

    FExtent2D fbSize        = GetFrameBufferSize();
    m_Definition.PixelRatio = (float)fbSize.Width / (float)w;
}

void CSDLWindow::OnUpdate() {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        ProcessEventWithPreprocessor(event);
        ProcessEvent(event);
    }
}

void CSDLWindow::ProcessEvent(const SDL_Event& event) {
    switch (event.type) {
        case SDL_EVENT_QUIT: m_bShouldClose = true; break;
        case SDL_EVENT_WINDOW_CLOSE_REQUESTED: m_bShouldClose = true; break;
        case SDL_EVENT_WINDOW_RESIZED:
        case SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED: {
            int w, h;
            SDL_GetWindowSize(GetSDLWindow(), &w, &h);
            FWindowResizedEvent e(w, h);
            m_Definition.EventCallback(e);
            break;
        }
        case SDL_EVENT_WINDOW_DISPLAY_SCALE_CHANGED: {
            float scale = SDL_GetWindowDisplayScale(GetSDLWindow());
            FWindowContentScaledEvent e(scale, scale);
            m_Definition.EventCallback(e);
            break;
        }
        case SDL_EVENT_KEY_DOWN:
        case SDL_EVENT_KEY_UP: {
            EKeyCode key = SDLScancodeToEngineKey(event.key.scancode);
            int repeat   = event.key.repeat;
            if (event.type == SDL_EVENT_KEY_DOWN) {
                FKeyPressedEvent e(key, repeat);
                m_Definition.EventCallback(e);
            } else {
                FKeyReleasedEvent e(key);
                m_Definition.EventCallback(e);
            }
            break;
        }
        case SDL_EVENT_TEXT_INPUT: {
            const char* text = event.text.text;
            if (text && text[0] != 0) {
                FKeyTypedEvent e((EKeyCode)text[0]);
                m_Definition.EventCallback(e);
            }
            break;
        }
        case SDL_EVENT_MOUSE_BUTTON_DOWN:
        case SDL_EVENT_MOUSE_BUTTON_UP: {
            EMouseButton btn = static_cast<EMouseButton>(event.button.button);
            if (event.type == SDL_EVENT_MOUSE_BUTTON_DOWN) {
                FMouseButtonPressedEvent e(btn);
                m_Definition.EventCallback(e);
            } else {
                FMouseButtonReleasedEvent e(btn);
                m_Definition.EventCallback(e);
            }
            break;
        }
        case SDL_EVENT_MOUSE_MOTION: {
            float x = (float)event.motion.x;
            float y = (float)event.motion.y;
            FMouseMovedEvent e(x, y);
            m_Definition.EventCallback(e);
            break;
        }
        case SDL_EVENT_MOUSE_WHEEL: {
            float xOff = (float)event.wheel.x;
            float yOff = (float)event.wheel.y;
            FMouseScrolledEvent e(xOff, yOff);
            m_Definition.EventCallback(e);
            break;
        }
        default: break;
    }
}

bool CSDLWindow::ShouldClose() const { return m_bShouldClose; }

FExtent2D CSDLWindow::GetSize() const {
    int w, h;
    SDL_GetWindowSize(GetSDLWindow(), &w, &h);
    return { (uint32_t)w, (uint32_t)h };
}

FExtent2D CSDLWindow::GetFrameBufferSize() const {
    int w, h;
    SDL_GetWindowSizeInPixels(GetSDLWindow(), &w, &h);
    return { (uint32_t)(w), (uint32_t)(h) };
}

std::vector<const char*> CSDLWindow::GetRequiredExtensions() const {
    uint32_t count          = 0;
    const char* const* exts = SDL_Vulkan_GetInstanceExtensions(&count);
    if (exts == nullptr) {
        CZ_LOG(LogSDLWindow, Error, "SDL_Vulkan_GetInstanceExtensions failed: %s", SDL_GetError());
        return {};
    }

    std::vector<const char*> result;
    result.reserve(count);
    for (uint32_t i = 0; i < count; ++i) {
        result.push_back(exts[i]);
    }
    return result;
}

FWindowHandle CSDLWindow::GetNativeHandle() const {
    SDL_PropertiesID props = SDL_GetWindowProperties(GetSDLWindow());
#ifdef CZ_PLATFORM_WINDOWS
    return (FWindowHandle)SDL_GetPointerProperty(props, "SDL.window.win32.hwnd", nullptr);
#elif defined(CZ_PLATFORM_LINUX)
    return (FWindowHandle)SDL_GetPointerProperty(props, "SDL.window.x11.window", nullptr);
#elif defined(CZ_PLATFORM_MACOS)
    return (FWindowHandle)SDL_GetPointerProperty(props, "SDL.window.cocoa.window", nullptr);
#else
    return nullptr;
#endif
}
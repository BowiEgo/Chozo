#pragma once

#include <Core/Event/Event.h>
#include <Core/Header/Extent.h>
#include <Core/Memory/Memory.h>

#include <atomic>
#include <functional>
#include <string>
#include <vector>

namespace CZ {

using WindowHandle  = void*;
using WindowSurface = void*;

struct WindowSpecifaciton {
    std::string Title;
    Extent2D Size;
    Extent2D FrameBufferScale;
    float PixelRatio;
    bool VSync;
    bool IsDecorated  = true;
    bool IsFullscreen = false;
    bool IsFocused    = true;
    EventCallback EventCallback;

    WindowSpecifaciton() : Title("Chozo Engine"), Size({ 1280, 720 }) {}
};

class Window {
public:
    Window(const WindowSpecifaciton& def) : m_Spec(def) {};
    ~Window();

    bool Init(std::string& err);
    void Shutdown();
    void OnUpdate();
    bool ShouldClose() const;
    void SetEventCallback(const EventCallback& callback) { m_Spec.EventCallback = callback; }

    void SetVSync(bool enabled) {
        if (m_Spec.VSync != enabled) {
            m_Spec.VSync = enabled;
            m_VSyncDirty.store(true);
        }
    }
    bool IsVSyncEnabled() const { return m_Spec.VSync; }
    bool CheckAndResetVSyncDirty() { return m_VSyncDirty.exchange(false); }

    Extent2D GetSize() const;
    Extent2D GetFrameBufferSize() const;
    Extent2D GetFrameBufferScale() const;
    std::vector<const char*> GetRequiredExtensions(std::string& err) const;
    WindowHandle GetWindowWrapper() const { return m_Window; }
    WindowHandle GetNativeHandle() const;

    float GetPixelRatio() const { return m_Spec.PixelRatio; }
    bool IsVSync() const { return m_Spec.VSync; }

    // void SetEventPreprocessor(std::function<void(const SDL_Event&)> preprocessor) {
    //     m_EventPreprocessor = preprocessor;
    // }

protected:
    // void ProcessEventWithPreprocessor(const SDL_Event& event) {
    //     if (m_EventPreprocessor) m_EventPreprocessor(event);
    // }

protected:
    WindowSpecifaciton m_Spec;
    WindowHandle m_Window{ nullptr };
    std::atomic_bool m_VSyncDirty{ false };

    // std::function<void(const SDL_Event&)> m_EventPreprocessor;

    bool m_BackendInitialized = false;
    bool m_ShouldClose        = false;
};

} // namespace CZ

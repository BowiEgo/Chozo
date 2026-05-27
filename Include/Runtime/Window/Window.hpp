#pragma once

#include <Core/Event/Event.hpp>
#include <Core/Header/Extent.hpp>
#include <Core/Header/Handle.hpp>

#include <string>

namespace CZ {

using WindowHandle  = void*;
using WindowSurface = void*;

struct WindowSpecifaciton {
    std::string Title;
    Extent2D Size;
    bool VSync;
    bool IsDecorated  = true;
    bool IsFullscreen = false;
    bool IsFocused    = true;
    EventCallback EventCallback;

    WindowSpecifaciton() : Title("Chozo Engine"), Size({ 1280, 720 }) {}
};

class WindowObj {
public:
    WindowObj(const WindowSpecifaciton& spec) : m_Spec(spec) {}
    virtual ~WindowObj() = default;

    virtual bool Init(std::string& err) = 0;

    virtual void Shutdown() = 0;

    virtual void OnUpdate() = 0;

    virtual bool ShouldClose() const = 0;

    virtual Extent2D GetSize() const = 0;

    virtual Extent2D GetFrameBufferSize() const = 0;

    virtual Extent2D GetFrameBufferScale() const = 0;

    virtual float GetPixelRatio() const = 0;

    virtual std::vector<const char*> GetRequiredExtensions(std::string& err) const = 0;

    virtual WindowHandle GetNativeHandle() const = 0;

    WindowHandle GetWindowWrapper() const { return m_Window; }

    void SetEventCallback(const EventCallback& callback) { m_Spec.EventCallback = callback; }

    void SetVSync(bool enabled) {
        if (m_Spec.VSync != enabled) {
            m_Spec.VSync = enabled;
            m_VSyncDirty.store(true);
        }
    }
    bool IsVSyncEnabled() const { return m_Spec.VSync; }

    bool CheckAndResetVSyncDirty() { return m_VSyncDirty.exchange(false); }

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

struct Window : Handle<class WindowObj> {
    static Window Create(const WindowSpecifaciton& spec);

    template <typename T> T* As() { return static_cast<T*>(InternalHandleReader::Unwrap(*this)); }
};

} // namespace CZ

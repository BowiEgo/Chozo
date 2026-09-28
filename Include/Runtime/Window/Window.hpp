#pragma once
#include <atomic>
#include <utility>
#include <vector>

#include <Core/Event/Event.hpp>
#include <Core/Event/InputImpl.hpp>
#include <Core/Header/Extent.hpp>
#include <Core/Header/Handle.hpp>

#include <functional>
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

    /// Optional hook that sees platform events before they are translated into engine events.
    /// The pointer is platform specific and opaque to callers (SDL events on the SDL backend);
    /// this is what lets a UI backend consume input without the window knowing about ImGui.
    using EventPreprocessor = std::function<void(const void* event)>;
    void SetEventPreprocessor(EventPreprocessor preprocessor) {
        m_EventPreprocessor = std::move(preprocessor);
    }

    WindowHandle GetWindowWrapper() const { return m_Window; }

    void SetEventCallback(const EventCallback& callback) { m_Spec.EventCallback = callback; }

    /// Called by the platform backend before an event is translated; keeps `EventPreprocessor`
    /// a window-internal detail.
    void ProcessEventPreprocessor(const void* event) const {
        if (m_EventPreprocessor) m_EventPreprocessor(event);
    }

    void SetVSync(bool enabled) {
        if (m_Spec.VSync != enabled) {
            m_Spec.VSync = enabled;
            m_VSyncDirty.store(true);
        }
    }
    bool IsVSyncEnabled() const { return m_Spec.VSync; }

    bool CheckAndResetVSyncDirty() { return m_VSyncDirty.exchange(false); }

protected:
protected:
    WindowSpecifaciton m_Spec;
    WindowHandle m_Window{ nullptr };
    EventPreprocessor m_EventPreprocessor;
    Scope<InputImpl> m_InputImpl;
    std::atomic_bool m_VSyncDirty{ false };

    bool m_BackendInitialized = false;
    bool m_ShouldClose        = false;
};

struct Window : Handle<class WindowObj> {
    using Handle<class WindowObj>::Handle;

    /// Creates a window owned by the caller.
    static Scope<WindowObj> Create(const WindowSpecifaciton& spec);

    template <typename T> T* As() { return static_cast<T*>(InternalHandleReader::Unwrap(*this)); }
};

} // namespace CZ

#pragma once

#include <Core/Event/Event.h>
#include <Core/Header/Extent.h>
#include <Core/Header/Handle.h>

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

class WindowObj;

struct Window : Handle<class WindowObj> {
    static Window Create(const WindowSpecifaciton& spec);

    template <typename T> T* As() { return static_cast<T*>(WindowInternalReader::Unwrap(*this)); }

    bool Init(std::string& err);
    void Shutdown();
    void OnUpdate();
    bool ShouldClose() const;
    Extent2D GetSize() const;
    Extent2D GetFrameBufferSize() const;
    Extent2D GetFrameBufferScale() const;
    std::vector<const char*> GetRequiredExtensions(std::string& err) const;
    WindowHandle GetNativeHandle() const;

    WindowHandle GetWindowWrapper() const;

    void SetEventCallback(const EventCallback& callback);

    void SetVSync(bool enabled);

    bool IsVSyncEnabled() const;
    bool CheckAndResetVSyncDirty();

    float GetPixelRatio() const;
};

} // namespace CZ

#pragma once

namespace Chozo {

using WindowHandle = void *;
using WindowSurface = void *;

class IRendererWindow {
public:
    virtual ~IRendererWindow() = default;

    virtual void GetFramebufferSize(int *width, int *height) const = 0;
    virtual std::vector<const char *> GetRequiredExtensions() const = 0;
    virtual WindowHandle GetWindowWrapper() const = 0;
    virtual WindowHandle GetNativeHandle()
        const = 0; // Windows HWND, Linux Window, MacOS NSWindow*
};
} // namespace Chozo
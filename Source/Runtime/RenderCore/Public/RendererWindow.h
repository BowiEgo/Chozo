#pragma once

using FWindowHandle = void *;
using FWindowSurface = void *;

class IRendererWindow {
public:
    virtual ~IRendererWindow() = default;

    virtual void GetFramebufferSize(int *width, int *height) const = 0;
    virtual std::vector<const char *> GetRequiredExtensions() const = 0;
    virtual FWindowHandle GetWindowWrapper() const = 0;
    virtual FWindowHandle GetNativeHandle()
        const = 0; // Windows HWND, Linux Window, MacOS NSWindow*
};
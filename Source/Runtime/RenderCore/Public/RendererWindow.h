#pragma once

#include "CoreMinimal.h"
#include "RHITypes.h"
#include "RenderCoreExport.h"

using FWindowHandle = void*;
using FWindowSurface = void*;

class RENDER_CORE_API IRendererWindow {
public:
    virtual ~IRendererWindow() = default;

    virtual FExtent2D GetLogicalSize() const = 0;
    virtual FExtent2D GetPhysicalSize() const = 0;
    virtual std::vector<const char*> GetRequiredExtensions() const = 0;
    virtual FWindowHandle GetWindowWrapper() const = 0;
    virtual FWindowHandle
        GetNativeHandle() const = 0; // Windows HWND, Linux Window, MacOS NSWindow*
};

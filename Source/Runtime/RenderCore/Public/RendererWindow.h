#pragma once

#include "RHITypes.h"

#include "CoreMinimal.h"
#include "RenderCoreExport.h"

using FWindowHandle  = void*;
using FWindowSurface = void*;

class RENDER_CORE_API IRendererWindow {
public:
    virtual ~IRendererWindow() = default;

    virtual void SetVSync(bool enabled)    = 0;
    virtual bool IsVSyncEnabled() const    = 0;
    virtual bool CheckAndResetVSyncDirty() = 0;

    virtual FExtent2D GetSize() const                              = 0;
    virtual FExtent2D GetFrameBufferSize() const                   = 0;
    virtual FExtent2D GetFrameBufferScale() const                  = 0;
    virtual std::vector<const char*> GetRequiredExtensions() const = 0;
    virtual FWindowHandle GetWindowWrapper() const                 = 0;
    virtual FWindowHandle
        GetNativeHandle() const = 0; // Windows HWND, Linux Window, MacOS NSWindow*
};

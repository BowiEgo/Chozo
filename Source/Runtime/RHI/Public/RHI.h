#pragma once

#include "RHIDevice.h"
#include "RHIExport.h"
#include "RHISwapchain.h"
#include "Ref.h"

struct FRHICreateInfo {
    // --- Windowing ---
    // [Note] Raw window handle (HWND on Windows, Window on X11)
    void* WindowHandle = nullptr;
    std::vector<const char*> RequiredExtensions;
};

class RHI_API IRHI : public FRefCounted {
public:
    IRHI();
    virtual ~IRHI();

    virtual void CreateDevice(const FRHIDeviceCreateInfo& info) = 0;
    virtual void CreateSwapchain(const FRHISwapchainCreateInfo& info) = 0;

    virtual TRef<IRHIDevice> GetDevice() const = 0;
};
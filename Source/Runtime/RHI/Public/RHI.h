#pragma once

#include "RHICommandBuffer.h"
#include "RHIDevice.h"
#include "RHIExport.h"
#include "RHISwapchain.h"
#include "Ref.h"

DECLARE_LOG_CATEGORY_EXTERN(LogRHI, Info);

struct FRHICreateInfo {
    // --- Windowing ---
    // [Note] Raw window handle (HWND on Windows, Window on X11)
    void* NativeWindow = nullptr;
    std::vector<const char*> RequiredExtensions;
};

class RHI_API IRHI : public FRefCounted {
public:
    IRHI();
    virtual ~IRHI();

    virtual TRef<IRHIDevice> CreateDevice(const FRHIDeviceCreateInfo& info) = 0;
    virtual TRef<IRHISwapchain> CreateSwapchain(const FRHISwapchainCreateInfo& info) = 0;
    virtual TRef<IRHISyncObject> CreateSyncObject() = 0;
    virtual TRef<IRHICommandBuffer> CreateCommandBuffer() = 0;

    virtual TRef<IRHIDevice> GetDevice() const = 0;
    virtual TRef<IRHISwapchain> GetSwapchain() const = 0;
};

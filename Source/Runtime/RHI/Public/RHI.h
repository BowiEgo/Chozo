#pragma once

#include "RHICommandBuffer.h"
#include "RHIDevice.h"
#include "RHIExport.h"
#include "RHIPipeline.h"
#include "RHISwapchain.h"
#include "RHISyncObject.h"
#include "Ref.h"

DECLARE_LOG_CATEGORY_EXTERN(LogRHI, Info);

using RecordCallback = std::function<void(uint32)>;

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

    // virtual void RecordCommandBuffer(const TRef<IRHICommandBuffer>& cmd,
    //                                  const TRef<IRHIPipeline>& pipeline,
    //                                  const uint32 imageIndex) = 0;
    virtual void DrawFrame(const TRef<IRHICommandBuffer>& cmd,
                           const TRef<IRHISyncObject>& syncObject,
                           RecordCallback recordCallback) = 0;
    virtual TRef<IRHIDevice> CreateDevice(const FRHIDeviceCreateInfo& info) = 0;
    virtual TRef<IRHISwapchain> CreateSwapchain(const FRHISwapchainCreateInfo& info) = 0;
    virtual TRef<IRHISyncObject> CreateSyncObject() = 0;
    virtual TRef<IRHICommandBuffer> CreateCommandBuffer() = 0;

    virtual void BeginRenderingToSwapchain(const TRef<IRHICommandBuffer>& cmd, uint32 imageIndex,
                                           bool bClear) = 0;
    virtual void EndRendering(const TRef<IRHICommandBuffer>& cmd) = 0;

    virtual TRef<IRHIDevice> GetDevice() const = 0;
    virtual TRef<IRHISwapchain> GetSwapchain() const = 0;
    virtual TRef<IRHICommandPool> GetCommandPool() const = 0;
};

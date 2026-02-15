#pragma once

#include "RHIDevice.h"
#include "RHIExport.h"
#include "Ref.h"

DECLARE_LOG_CATEGORY_EXTERN(LogRHISwapchain, Info);

struct FRHISwapchainCreateInfo {
    std::string Name;
    FExtent2D FrameBufferSize;
    void* NativeWindow = nullptr;
};

class RHI_API IRHISwapchain : public FRefCounted {
public:
    IRHISwapchain(const FRHISwapchainCreateInfo& info);
    virtual ~IRHISwapchain();

    virtual const uint32 AcquireNextImage(TRef<IRHISyncObject> semaphore) = 0;
    virtual const EPixelFormat GetImageFormat() = 0;

    virtual void RecreateSwapchain(const FExtent2D& frameBufferSize) = 0;

protected:
    FRHISwapchainCreateInfo m_Info;
};

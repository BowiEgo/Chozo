#pragma once

#include "RHIDevice.h"
#include "RHIExport.h"
#include "Ref.h"

struct FRHISwapchainCreateInfo {
    std::string Name;
    TRef<IRHIDevice> Device;
    int FrameBufferWidth, FrameBufferHeight;
    void* WindowHandle = nullptr;
};

class RHI_API IRHISwapchain : public FRefCounted {
public:
    IRHISwapchain(const FRHISwapchainCreateInfo& info);
    virtual ~IRHISwapchain();

protected:
    FRHISwapchainCreateInfo m_Data;
};
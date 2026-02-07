#pragma once

#include "RHIDevice.h"
#include "RHIExport.h"
#include "Ref.h"

struct FRHISwapchainCreateInfo {
    std::string Name;
    int FrameBufferWidth, FrameBufferHeight;
    void* WindowHandle = nullptr;
};

class RHI_API IRHISwapchain : public FRefCounted {
public:
    IRHISwapchain(const FRHISwapchainCreateInfo& info);
    virtual ~IRHISwapchain();

    virtual const EPixelFormat GetImageFormat() const = 0;

protected:
    FRHISwapchainCreateInfo m_Info;
};
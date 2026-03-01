#pragma once

#include "RHIDevice.h"
#include "RHIExport.h"
#include "RHITexture2D.h"
#include "Ref.h"

DECLARE_LOG_CATEGORY_EXTERN(LogRHISwapchain, Info);

struct FSwapchainSpecification {
    std::string Name;
    FExtent2D FrameBufferSize;
    void* NativeWindow = nullptr;
};

class RHI_API IRHISwapchain : public FRefCounted {
public:
    IRHISwapchain(const FSwapchainSpecification& spec);
    virtual ~IRHISwapchain();

    virtual const EPixelFormat GetImageFormat() const = 0;
    virtual const EPixelFormat GetDepthFormat() const = 0;
    virtual const FExtent2D GetExtent() const = 0;

    virtual void SetPresentMode(const EPresentMode mode) = 0;
    virtual void Recreate(const FExtent2D& frameBufferSize) = 0;

    TRef<IRHITexture2D> GetColorAttachment(uint32 index) { return m_ColorAttachments[index]; }

protected:
    FSwapchainSpecification m_Spec;
    std::vector<TRef<IRHITexture2D>> m_ColorAttachments;
    EPresentMode m_PresentMode = EPresentMode::FIFO;
};

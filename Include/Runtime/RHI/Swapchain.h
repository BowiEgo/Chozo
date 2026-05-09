#pragma once

#include <Core/Header/Extent.h>
#include <Core/Header/Handle.h>
#include <Core/Memory/Memory.h>
#include <Runtime/RHI/Device.h>
#include <Runtime/RHI/RHITypes.h>
#include <Runtime/RHI/Texture.h>

namespace CZ {

struct SwapchainSpecification {
    std::string Name;
    Extent2D FrameBufferSize;
    void* NativeWindow = nullptr;
};

class SwapchainObj {
public:
    SwapchainObj(const Device device, const SwapchainSpecification& spec)
        : m_Device(device), m_Spec(spec) {}
    virtual ~SwapchainObj() = default;

    virtual PixelFormat GetImageFormat() const = 0;
    virtual PixelFormat GetDepthFormat() const = 0;
    virtual const Extent2D GetExtent() const   = 0;

    virtual void SetPresentMode(const PresentMode mode)    = 0;
    virtual void Recreate(const Extent2D& frameBufferSize) = 0;

protected:
    Device m_Device;
    SwapchainSpecification m_Spec;

    PresentMode m_PresentMode = PresentMode::FIFO;
    std::vector<Texture> m_ColorAttachments;
};

struct Swapchain : Handle<SwapchainObj> {
    static void Destroy(Swapchain Swapchain);
};

} // namespace CZ

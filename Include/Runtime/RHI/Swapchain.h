#pragma once

#include <Core/Header/Extent.h>
#include <Core/Header/Handle.h>
#include <Core/Memory/Memory.h>
#include <Runtime/RHI/Fence.h>
#include <Runtime/RHI/RHITypes.h>
#include <Runtime/RHI/Semaphore.h>
#include <Runtime/RHI/Texture.h>

namespace CZ {

struct SwapchainSpecification {
    std::string Name;
    Extent2D FrameBufferSize;
    void* NativeWindow = nullptr;

    uint32 MaxFramesInFlight;
};

class SwapchainObj;

struct Swapchain : Handle<class SwapchainObj> {

    // static void Destroy(Swapchain swapchain);

    template <typename T> T* As() { return static_cast<T*>(InternalHandleReader::Unwrap(*this)); }

    PixelFormat GetImageFormat() const;

    PixelFormat GetDepthFormat() const;

    uint32 GetImageCount() const;

    Fence GetFence(uint32 currentFrame) const;

    Semaphore GetImageAvailableSemaphore(uint32 currentFrame) const;

    Texture GetColorAttachment(uint32 index);

    void SetCurrentImageIndex(uint32 imageIndex);
};

} // namespace CZ

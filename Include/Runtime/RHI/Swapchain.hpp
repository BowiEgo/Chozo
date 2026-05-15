#pragma once

#include <Core/Header/Extent.hpp>
#include <Core/Header/Handle.hpp>
#include <Core/Memory/Memory.hpp>
#include <Runtime/RHI/Fence.hpp>
#include <Runtime/RHI/RHITypes.hpp>
#include <Runtime/RHI/Semaphore.hpp>
#include <Runtime/RHI/Texture.hpp>

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

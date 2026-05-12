#pragma once

#include "Core/Header/Types.h"
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

class SwapchainObj;

struct Swapchain : Handle<SwapchainObj> {
    static void Destroy(Swapchain Swapchain);

    PixelFormat GetImageFormat() const;

    PixelFormat GetDepthFormat() const;

    uint32 GetImageCount() const;
};

} // namespace CZ

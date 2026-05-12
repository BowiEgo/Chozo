#pragma once

#include <Core/Header/Extent.h>
#include <Core/Header/Handle.h>
#include <Core/Header/Types.h>
#include <Core/Memory/Memory.h>
#include <Runtime/RHI/RHITypes.h>

#include <Runtime/RHI/Device.h>
#include <Runtime/RHI/Swapchain.h>

#include <vector>

namespace CZ {

struct GraphicsContextSpecification {
    Extent2D FrameBufferSize;
    void* NativeWindow;
    std::vector<const char*> WindowRequiredExtensions;

    // --- Debugging ---
    bool EnableValidationLayers = true;
    bool EnableGPUProfiling     = false;
};

class GraphicsContextObj;

struct GraphicsContext : Handle<GraphicsContextObj> {
    static void Destroy(GraphicsContext context);

    uint32 GetMaxFramesInFlight() const { return 2; }

    Device GetDevice();

    Swapchain GetSwapchain();
};

} // namespace CZ

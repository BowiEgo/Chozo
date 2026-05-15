#pragma once

#include <Core/Header/Extent.hpp>
#include <Core/Header/Handle.hpp>
#include <Core/Header/Types.h>
#include <Core/Memory/Memory.hpp>
#include <Runtime/RHI/RHITypes.hpp>

#include <Runtime/RHI/Device.hpp>
#include <Runtime/RHI/Swapchain.hpp>

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

struct GraphicsContext : Handle<class GraphicsContextObj> {

    static GraphicsContext Create(const GraphicsContextSpecification& spec);

    // static void Destroy(GraphicsContext ctx);

    template <typename T> T* As() { return static_cast<T*>(InternalHandleReader::Unwrap(*this)); }

    uint32 GetMaxFramesInFlight() const;

    uint32 GetCurrentFrameIndex() const;

    Device GetDevice();

    Swapchain GetSwapchain();

    void End();
};

} // namespace CZ

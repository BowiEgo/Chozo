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

struct NativeRenderContext {
    void* Instance;

    void* PhysicalDevice;
    void* Device;
    uint32 GraphicsQueueIndex;
    void* GraphicsQueue;
    void* GlobalDescriptorPool;

    void* Swapchain;
};

class GraphicsContextObj {
public:
    GraphicsContextObj(const GraphicsContextSpecification& spec) : m_Spec(spec) {}
    virtual ~GraphicsContextObj() = default;

    GraphicsContextObj(const GraphicsContextObj&)            = delete;
    GraphicsContextObj& operator=(const GraphicsContextObj&) = delete;
    GraphicsContextObj(GraphicsContextObj&&)                 = delete;
    GraphicsContextObj& operator=(GraphicsContextObj&&)      = delete;

    void SetCurrentFrame(uint32 frame) { m_CurrentFrame = frame; }

    uint32 GetMaxFramesInFlight() const { return 2; }

    uint32 GetCurrentFrameIndex() const { return m_CurrentFrameIndex; }

    uint32 GetCurrentFrame() const { return m_CurrentFrame; }

    Device GetDevice() { return m_Device; }

    Swapchain GetSwapchain() { return m_Swapchain; }

    void End() { m_CurrentFrameIndex = (m_CurrentFrameIndex + 1) % GetMaxFramesInFlight(); }

protected:
    GraphicsContextSpecification m_Spec;

    Device m_Device;
    Swapchain m_Swapchain;

    uint32 m_CurrentFrameIndex = 0;
    uint32 m_CurrentFrame      = 0;
};

struct GraphicsContext : Handle<class GraphicsContextObj> {
    static GraphicsContext Create(const GraphicsContextSpecification& spec);

    template <typename T> T* As() { return static_cast<T*>(InternalHandleReader::Unwrap(*this)); }
};

} // namespace CZ

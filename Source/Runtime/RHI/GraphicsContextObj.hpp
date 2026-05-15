#include "Core/Header/Types.h"
#include <Runtime/RHI/GraphicsContext.hpp>

namespace CZ {

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

    uint32 GetMaxFramesInFlight() const { return 2; }

    uint32 GetCurrentFrameIndex() const { return m_CurrentFrameIndex; }

    Device GetDevice() { return m_Device; }

    Swapchain GetSwapchain() { return m_Swapchain; }

    void End() { m_CurrentFrameIndex = (m_CurrentFrameIndex + 1) % GetMaxFramesInFlight(); }

protected:
    GraphicsContextSpecification m_Spec;

    Device m_Device;
    Swapchain m_Swapchain;

    uint32 m_CurrentFrameIndex = 0;
};
} // namespace CZ
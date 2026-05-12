#include <Runtime/RHI/GraphicsContext.h>

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
    static const int MAX_FRAMES_IN_FLIGHT = 2;

public:
    GraphicsContextObj(const GraphicsContextSpecification& spec) : m_Spec(spec) {}
    virtual ~GraphicsContextObj() = default;

    GraphicsContextObj(const GraphicsContextObj&)            = delete;
    GraphicsContextObj& operator=(const GraphicsContextObj&) = delete;
    GraphicsContextObj(GraphicsContextObj&&)                 = delete;
    GraphicsContextObj& operator=(GraphicsContextObj&&)      = delete;

    Device GetDevice() { return m_Device; }

    Swapchain GetSwapchain() { return m_Swapchain; }

protected:
    GraphicsContextSpecification m_Spec;

    Device m_Device;
    Swapchain m_Swapchain;
};
} // namespace CZ
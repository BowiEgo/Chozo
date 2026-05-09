#pragma once

#include <Core/Header/Extent.h>
#include <Core/Header/Handle.h>
#include <Core/Memory/Memory.h>

#include <vector>

namespace CZ {

struct GraphicContextSpecification {
    Extent2D FrameBufferSize;
    void* NativeWindow;
    std::vector<const char*> WindowRequiredExtensions;

    // --- Debugging ---
    bool EnableValidationLayers = true;
    bool EnableGPUProfiling     = false;
};

class GraphicContextObj {
    static const int MAX_FRAMES_IN_FLIGHT = 2;

public:
    GraphicContextObj(const GraphicContextSpecification& spec) : m_Spec(spec) {}
    virtual ~GraphicContextObj() = default;

    GraphicContextObj(const GraphicContextObj&)            = delete;
    GraphicContextObj& operator=(const GraphicContextObj&) = delete;
    GraphicContextObj(GraphicContextObj&&)                 = delete;
    GraphicContextObj& operator=(GraphicContextObj&&)      = delete;

protected:
    GraphicContextSpecification m_Spec;
};

struct GraphicContext : Handle<GraphicContextObj> {
    static void Destroy(GraphicContext context);
};

} // namespace CZ

#pragma once

#include <Core/Header/Handle.hpp>
#include <Core/Header/Types.h>
#include <Core/Memory/Memory.hpp>
#include <Runtime/RHI/Pipeline.hpp>
#include <Runtime/RHI/RHITypes.hpp>
// #include <Runtime/RenderCore/Viewport.hpp>

namespace CZ {

struct Pipeline;
struct DescriptorSet;
struct GraphicsBuffer;

struct RenderViewport {
    float x, y, width, height, minDepth, maxDepth;
};

struct RenderScissor {
    int32 x, y;
    uint32 width, height;
};

class CommandListObj {
public:
    CommandListObj()          = default;
    virtual ~CommandListObj() = default;

    CommandListObj(const CommandListObj&)            = delete;
    CommandListObj& operator=(const CommandListObj&) = delete;
    CommandListObj(CommandListObj&&)                 = delete;
    CommandListObj& operator=(CommandListObj&&)      = delete;

    virtual void Begin() = 0;

    virtual void SetViewport(const RenderViewport& viewport) = 0;

    virtual void SetScissor(const RenderScissor& scissor) = 0;

    virtual void SetPolygonMode(PolygonMode mode) = 0;

    virtual void BindPipeline(Pipeline pipeline) = 0;

    // virtual void BindDescriptorSets(int set, DescriptorSet descSet) = 0;

    // virtual void PushConstants(const void* data, uint32 size, uint32 offset) = 0;

    // virtual void BindVertexBuffer(GraphicsBuffer vertexBuffer, int binding) = 0;

    // virtual void BindIndexBuffer(GraphicsBuffer indexBuffer) = 0;

    virtual void DrawIndexed(uint32 indexCount) = 0;

    virtual void DrawIndexed(uint32 indexCount, uint32 instanceCount, uint32 firstIndex,
                             int32 vertexOffset, uint32 firstInstance) = 0;

    virtual void Draw(uint32 vertexCount, uint32 instanceCount, uint32 firstVertex,
                      uint32 firstInstance) = 0;

    virtual void End() = 0;

protected:
    Pipeline m_CurrentPipeline;
};

struct CommandList : Handle<class CommandListObj> {
    template <typename T> T* As() { return static_cast<T*>(InternalHandleReader::Unwrap(*this)); }
};

} // namespace CZ

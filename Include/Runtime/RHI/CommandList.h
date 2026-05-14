#pragma once

#include <Core/Header/Handle.h>
#include <Core/Header/Types.h>
#include <Core/Memory/Memory.h>
#include <Runtime/RHI/RHITypes.h>

namespace CZ {

class Viewport;
class Pipeline;
class DescriptorSet;
class GraphicsBuffer;

struct RenderViewport {
    float x, y, width, height, minDepth, maxDepth;
};

struct RenderScissor {
    int32 x, y;
    uint32 width, height;
};

class CommandListObj;

struct CommandList : Handle<class CommandListObj> {
    template <typename T> T* As() { return static_cast<T*>(RHIInternalReader::Unwrap(*this)); }

    void Begin();

    void SetViewport(const RenderViewport& viewport);

    void SetScissor(const RenderScissor& scissor);

    void SetPolygonMode(PolygonMode mode);

    // void BindPipeline(Pipeline pipeline);

    // void BindDescriptorSets(int set, DescriptorSet descSet);

    // void PushConstants(const void* data, uint32 size, uint32 offset);

    // void BindVertexBuffer(GraphicsBuffer vertexBuffer, int binding);

    // void BindIndexBuffer(GraphicsBuffer indexBuffer);

    void DrawIndexed(uint32 indexCount);

    void DrawIndexed(uint32 indexCount, uint32 instanceCount, uint32 firstIndex, int32 vertexOffset,
                     uint32 firstInstance);

    void Draw(uint32 vertexCount, uint32 instanceCount, uint32 firstVertex, uint32 firstInstance);

    void End();
};

} // namespace CZ

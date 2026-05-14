#include <Runtime/RHI/CommandList.h>
#include <Runtime/RHI/CommandPool.h>

namespace CZ {

class CommandListObj {
public:
    CommandListObj() {}
    virtual ~CommandListObj() { CZ_CORE_LOG(Error, "CommandListObj destructed"); }

    CommandListObj(const CommandListObj&)            = delete;
    CommandListObj& operator=(const CommandListObj&) = delete;
    CommandListObj(CommandListObj&&)                 = delete;
    CommandListObj& operator=(CommandListObj&&)      = delete;

    virtual void Begin() = 0;

    virtual void SetViewport(const RenderViewport& viewport) = 0;

    virtual void SetScissor(const RenderScissor& scissor) = 0;

    virtual void SetPolygonMode(PolygonMode mode) = 0;

    // virtual void BindPipeline(Pipeline pipeline) = 0;

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
};
} // namespace CZ
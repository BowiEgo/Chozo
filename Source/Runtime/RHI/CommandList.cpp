#include <Runtime/RHI/CommandList.h>

#include "CommandListObj.h"

namespace CZ {

DEFINE_LOG_CATEGORY_STATIC(LogCommandList, Info);

DEFINE_HANDLE_DESTROY(CommandListObj)

void CommandList::Begin() { m_Obj->Begin(); }

void CommandList::SetViewport(const RenderViewport& viewport) { m_Obj->SetViewport(viewport); }

void CommandList::SetScissor(const RenderScissor& scissor) { m_Obj->SetScissor(scissor); }

void CommandList::SetPolygonMode(PolygonMode mode) { m_Obj->SetPolygonMode(mode); }

// void CommandList::BindPipeline(Pipeline pipeline) { m_Obj->BindPipeline(pipeline); }

// void CommandList::BindDescriptorSets(int set, DescriptorSet descSet) {
// m_Obj->BindDescriptorSets(set, decsSet); }

// void CommandList::PushConstants(const void* data, uint32 size, uint32 offset) {
//     m_Obj->PushConstants(data, size, offset);
// }

// void CommandList::BindVertexBuffer(GraphicsBuffer vertexBuffer, int binding) {
// m_Obj->BindVertexBuffer(vertexBuffer, binding); }

// void CommandList::BindIndexBuffer(GraphicsBuffer indexBuffer) {
// m_Obj->BindIndexBuffer(indexBuffer); }

void CommandList::DrawIndexed(uint32 indexCount) { m_Obj->DrawIndexed(indexCount); }

void CommandList::DrawIndexed(uint32 indexCount, uint32 instanceCount, uint32 firstIndex,
                              int32 vertexOffset, uint32 firstInstance) {
    m_Obj->DrawIndexed(indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
}

void CommandList::Draw(uint32 vertexCount, uint32 instanceCount, uint32 firstVertex,
                       uint32 firstInstance) {
    m_Obj->Draw(vertexCount, instanceCount, firstVertex, firstInstance);
}

void CommandList::End() { m_Obj->End(); }

} // namespace CZ
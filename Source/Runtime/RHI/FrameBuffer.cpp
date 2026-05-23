#include <Runtime/RHI/FrameBuffer.hpp>

namespace CZ {

DEFINE_HANDLE_DESTROY(FrameBufferObj)

void FrameBufferObj::Clear() {
    for (auto& tex : m_ColorAttachments)
        tex.Destroy();
    m_ColorAttachments.clear();
    m_DepthAttachment.Destroy();
}

} // namespace CZ
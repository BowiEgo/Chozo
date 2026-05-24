#include <Runtime/RHI/FrameBuffer.hpp>

namespace CZ {

template <> void Handle<FrameBufferObj>::Destroy() {
    if (m_Obj) {
        m_Obj->Clear();
        Delete(m_Obj);
        m_Obj = nullptr;
    }
}

void FrameBufferObj::Clear() {
    for (auto& tex : m_ColorAttachments)
        tex.Destroy();
    m_ColorAttachments.clear();
    m_DepthAttachment.Destroy();
}

} // namespace CZ
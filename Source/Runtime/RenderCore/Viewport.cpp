#include <Core/Log/LogMacros.hpp>
#include <Runtime/RHI/RHIAPI.hpp>
#include <Runtime/RenderCore/Viewport.hpp>

namespace CZ {

template <> void Handle<ViewportObj>::Destroy() {
    if (m_Obj) {
        m_Obj->m_FrameBuffer.Destroy();
        Delete(m_Obj);
        m_Obj = nullptr;
    }
}

ViewportObj::ViewportObj(const ViewportSpecification& spec) : m_Spec(spec) {
    CZ_CORE_LOG(Info, "Viewport '{}' created with size {}x{}", spec.Name, spec.Width, spec.Height);

    CreateFrameBuffer();
}

void ViewportObj::CreateFrameBuffer() {
    FrameBufferSpecification fbSpec;
    fbSpec.Name         = m_Spec.Name + "_Framebuffer";
    fbSpec.Size         = { m_Spec.Width, m_Spec.Height };
    fbSpec.ColorFormats = { PixelFormat::RGBA16F };
    fbSpec.DepthFormat  = PixelFormat::D32_SFLOAT;

    m_FrameBuffer = RHIAPI::Get()->CreateFrameBuffer(fbSpec);
}

} // namespace CZ
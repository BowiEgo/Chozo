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
    CZ_RENDERCORE_LOG(Info, "Viewport '{}' created with size {}x{}", spec.Name, spec.Width,
                      spec.Height);

    m_Camera = SceneCamera(
        CZ_NEW(MEMORY_USAGE_SCENE, CameraObj, 45.0f, m_Spec.Width / m_Spec.Height, 0.1f, 1000.0f));

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

void ViewportObj::Resize(uint32 width, uint32 height) {
    if (width == 0 || height == 0) return;
    if (m_Spec.Width == width && m_Spec.Height == height) return;

    auto device = RHIAPI::Get()->GetGraphicsContext()->GetDevice();
    device->WaitIdle();

    m_FrameBuffer.Destroy();
    // auto oldFrameBuffer = m_FrameBuffer;
    // device->EnqueueCleanup([oldFrameBuffer]() mutable {
    //     //
    //     oldFrameBuffer.Reset();
    // });

    m_Spec.Width  = width;
    m_Spec.Height = height;
    m_Camera->SetViewportSize(width, height);
    CreateFrameBuffer();
}

} // namespace CZ
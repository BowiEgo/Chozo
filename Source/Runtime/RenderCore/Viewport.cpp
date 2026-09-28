#include <Core/Log/LogMacros.hpp>
#include <Runtime/RHI/RHIAPI.hpp>
#include <Runtime/RenderCore/Viewport.hpp>

namespace CZ {

ViewportObj::ViewportObj(const ViewportSpecification& spec) : m_Spec(spec) {
    CZ_RENDERCORE_LOG(Info, "Viewport '{}' created with size {}x{}", spec.Name, spec.Width,
                      spec.Height);

    m_Camera = CZ_CREATE_SCOPE(MEMORY_USAGE_SCENE, CameraObj, 45.0f, m_Spec.Width / m_Spec.Height,
                               0.1f, 1000.0f);
    m_Scene  = Scene::Create();

    CreateFrameBuffer();
}

void ViewportObj::CreateFrameBuffer() {
    FrameBufferSpecification fbSpec;
    fbSpec.Name         = m_Spec.Name + "_Framebuffer";
    fbSpec.Size         = { m_Spec.Width, m_Spec.Height };
    fbSpec.ColorFormats = { PixelFormat::RGBA16F };
    fbSpec.DepthFormat  = PixelFormat::D32_SFLOAT;

    m_FrameBuffer = RHIAPI::Get()->CreateFrameBuffer(fbSpec);
    CZ_CORE_ASSERT(m_FrameBuffer, "Failed to create the viewport framebuffer");
}

void ViewportObj::Resize(uint32 width, uint32 height) {
    if (width == 0 || height == 0) return;
    if (m_Spec.Width == width && m_Spec.Height == height) return;

    auto device = RHIAPI::Get()->GetGraphicsContext()->GetDevice();
    device->WaitIdle();

    m_FrameBuffer.reset();
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
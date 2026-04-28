#include "Viewport.h"

#include "RHIAPI.h"
#include "RHICommandList.h"
#include "RHIContext.h"

#include "CameraUniformManager.h"

CViewport::CViewport(const FViewportSpecification& spec) : m_Spec(spec) {
    m_Camera = CreateRef<CSceneCamera>();
    m_Camera->SetViewportSize(spec.Width, spec.Height);

    CreateFrameBuffer();
}

void CViewport::CreateFrameBuffer() {
    FFrameBufferSpecification fbSpec;
    fbSpec.Name         = m_Spec.Name + "_Framebuffer";
    fbSpec.Size         = { m_Spec.Width, m_Spec.Height };
    fbSpec.ColorFormats = { EPixelFormat::RGBA16F };
    fbSpec.DepthFormat  = EPixelFormat::D32_SFLOAT;

    m_FrameBuffer = IRHIAPI::CreateFrameBuffer(fbSpec);
}

void CViewport::Resize(uint32 width, uint32 height) {
    if (width == 0 || height == 0) return;
    if (m_Spec.Width == width && m_Spec.Height == height) return;

    auto device = IRHIAPI::GetContext()->GetDevice();
    device->WaitIdle();

    m_FrameBuffer.Reset();
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
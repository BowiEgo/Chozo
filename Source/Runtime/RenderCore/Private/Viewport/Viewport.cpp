#include "Viewport.h"

#include "RHIAPI.h"
#include "RHICommandList.h"
#include "RHIContext.h"

#include "CameraUniformManager.h"

CViewport::CViewport(IRHIContext* ctx, const FViewportSpecification& spec)
    : m_Context(ctx), m_Spec(spec) {
    m_Camera = CreateRef<CSceneCamera>();
    m_Camera->SetViewportSize(spec.Width, spec.Height);

    CreateFrameBuffer();
}

void CViewport::CreateFrameBuffer() {
    FFrameBufferSpecification fbSpec;
    fbSpec.Name         = m_Spec.Name + "_Framebuffer";
    fbSpec.Size         = { m_Spec.Width, m_Spec.Height };
    fbSpec.ColorFormats = { EPixelFormat::RGBA8_UNORM };
    fbSpec.DepthFormat  = EPixelFormat::D32_SFLOAT;

    m_FrameBuffer = IRHIAPI::CreateFrameBuffer(m_Context, fbSpec);
}

void CViewport::Resize(uint32 width, uint32 height) {
    if (width == 0 || height == 0) return;
    if (m_Spec.Width == width && m_Spec.Height == height) return;

    m_Spec.Width  = width;
    m_Spec.Height = height;
    m_Camera->SetViewportSize(width, height);
    CreateFrameBuffer();
}
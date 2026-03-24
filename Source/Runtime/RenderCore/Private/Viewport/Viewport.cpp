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
    fbSpec.Name = m_Spec.Name + "_Framebuffer";
    fbSpec.Size = { m_Spec.Width, m_Spec.Height };
    fbSpec.ColorFormats = { EPixelFormat::RGBA8_UNORM };
    fbSpec.DepthFormat = EPixelFormat::D32_SFLOAT;

    m_FrameBuffer = IRHIAPI::CreateFrameBuffer(m_Context, fbSpec);
}

void CViewport::BeginRender(IRHICommandList* cmdList, TRef<IRHIPipeline> pipeline) {
    auto uniformBuffer = CCameraUniformManager::Get().GetBufferForCamera(m_Camera.get());
    auto target = m_FrameBuffer->GetColorAttachment(0);
    m_Context->SetTarget(target);

    IRHIAPI::BeginRendering(m_Context, cmdList, true);
    cmdList->BindPipeline(pipeline);
    cmdList->BindUniformBuffer(uniformBuffer, 0, 0);
    cmdList->SetViewport({ 0, 0, (float)m_Spec.Width, (float)m_Spec.Height, 0, 1 });
    cmdList->SetScissor({ 0, 0, m_Spec.Width, m_Spec.Height });

    m_Scene->Draw(m_Context, cmdList);
}

void CViewport::EndRender(IRHICommandList* cmdList) {
    auto target = m_FrameBuffer->GetColorAttachment(0);

    IRHIAPI::EndRendering(m_Context, cmdList);
    IRHIAPI::PrepareTextureForSampling(m_Context, cmdList, target);
}

void CViewport::Resize(uint32 width, uint32 height) {
    if (width == 0 || height == 0) return;
    if (m_Spec.Width == width && m_Spec.Height == height) return;

    m_Spec.Width = width;
    m_Spec.Height = height;
    m_Camera->SetViewportSize(width, height);
    CreateFrameBuffer();
}
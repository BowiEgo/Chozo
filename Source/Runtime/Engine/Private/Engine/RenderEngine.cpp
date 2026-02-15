#include "RenderEngine.h"

DEFINE_LOG_CATEGORY(LogRenderEngine);

CRenderEngine::~CRenderEngine() {}

void CRenderEngine::Init() {
    CZ_LOG(LogRenderEngine, Trace, "Render Engine Initializing...");

    if (m_Window) {
        m_Renderer = CreateScope<CRenderer>(m_Window);
        m_Renderer->Init();
    }

    CZ_LOG(LogRenderEngine, Info, "Render Engine Initialized.");
}

void CRenderEngine::Tick() { m_Renderer->Tick(); }

void CRenderEngine::Shutdown() { m_Renderer->Shutdown(); }

bool CRenderEngine::OnEvent(IEvent& e) {
    if (typeid(e) == typeid(CWindowResizedEvent)) {
        OnWindowResize(static_cast<CWindowResizedEvent&>(e));
    }

    return true;
}

void CRenderEngine::OnWindowResize(CWindowResizedEvent& e) {
    CZ_LOG(LogRenderEngine, Trace, e.ToString());

    FExtent2D frameBufferSize;
    frameBufferSize.Width = e.GetWidth();
    frameBufferSize.Height = e.GetHeight();
    m_Renderer->GetGraphicsContext()->GetRHI()->GetSwapchain()->RecreateSwapchain(frameBufferSize);
}
#include "RenderEngine.h"

#include "RHIContext.h"

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
    if (typeid(e) == typeid(FWindowResizedEvent)) {
        OnWindowResize(static_cast<FWindowResizedEvent&>(e));
    }

    return true;
}

void CRenderEngine::OnWindowResize(FWindowResizedEvent& e) {
    // CZ_LOG(LogRenderEngine, Trace, e.ToString());
    FExtent2D frameBufferSize = m_Window->GetFrameBufferSize();
    m_Renderer->RecreateSwapchain(frameBufferSize);
}
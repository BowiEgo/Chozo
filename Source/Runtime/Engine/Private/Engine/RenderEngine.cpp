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

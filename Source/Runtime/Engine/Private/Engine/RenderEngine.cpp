#include "RenderEngine.h"

DEFINE_LOG_CATEGORY(LogRenderEngine);

CRenderEngine::~CRenderEngine() {}

void CRenderEngine::Init() {
    CZ_LOG(LogRenderEngine, Trace, "Render Engine Initializing...");

    if (m_Window) {
        m_Context = CreateScope<CGraphicsContext>(m_Window);
        m_Context->Init();
    }

    CZ_LOG(LogRenderEngine, Info, "Render Engine Initialized.");
}
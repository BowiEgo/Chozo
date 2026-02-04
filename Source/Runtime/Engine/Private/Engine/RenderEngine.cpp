#include "RenderEngine.h"
#include "ShaderManager.h"

DEFINE_LOG_CATEGORY(LogRenderEngine);

void CRenderEngine::Init() {
    CZ_LOG(LogRenderEngine, Trace, "Render Engine Initializing...");

    // Context creation will be platform-specific; placeholder for now
    if (m_Window) {
        m_Context = CGraphicsContext::Create(m_Window);
        m_Context->Init();
    }

    CShaderManager::Init(m_Context.get());

    CZ_LOG(LogRenderEngine, Info, "Render Engine Initialized.");

    m_Context->CreateRenderer();
}
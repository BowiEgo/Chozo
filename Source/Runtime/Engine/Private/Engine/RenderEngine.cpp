#include "RenderEngine.h"
#include "ShaderManager.h"

namespace Chozo {

DEFINE_LOG_CATEGORY(LogRenderEngine);

void RenderEngine::Init() {
    CZ_LOG(LogRenderEngine, Trace, "Render Engine Initializing...");

    // Context creation will be platform-specific; placeholder for now
    if (m_Window) {
        m_Context = GraphicsContext::Create(m_Window);
        m_Context->Init();
    }

    ShaderManager::Init(m_Context.get());

    CZ_LOG(LogRenderEngine, Trace, "Render Engine Initialized.");

    m_Context->CreateRenderer();
}
} // namespace Chozo
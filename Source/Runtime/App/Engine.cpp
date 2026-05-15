#include "Runtime/RHI/GraphicsContext.h"
#include <Runtime/App/Application.h>
#include <Runtime/App/Engine.h>

#include <Core/Log/LogMacros.h>
#include <Runtime/RHI/RHIAPI.h>

namespace CZ {

DEFINE_LOG_CATEGORY_STATIC(LogEngine, Info);

Engine::~Engine() {}

bool Engine::Init(std::string& err) {
    bool success = false;

    auto window = Application::Get().GetWindow();
    auto fbSize = window.GetFrameBufferSize();

    {
        GraphicsContextSpecification spec;
        spec.FrameBufferSize          = fbSize;
        spec.NativeWindow             = window.GetNativeHandle();
        spec.WindowRequiredExtensions = window.GetRequiredExtensions(err);
#ifdef CZ_DEBUG
        spec.EnableValidationLayers = true;
#else
        spec.EnableValidationLayers = false;
#endif

        m_GraphicsContext = GraphicsContext::Create(spec);

        success = RHIAPI::Get().Init(m_GraphicsContext, err);
    }

    {
        RendererSpecification spec;
        spec.Window = window;

        m_Renderer = Renderer::Create(spec);
    }

    CZ_LOG(LogEngine, Info, "Render Engine Initialized.");
    return success;
}

void Engine::Tick(float deltaTime) { m_Renderer.Tick(deltaTime); }

void Engine::Shutdown() {
    m_Renderer.Shutdown();

    RHIAPI::Get().Shutdown();

    m_GraphicsContext.Destroy();

    // GraphicsContext::Destroy(m_GraphicsContext);
}

bool Engine::OnEvent(Event& e) {
    if (typeid(e) == typeid(WindowResizedEvent)) {
        OnWindowResize(static_cast<WindowResizedEvent&>(e));
    }

    return true;
}

void Engine::OnWindowResize(WindowResizedEvent& e) {
    (void)e;
    // CZ_LOG(LogEngine, Trace, e.ToString());
}
} // namespace CZ

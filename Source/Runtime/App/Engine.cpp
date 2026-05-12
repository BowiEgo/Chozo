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

        success = RHIAPI::Get().Init(spec, err);
    }

    // {
    //     RendererSpecification spec;
    //     spec.Window         = m_Window;
    //     spec.GraphicsContext = m_GraphicsContext;

    //     m_Renderer = Renderer::Create(spec);
    // }

    CZ_LOG(LogEngine, Info, "Render Engine Initialized.");
    return success;
}

void Engine::Tick(float deltaTime) { m_Renderer.Tick(deltaTime); }

void Engine::Shutdown() {
    Renderer::Destroy(m_Renderer);
    RHIAPI::Get().Shutdown();
}

bool Engine::OnEvent(Event& e) {
    if (typeid(e) == typeid(WindowResizedEvent)) {
        OnWindowResize(static_cast<WindowResizedEvent&>(e));
    }

    return true;
}

void Engine::OnWindowResize(WindowResizedEvent& e) {
    // CZ_LOG(LogEngine, Trace, e.ToString());
}
} // namespace CZ

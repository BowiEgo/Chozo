#include <Runtime/App/Application.hpp>
#include <Runtime/App/Engine.hpp>
#include <Runtime/RHI/GraphicsContext.hpp>

#include <Core/Header/RendererAPI.hpp>
#include <Core/JobSystem/JobSystem.h>
#include <Core/Log/LogMacros.hpp>
#include <Runtime/RHI/RHIAPI.hpp>

namespace CZ {

DEFINE_LOG_CATEGORY_STATIC(LogEngine, Info);

Engine::~Engine() {}

bool Engine::Init(std::string& err) {
    bool success = false;

    JobSystemInfo jsInfo;
    jsInfo.ImmediateQueueCapacity = 64;
    jsInfo.StandardQueueCapacity  = 256;
    JobSystem::Init(jsInfo);

    m_ShaderRegistry = CZ_CREATE_SCOPE(MEMORY_USAGE_RUNTIME, ShaderRegistry);
    m_ShaderRegistry->Init();

    m_MeshRegistry = CZ_CREATE_SCOPE(MEMORY_USAGE_RUNTIME, MeshRegistry);
    m_MeshRegistry->Init();

    auto window = Application::Get().GetWindow();
    auto fbSize = window->GetFrameBufferSize();

    {
        GraphicsContextSpecification spec;
        spec.FrameBufferSize          = fbSize;
        spec.NativeWindow             = window->GetNativeHandle();
        spec.WindowRequiredExtensions = window->GetRequiredExtensions(err);
#ifdef CZ_DEBUG
        spec.EnableValidationLayers = true;
#else
        spec.EnableValidationLayers = false;
#endif

        m_GraphicsContext = GraphicsContext::Create(spec);

        success = RHIAPI::Get().Init(m_GraphicsContext, err);
    }

    {
        RendererAPI::SetType(RendererAPI::Type::Vulkan);

        RendererSpecification spec;
        spec.Window = window;

        m_Renderer = Renderer::Create(spec);
    }

    CZ_LOG(LogEngine, Info, "Render Engine Initialized.");
    return success;
}

void Engine::Tick(float deltaTime) { m_Renderer.Tick(deltaTime); }

void Engine::Shutdown() {
    JobSystem::Get().WaitAll();

    GetShaderRegistry()->Clear();

    JobSystem::Shutdown();

    m_Renderer.Shutdown();

    RHIAPI::Shutdown();
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

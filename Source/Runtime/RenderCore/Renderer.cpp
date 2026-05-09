#include <Runtime/RenderCore/Renderer.h>

#include <Core/Log/LogMacros.h>

namespace CZ {

DEFINE_LOG_CATEGORY_STATIC(LogRenderer, Info);

struct RendererObj {
    uint32 CurrentFrameIndex = 0;
    Window* Window;
    GraphicContext GraphicContext;
};

Renderer Renderer::Create(const RendererSpecification& spec) {
    RendererObj* obj = New<RendererObj>(MEMORY_USAGE_RENDER);

    obj->Window         = spec.Window;
    obj->GraphicContext = spec.GraphicContext;

    return { obj };
}

void Renderer::Destroy(Renderer renderer) {
    RendererObj* obj = renderer;

    Delete(obj);
}

void Renderer::Init() { auto fbSize = m_Obj->Window->GetFrameBufferSize(); }

void Renderer::Tick(float deltaTime) {}

} // namespace CZ
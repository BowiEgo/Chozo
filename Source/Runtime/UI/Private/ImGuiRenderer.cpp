#include "ImGuiRenderer.h"

#include "RHIAPI.h"

DEFINE_LOG_CATEGORY(LogImGuiRenderer);

IImGuiRenderer::IImGuiRenderer(CWindow* window, IRHIContext* rhiContext)
    : m_Window(window), m_Context(rhiContext) {
    FTextureSpecification spec;
    spec.Size   = { 1, 1 };
    spec.Format = EPixelFormat::RGBA8_UNORM;
    spec.Usage  = ETextureUsage::Texture;

    m_DefaultBlackTexture = IRHIAPI::CreateTexture(spec);

    FEventBus::Get().AddListener(EEventType::ResourceDestroyed, [this](IEvent& e) {
        auto& resDestroyedEvent = static_cast<FRHIResourceDestroyedEvent&>(e);
        auto* res               = resDestroyedEvent.GetResource();

        if (res->GetResourceType() == EResourceType::Texture) {
            auto tex = static_cast<IRHITexture*>(res);

            auto it = m_TextureIDCache.find(tex);
            if (it != m_TextureIDCache.end()) {
                ReleaseTextureID(it->second);
                m_TextureIDCache.erase(it);
            }
        }
        return true; // Return false to indicate we don't want to mark the event as handled
    });
}

IImGuiRenderer::~IImGuiRenderer() {
    CZ_LOG(LogImGuiRenderer, Trace, "ImGuiRenderer destroying...");
}
#include "GraphicsContext.h"
#include "OpenGLContext.h"
#include "RendererAPI.h"
#include "RendererWindow.h"
#include "VulkanContext.h"

namespace Chozo {

Scope<GraphicsContext> GraphicsContext::Create(IRendererWindow *windowHandle) {
    CZ_CORE_ASSERT(windowHandle, "Window handle is null!");

    switch (RendererAPI::GetAPI()) {
    case RendererAPI::API::None:
        CZ_CORE_ASSERT(false, "RenderAPI::None is currently not supported!");
        return nullptr;

    case RendererAPI::API::Vulkan:
        return CreateScope<VulkanContext>(windowHandle);

    case RendererAPI::API::OpenGL:
        return CreateScope<OpenGLContext>(windowHandle);

    // English comment: No need for default if all enum cases are handled,
    // but keeping it for safety against future API additions.
    default:
        CZ_CORE_ASSERT(false, "Unknown RenderAPI!");
        return nullptr;
    }
}

} // namespace Chozo
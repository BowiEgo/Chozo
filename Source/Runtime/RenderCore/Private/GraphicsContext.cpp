#include "GraphicsContext.h"
#include "OpenGLContext.h"
#include "RendererAPI.h"
#include "RendererWindow.h"
#include "VulkanContext.h"

TScope<CGraphicsContext>
    CGraphicsContext::Create(IRendererWindow *windowHandle) {
    CZ_CORE_ASSERT(windowHandle, "Window handle is null!");

    switch (CRendererAPI::GetType()) {
    case CRendererAPI::EType::None:
        CZ_CORE_ASSERT(false, "RenderAPI::None is currently not supported!");
        return nullptr;

    case CRendererAPI::EType::Vulkan:
        return CreateScope<CVulkanContext>(windowHandle);

    case CRendererAPI::EType::OpenGL:
        return CreateScope<COpenGLContext>(windowHandle);

    // English comment: No need for default if all enum cases are handled,
    // but keeping it for safety against future API additions.
    default:
        CZ_CORE_ASSERT(false, "Unknown RenderAPI!");
        return nullptr;
    }
}

#include <Runtime/RHI/GraphicContext.h>

#include <Core/DynamicLibrary/BackendRegistry.h>
#include <Core/Log/LogMacros.h>

namespace CZ {

DEFINE_LOG_CATEGORY_STATIC(LogGraphicContext, Info);

void GraphicContext::Destroy(GraphicContext ctx) {
    if (!ctx) return;

    auto& registry = BackendRegistry::Get();

    auto destroyFn = registry.GetFunction<void (*)(GraphicContextObj*)>(
        "vulkan", "DestroyVulkanGraphicContextObj");
    if (destroyFn) {
        destroyFn(static_cast<GraphicContextObj*>(ctx.Unwrap()));
    } else {
        CZ_LOG(LogGraphicContext, Error,
               "DestroyVulkanGraphicContextObj not found, memory leak possible.");
    }
}

} // namespace CZ
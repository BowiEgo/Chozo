#include <Runtime/RHI/GraphicsContext.h>

#include "Core/Log/LogMacros.h"
#include "GraphicsContextObj.h"

namespace CZ {

DEFINE_LOG_CATEGORY_STATIC(LogGraphicsContext, Info);

GraphicsContext GraphicsContext::Create(const GraphicsContextSpecification& spec) {
    auto& registry = DynamicLibraryRegistry::Get();
    if (!registry.LoadLib("vulkan_backend", "./libCZVulkan.dylib")) {
        CZ_LOG(LogGraphicsContext, Error, "CreateVulkanGraphicsContextObj not found in backend.");
        return {};
    }

    auto createFn =
        registry.GetFunction<GraphicsContextObj* (*)(const GraphicsContextSpecification& spec)>(
            "vulkan_backend", "CreateVulkanGraphicsContextObj");

    if (!createFn) {
        CZ_LOG(LogGraphicsContext, Error, "CreateVulkanGraphicsContextObj not found in backend.");
        return {};
    }

    auto obj = createFn(spec);

    return { obj };
}

template <> void Handle<GraphicsContextObj>::Destroy() {
    if (m_Obj) {
        CZ_LOG(LogGraphicsContext, Error, "Destroy.");
        m_Obj->GetSwapchain().Destroy();
        m_Obj->GetDevice().Destroy();

        Delete(m_Obj);
        m_Obj = nullptr;
    }
}

// void GraphicsContext::Destroy(GraphicsContext ctx) {
//     CZ_LOG(LogGraphicsContext, Error, "Destroy.");
//     ctx.GetSwapchain().Destroy();
//     // Swapchain::Destroy(ctx.GetSwapchain());
//     ctx.GetDevice().Destroy();

//     auto& registry = DynamicLibraryRegistry::Get();

//     auto destroyFn = registry.GetFunction<void (*)(GraphicsContextObj* obj)>(
//         "vulkan_backend", "DestroyVulkanGraphicsContextObj");
//     if (!destroyFn) {
//         CZ_LOG(LogGraphicsContext, Error, "DestroyVulkanGraphicsContextObj not found in
//         backend.");
//     }

//     if (ctx.m_Obj) {
//         destroyFn(ctx.m_Obj);
//         ctx.m_Obj = nullptr;
//     }

//     return;
// }

uint32 GraphicsContext::GetMaxFramesInFlight() const { return m_Obj->GetMaxFramesInFlight(); }

uint32 GraphicsContext::GetCurrentFrameIndex() const { return m_Obj->GetCurrentFrameIndex(); }

Device GraphicsContext::GetDevice() { return m_Obj->GetDevice(); }

Swapchain GraphicsContext::GetSwapchain() { return m_Obj->GetSwapchain(); }

} // namespace CZ
#include <Runtime/RHI/GraphicsContext.h>

#include "Core/Log/LogMacros.h"
#include "GraphicsContextObj.h"

namespace CZ {

DEFINE_LOG_CATEGORY_STATIC(LogGraphicsContext, Info);

GraphicsContext GraphicsContext::Create(const GraphicsContextSpecification& spec) {
    auto& registry = DynamicLibraryRegistry::Get();
    if (!registry.LoadLib("vulkan_backend", "libCZVulkan.dylib")) {
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
        m_Obj->GetSwapchain().Destroy();
        m_Obj->GetDevice().Destroy();

        Delete(m_Obj);
        m_Obj = nullptr;
    }
}

uint32 GraphicsContext::GetMaxFramesInFlight() const { return m_Obj->GetMaxFramesInFlight(); }

uint32 GraphicsContext::GetCurrentFrameIndex() const { return m_Obj->GetCurrentFrameIndex(); }

Device GraphicsContext::GetDevice() { return m_Obj->GetDevice(); }

Swapchain GraphicsContext::GetSwapchain() { return m_Obj->GetSwapchain(); }

void GraphicsContext::End() { m_Obj->End(); }

} // namespace CZ
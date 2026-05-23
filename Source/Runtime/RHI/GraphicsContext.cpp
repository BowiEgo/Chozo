#include <Core/Log/LogMacros.hpp>
#include <Runtime/RHI/GraphicsContext.hpp>

namespace CZ {

GraphicsContext GraphicsContext::Create(const GraphicsContextSpecification& spec) {
    auto& registry = DynamicLibraryRegistry::Get();
    if (!registry.LoadLib("vulkan_backend", "libCZVulkan.dylib")) {
        CZ_CORE_LOG(Error, "CreateVulkanGraphicsContextObj not found in backend.");
        return {};
    }

    auto createFn =
        registry.GetFunction<GraphicsContextObj* (*)(const GraphicsContextSpecification& spec)>(
            "vulkan_backend", "CreateVulkanGraphicsContextObj");

    if (!createFn) {
        CZ_CORE_LOG(Error, "CreateVulkanGraphicsContextObj not found in backend.");
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

} // namespace CZ
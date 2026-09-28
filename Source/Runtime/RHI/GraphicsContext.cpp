#include <Core/DynamicLibrary/ModuleNames.hpp>
#include <Core/Log/LogMacros.hpp>
#include <Runtime/RHI/GraphicsContext.hpp>

namespace CZ {

Scope<GraphicsContextObj> GraphicsContext::Create(const GraphicsContextSpecification& spec) {
    auto& registry = DynamicLibraryRegistry::Get();
    if (!registry.LoadLib(Modules::GraphicsBackendName, Modules::GraphicsBackendFile)) {
        CZ_RHI_LOG(Error, "CreateVulkanGraphicsContextObj not found in backend.");
        return nullptr;
    }

    auto createFn =
        registry.GetFunction<GraphicsContextObj* (*)(const GraphicsContextSpecification& spec)>(
            "vulkan_backend", "CreateVulkanGraphicsContextObj");

    if (!createFn) {
        CZ_RHI_LOG(Error, "CreateVulkanGraphicsContextObj not found in backend.");
        return nullptr;
    }

    return Scope<GraphicsContextObj>(createFn(spec));
}

} // namespace CZ
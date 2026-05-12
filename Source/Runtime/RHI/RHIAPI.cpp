#include <Runtime/RHI/RHIAPI.h>

#include <Runtime/RHI/GraphicsContext.h>

namespace CZ {

DEFINE_LOG_CATEGORY_STATIC(LogRHIAPI, Info);

RHIAPI& RHIAPI::Get() {
    static RHIAPI Instance;
    return Instance;
}

bool RHIAPI::Init(const GraphicsContextSpecification& gcSpec, std::string& err) {
    auto& registry = DynamicLibraryRegistry::Get();
    if (!registry.LoadLib("Vulkan", "./libCZVulkan.dylib")) {
        err = "Cannot load Vulkan backend.";
        return false;
    }

    m_GraphicsContext = CreateGraphicsContext(gcSpec);

    return true;
}

void RHIAPI::Shutdown() {
    // Swapchain::Destroy(m_Swapchain);
    // Device::Destroy(m_Device);
    // GraphicsContext::Destroy(m_GraphicsContext);
}

GraphicsContext RHIAPI::CreateGraphicsContext(const GraphicsContextSpecification& spec) {
    auto& registry = DynamicLibraryRegistry::Get();

    auto createFn =
        registry.GetFunction<GraphicsContextObj* (*)(const GraphicsContextSpecification&)>(
            "Vulkan", "CreateVulkanGraphicsContextObj");
    if (!createFn) {
        CZ_LOG(LogRHIAPI, Error, "CreateVulkanGraphicsContextObj not found in backend.");
        return {};
    }

    GraphicsContextObj* obj = createFn(spec);
    if (!obj) {
        CZ_LOG(LogRHIAPI, Error, "Backend failed to create GraphicsContextObj.");
        return {};
    }

    CZ_LOG(LogRHIAPI, Info, "GraphicsContext created via backend 'vulkan'.");
    return GraphicsContext(obj);
}

} // namespace CZ
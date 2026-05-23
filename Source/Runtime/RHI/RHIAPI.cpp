#include <Runtime/RHI/RHIAPI.hpp>

#include <Core/DynamicLibrary/DynamicLibraryRegistry.hpp>
#include <Runtime/RHI/GraphicsContext.hpp>

namespace CZ {

DEFINE_LOG_CATEGORY_STATIC(LogRHIAPI, Info);

DEFINE_HANDLE_DESTROY(RHIAPIObj)

RHIAPI& RHIAPI::Get() {
    static RHIAPI instance;

    return instance;
}

bool RHIAPI::Init(GraphicsContext ctx, std::string& err) {
    auto& registry = DynamicLibraryRegistry::Get();
    if (!registry.LoadLib("vulkan_backend", "libCZVulkan.dylib")) {
        err = "Cannot load Vulkan backend.";
        return false;
    }

    auto createFn = registry.GetFunction<RHIAPIObj* (*)(GraphicsContext)>("vulkan_backend",
                                                                          "CreateVulkanAPIObj");
    if (!createFn) {
        CZ_LOG(LogRHIAPI, Error, "CreateVulkanAPIObj not found in backend.");
        return false;
    }

    Get().m_Obj = createFn(ctx);

    return true;
}

void RHIAPI::Shutdown() { Get().Destroy(); }

} // namespace CZ
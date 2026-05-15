#include <Runtime/RHI/RHIAPI.h>

#include <Core/DynamicLibrary/DynamicLibraryRegistry.h>
#include <Runtime/RHI/GraphicsContext.h>

#include "RHIAPIObj.h"

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

    m_Obj = createFn(ctx);

    return true;
}

void RHIAPI::Shutdown() { Destroy(); }

void RHIAPI::WaitIdle() const { GetGraphicsContext().GetDevice().WaitIdle(); }

void RHIAPI::BeginRendering(CommandList cmdList, std::vector<Texture>& targets, bool bClear,
                            uint32_t faceIndex) {
    m_Obj->BeginRendering(cmdList, targets, bClear, faceIndex);
}

void RHIAPI::DrawFrame(const CommandList cmdList, RecordCallback recordCallback) {
    m_Obj->DrawFrame(cmdList, recordCallback);
}

void RHIAPI::EndRendering(CommandList cmdList) { m_Obj->EndRendering(cmdList); }

void RHIAPI::TransitionImageLayout(CommandList cmdList, Image image, const ImageLayout newLayout,
                                   uint32_t baseArrayLayer) {
    m_Obj->TransitionImageLayout(cmdList, image, newLayout, baseArrayLayer);
};

GraphicsContext RHIAPI::GetGraphicsContext() const { return m_Obj->GetGraphicsContext(); }

Sampler RHIAPI::GetSampler(const SamplerSpecification spec) {
    return GetGraphicsContext().GetDevice().GetOrCreateSampler(spec);
}

} // namespace CZ
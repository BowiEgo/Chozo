#include <Runtime/RHI/Image.h>

#include <Core/DynamicLibrary/BackendRegistry.h>
#include <Core/Log/LogMacros.h>

namespace CZ {

DEFINE_LOG_CATEGORY_STATIC(LogImage, Info);

void Image::Destroy(Image image) {
    if (!image) return;

    auto& registry = BackendRegistry::Get();
    auto destroyFn = registry.GetFunction<void (*)(ImageObj*)>("vulkan", "DestroyVulkanImageObj");
    if (destroyFn) {
        destroyFn(static_cast<ImageObj*>(image.Unwrap()));
    } else {
        CZ_LOG(LogImage, Error, "DestroyVulkanImageObj not found, memory leak possible.");
    }
}

} // namespace CZ
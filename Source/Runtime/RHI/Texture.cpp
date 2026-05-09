#include <Runtime/RHI/Texture.h>

#include <Core/DynamicLibrary/BackendRegistry.h>
#include <Core/Log/LogMacros.h>

namespace CZ {

DEFINE_LOG_CATEGORY_STATIC(LogTexture, Info);

void Texture::Destroy(Texture texture) {
    if (!texture) return;

    auto& registry = BackendRegistry::Get();
    auto destroyFn =
        registry.GetFunction<void (*)(TextureObj*)>("vulkan", "DestroyVulkanTextureObj");
    if (destroyFn) {
        destroyFn(static_cast<TextureObj*>(texture.Unwrap()));
    } else {
        CZ_LOG(LogTexture, Error, "DestroyVulkanTextureObj not found, memory leak possible.");
    }
}

} // namespace CZ
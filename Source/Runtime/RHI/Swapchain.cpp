#include <Runtime/RHI/Swapchain.h>

#include <Core/DynamicLibrary/BackendRegistry.h>
#include <Core/Log/LogMacros.h>

namespace CZ {

DEFINE_LOG_CATEGORY_STATIC(LogSwapchain, Info);

void Swapchain::Destroy(Swapchain swapchain) {
    if (!swapchain) return;

    auto& registry = BackendRegistry::Get();
    auto destroyFn =
        registry.GetFunction<void (*)(SwapchainObj*)>("vulkan", "DestroyVulkanSwapchainObj");
    if (destroyFn) {
        destroyFn(static_cast<SwapchainObj*>(swapchain.Unwrap()));
    } else {
        CZ_LOG(LogSwapchain, Error, "DestroyVulkanSwapchainObj not found, memory leak possible.");
    }
}

} // namespace CZ
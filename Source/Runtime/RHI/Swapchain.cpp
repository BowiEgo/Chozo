#include <Runtime/RHI/Swapchain.h>

#include <Core/DynamicLibrary/BackendRegistry.h>
#include <Core/Log/LogMacros.h>

namespace CZ {

DEFINE_LOG_CATEGORY_STATIC(LogSwapchain, Info);

void Swapchain::Destroy(Swapchain swapchain) {
    if (!swapchain) return;

    Delete(swapchain.Unwrap());
}

} // namespace CZ
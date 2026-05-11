#include <Runtime/RHI/Swapchain.h>

namespace CZ {

DEFINE_LOG_CATEGORY_STATIC(LogSwapchain, Info);

void Swapchain::Destroy(Swapchain swapchain) {
    if (!swapchain) return;

    Delete(swapchain.Unwrap());
}

} // namespace CZ
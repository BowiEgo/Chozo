#include <Runtime/RHI/Swapchain.h>

#include "SwapchainObj.h"

namespace CZ {

DEFINE_LOG_CATEGORY_STATIC(LogSwapchain, Info);

void Swapchain::Destroy(Swapchain swapchain) {
    if (!swapchain) return;

    Delete(swapchain.Unwrap());
}

PixelFormat Swapchain::GetImageFormat() const { return m_Obj->GetImageFormat(); }

PixelFormat Swapchain::GetDepthFormat() const { return m_Obj->GetDepthFormat(); }

uint32_t Swapchain::GetImageCount() const { return m_Obj->GetImageCount(); }

} // namespace CZ
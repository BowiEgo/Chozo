#include <Runtime/RHI/Swapchain.hpp>

#include "SwapchainObj.hpp"
#include <Runtime/RHI/RHIAPI.hpp>

namespace CZ {

DEFINE_LOG_CATEGORY_STATIC(LogSwapchain, Info);

// DEFINE_HANDLE_BACKEND_DESTROY(SwapchainObj, "vulkan_backend", "DestroyVulkanSwapchainObj")

// DEFINE_HANDLE_DESTROY(TextureObj)

template <> void Handle<SwapchainObj>::Destroy() {
    if (m_Obj) {
        m_Obj->Destroy();
        Delete(m_Obj);
        m_Obj = nullptr;
    }
}

PixelFormat Swapchain::GetImageFormat() const { return m_Obj->GetImageFormat(); }

PixelFormat Swapchain::GetDepthFormat() const { return m_Obj->GetDepthFormat(); }

uint32_t Swapchain::GetImageCount() const { return m_Obj->GetImageCount(); }

Fence Swapchain::GetFence(uint32 currentFrame) const { return m_Obj->GetFence(currentFrame); }

Semaphore Swapchain::GetImageAvailableSemaphore(uint32 currentFrame) const {
    return m_Obj->GetImageAvailableSemaphore(currentFrame);
}

Texture Swapchain::GetColorAttachment(uint32 index) { return m_Obj->GetColorAttachment(index); }

} // namespace CZ
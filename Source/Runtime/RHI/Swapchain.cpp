#include <Runtime/RHI/Swapchain.hpp>

#include <Runtime/RHI/RHIAPI.hpp>

namespace CZ {

DEFINE_LOG_CATEGORY_STATIC(LogSwapchain, Info);

template <> void Handle<SwapchainObj>::Destroy() {
    if (m_Obj) {
        m_Obj->Destroy();
        Delete(m_Obj);
        m_Obj = nullptr;
    }
}

void SwapchainObj::Destroy() {
    for (auto& tex : m_ColorAttachments) {
        tex.Destroy();
    }
    m_ColorAttachments.clear();

    for (auto& sem : m_ImageAvailableSemaphores) {
        sem.Destroy();
    }
    m_ImageAvailableSemaphores.clear();

    for (auto& sem : m_RenderFinishedSemaphores) {
        sem.Destroy();
    }
    m_RenderFinishedSemaphores.clear();

    for (auto& fence : m_InFlightFences) {
        fence.Destroy();
    }
    m_InFlightFences.clear();
}

} // namespace CZ
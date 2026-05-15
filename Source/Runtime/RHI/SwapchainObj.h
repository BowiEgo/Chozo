#include "Core/Log/LogMacros.h"
#include <Runtime/RHI/Fence.h>
#include <Runtime/RHI/Semaphore.h>
#include <Runtime/RHI/Swapchain.h>
#include <Runtime/RHI/Texture.h>

namespace CZ {

class SwapchainObj {

public:
    SwapchainObj(const SwapchainSpecification& spec) : m_Spec(spec) {}
    virtual ~SwapchainObj() = default;

    void Destroy() {
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

    virtual PixelFormat GetImageFormat() const = 0;

    virtual PixelFormat GetDepthFormat() const = 0;

    virtual const Extent2D GetExtent() const = 0;

    virtual uint32_t GetImageCount() const = 0;

    virtual void SetPresentMode(const PresentMode mode) = 0;

    virtual void Recreate(const Extent2D& frameBufferSize) = 0;

    Fence GetFence(uint32 currentFrameIdx) const { return m_InFlightFences[currentFrameIdx]; }

    Semaphore GetImageAvailableSemaphore(uint32 currentFrameIdx) const {
        return m_ImageAvailableSemaphores[currentFrameIdx];
    }

    Semaphore GetRenderFinishedSemaphore(uint32 imageIdx) const {
        return m_RenderFinishedSemaphores[imageIdx];
    }

    Texture GetColorAttachment(uint32 index) { return m_ColorAttachments[index]; }

    void SetCurrentImageIndex(uint32 imageIndex) { m_CurrentImageIndex = imageIndex; }

protected:
    SwapchainSpecification m_Spec;

    bool m_NeedsRecreation = false;

    PresentMode m_PresentMode = PresentMode::FIFO;
    std::vector<Texture> m_ColorAttachments;
    uint32_t m_ImageCount      = 0;
    uint32 m_CurrentImageIndex = 0;

    std::vector<Fence> m_InFlightFences;
    std::vector<Semaphore> m_ImageAvailableSemaphores;
    std::vector<Semaphore> m_RenderFinishedSemaphores;
};

} // namespace CZ
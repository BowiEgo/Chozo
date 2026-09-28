#pragma once
#include <cstdint>

#include <Core/Header/Extent.hpp>
#include <Core/Header/Handle.hpp>
#include <Core/Memory/Memory.hpp>
#include <Runtime/RHI/Fence.hpp>
#include <Runtime/RHI/RHITypes.hpp>
#include <Runtime/RHI/Semaphore.hpp>
#include <Runtime/RHI/Texture.hpp>

namespace CZ {

struct SwapchainSpecification {
    std::string Name;
    Extent2D FrameBufferSize;
    void* NativeWindow = nullptr;

    uint32 MaxFramesInFlight;
};

class SwapchainObj {
public:
    SwapchainObj(const SwapchainSpecification& spec) : m_Spec(spec) {}
    virtual ~SwapchainObj() = default;

    virtual PixelFormat GetImageFormat() const = 0;

    virtual PixelFormat GetDepthFormat() const = 0;

    virtual const Extent2D GetExtent() const = 0;

    virtual uint32_t GetImageCount() const = 0;

    virtual void SetPresentMode(const PresentMode mode) = 0;

    virtual void Recreate(const Extent2D& frameBufferSize) = 0;

    Fence GetFence(uint32 currentFrameIdx) const {
        return ViewAs<Fence>(m_InFlightFences[currentFrameIdx]);
    }

    Semaphore GetImageAvailableSemaphore(uint32 currentFrameIdx) const {
        return ViewAs<Semaphore>(m_ImageAvailableSemaphores[currentFrameIdx]);
    }

    Semaphore GetRenderFinishedSemaphore(uint32 imageIdx) const {
        return ViewAs<Semaphore>(m_RenderFinishedSemaphores[imageIdx]);
    }

    Texture GetColorAttachment(uint32 index) { return ViewAs<Texture>(m_ColorAttachments[index]); }

    void SetCurrentImageIndex(uint32 imageIndex) { m_CurrentImageIndex = imageIndex; }

protected:
    SwapchainSpecification m_Spec;

    bool m_NeedsRecreation = false;

    PresentMode m_PresentMode = PresentMode::FIFO;

    // Owned by the swapchain; handed out as views.
    std::vector<Scope<TextureObj>> m_ColorAttachments;
    uint32_t m_ImageCount      = 0;
    uint32 m_CurrentImageIndex = 0;

    std::vector<Scope<FenceObj>> m_InFlightFences;
    std::vector<Scope<SemaphoreObj>> m_ImageAvailableSemaphores;
    std::vector<Scope<SemaphoreObj>> m_RenderFinishedSemaphores;
};

struct Swapchain : Handle<class SwapchainObj> {

    template <typename T> T* As() { return static_cast<T*>(InternalHandleReader::Unwrap(*this)); }
};

} // namespace CZ

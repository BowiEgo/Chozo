#include <Runtime/RHI/Swapchain.h>

namespace CZ {

class SwapchainObj {
public:
    SwapchainObj(const Device device, const SwapchainSpecification& spec)
        : m_Device(device), m_Spec(spec) {}
    virtual ~SwapchainObj() {
        for (auto& tex : m_ColorAttachments) {
            Texture::Destroy(tex);
        }
        m_ColorAttachments.clear();
    }

    virtual PixelFormat GetImageFormat() const = 0;

    virtual PixelFormat GetDepthFormat() const = 0;

    virtual const Extent2D GetExtent() const = 0;

    virtual uint32_t GetImageCount() const = 0;

    virtual void SetPresentMode(const PresentMode mode) = 0;

    virtual void Recreate(const Extent2D& frameBufferSize) = 0;

protected:
    Device m_Device;
    SwapchainSpecification m_Spec;

    PresentMode m_PresentMode = PresentMode::FIFO;
    std::vector<Texture> m_ColorAttachments;
};

} // namespace CZ
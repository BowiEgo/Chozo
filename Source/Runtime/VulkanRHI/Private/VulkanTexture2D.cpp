#include "VulkanTexture2D.h"

#include "VulkanImage.h"
#include "VulkanUtils.h"

DEFINE_LOG_CATEGORY(LogVulkanTexture2D);

CVulkanTexture2D::CVulkanTexture2D(const WeakRef<IRHIDevice> device,
                                   const FTextureSpecification& spec)
    : IRHITexture2D(device, spec) {}

CVulkanTexture2D::CVulkanTexture2D(const WeakRef<IRHIDevice> device,
                                   const FTextureSpecification& spec, const TRef<IRHIImage> image)
    : IRHITexture2D(device, spec, image) {}

CVulkanTexture2D::CVulkanTexture2D(const WeakRef<IRHIDevice> device,
                                   const FTextureSpecification& spec, FBuffer& data)
    : IRHITexture2D(device, spec) {
    auto image = GetImage();
    image->SetData(data);
}

CVulkanTexture2D::~CVulkanTexture2D() {
    auto device = m_Device.lock().As<CVulkanDevice>();
    if (!device) return;

    auto image = m_Image;
    device->EnqueueCleanup([=] {
        if (image) image.As<CVulkanImage>()->Destroy();
    });
}

vk::RenderingAttachmentInfo
    CVulkanTexture2D::GetColorAttachmentInfo(const vk::ClearValue clearColor, const bool bClear) {
    vk::ImageView imageView = GetImage().As<CVulkanImage>()->GetVKView();

    return vk::RenderingAttachmentInfo()
        .setImageView(imageView)
        .setImageLayout(vk::ImageLayout::eColorAttachmentOptimal)
        .setLoadOp(bClear ? vk::AttachmentLoadOp::eClear : vk::AttachmentLoadOp::eLoad)
        .setStoreOp(vk::AttachmentStoreOp::eStore)
        .setClearValue(clearColor);
}

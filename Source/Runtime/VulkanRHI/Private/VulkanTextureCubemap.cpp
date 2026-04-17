#include "VulkanTextureCubemap.h"

#include "VulkanImage.h"
#include "VulkanUtils.h"

DEFINE_LOG_CATEGORY(LogVulkanTextureCubemap);

CVulkanTextureCubemap::~CVulkanTextureCubemap() {
    // // CZ_LOG(LogVulkanTextureCubemap, Trace, "VulkanTextureCubemap: {} destroying...",
    // // m_Spec.Name);

    auto device = m_Device.lock().As<CVulkanDevice>();
    if (!device) return;

    IRHIImage* image = m_OwnedImage.release();
    if (image) {
        device->EnqueueCleanup([image]() { static_cast<CVulkanImage*>(image)->Destroy(); });
    }
}

vk::RenderingAttachmentInfo
    CVulkanTextureCubemap::GetColorAttachmentInfo(const vk::ClearValue clearColor,
                                                  const bool bClear, uint32_t face) {
    CZ_ASSERT(face < 6);

    IRHIImage* image = GetImage();

    FImageViewSpecification faceViewSpec;
    faceViewSpec.ViewType       = EImageViewType::View2D;
    faceViewSpec.BaseArrayLayer = face;
    faceViewSpec.LayerCount     = 1;
    faceViewSpec.BaseMipLevel   = 0;
    faceViewSpec.MipCount       = 1;
    vk::ImageView imageView     = static_cast<CVulkanImage*>(image)->GetVKView(faceViewSpec);

    return vk::RenderingAttachmentInfo()
        .setImageView(imageView)
        .setImageLayout(vk::ImageLayout::eColorAttachmentOptimal)
        .setLoadOp(bClear ? vk::AttachmentLoadOp::eClear : vk::AttachmentLoadOp::eLoad)
        .setStoreOp(vk::AttachmentStoreOp::eStore)
        .setClearValue(clearColor);
}

#include "VulkanTexture2D.h"

#include "VulkanImage.h"
#include "VulkanUtils.h"

DEFINE_LOG_CATEGORY(LogVulkanTexture2D);

CVulkanTexture2D::~CVulkanTexture2D() {
    // CZ_LOG(LogVulkanTexture2D, Trace, "VulkanTexture2D: {} destroying...", m_Spec.Name);

    auto device = m_Device.lock().As<CVulkanDevice>();
    if (!device) return;

    IRHIImage* image = m_OwnedImage.release();
    if (image) {
        device->EnqueueCleanup([image]() { static_cast<CVulkanImage*>(image)->Destroy(); });
    }
}

vk::RenderingAttachmentInfo
    CVulkanTexture2D::GetColorAttachmentInfo(const vk::ClearValue clearColor, const bool bClear,
                                             uint32_t face) {
    vk::ImageView imageView = static_cast<CVulkanImage*>(GetImage())->GetVKView();

    return vk::RenderingAttachmentInfo()
        .setImageView(imageView)
        .setImageLayout(vk::ImageLayout::eColorAttachmentOptimal)
        .setLoadOp(bClear ? vk::AttachmentLoadOp::eClear : vk::AttachmentLoadOp::eLoad)
        .setStoreOp(vk::AttachmentStoreOp::eStore)
        .setClearValue(clearColor);
}

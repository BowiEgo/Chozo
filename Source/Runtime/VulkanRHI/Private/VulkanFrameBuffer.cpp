#include "VulkanFrameBuffer.h"

#include "VulkanTexture2D.h"

DEFINE_LOG_CATEGORY(LogVulkanFrameBuffer);

CVulkanFrameBuffer::CVulkanFrameBuffer(const FFrameBufferSpecification& spec,
                                       const TRef<CVulkanDevice>& device)
    : IRHIFrameBuffer(spec), m_Device(device) {
    m_ColorAttachments.reserve(spec.ColorFormats.size());
    for (const auto& format : spec.ColorFormats) {
        FTextureSpecification texSpec;
        texSpec.Name = spec.Name + "_ColorAttachment_" + std::to_string(m_ColorAttachments.size());
        texSpec.Size = spec.Size;
        texSpec.Format = format;
        texSpec.Usage  = ETextureUsage::Attachment;

        m_ColorAttachments.push_back(
            CreateRef<CVulkanTexture2D>(WeakRef<IRHIDevice>(device), texSpec));
    }

    if (spec.DepthFormat != EPixelFormat::Unknown) {
        FTextureSpecification depthSpec;
        depthSpec.Name   = spec.Name + "_DepthAttachment";
        depthSpec.Size   = spec.Size;
        depthSpec.Format = spec.DepthFormat;
        depthSpec.Usage  = ETextureUsage::Attachment;

        m_DepthAttachment = CreateRef<CVulkanTexture2D>(WeakRef<IRHIDevice>(device), depthSpec);
    }
}

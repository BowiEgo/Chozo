#include "VulkanFrameBuffer.h"

#include "VulkanTexture2D.h"

CVulkanFrameBuffer::CVulkanFrameBuffer(const FFrameBufferSpecification& spec,
                                       const TRef<CVulkanDevice>& device)
    : IRHIFrameBuffer(spec), m_Device(device) {
    m_ColorAttachments.reserve(spec.ColorFormats.size());
    for (const auto& format : spec.ColorFormats) {
        FTexture2DSpecification texSpec;
        texSpec.Name = spec.Name + "_ColorAttachment_" + std::to_string(m_ColorAttachments.size());
        texSpec.Size = spec.Size;
        texSpec.Format = format;
        texSpec.Usage = ETextureUsage::ColorAttachment;

        m_ColorAttachments.push_back(CreateRef<CVulkanTexture2D>(texSpec, m_Device));
    }

    if (spec.DepthFormat != EPixelFormat::Unknown) {
        FTexture2DSpecification depthSpec;
        depthSpec.Name = spec.Name + "_DepthAttachment";
        depthSpec.Size = spec.Size;
        depthSpec.Format = spec.DepthFormat;
        depthSpec.Usage = ETextureUsage::DepthAttachment;

        m_DepthAttachment = CreateRef<CVulkanTexture2D>(depthSpec, m_Device);
    }
}

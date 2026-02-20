#include "VulkanRHIFrameBuffer.h"

#include "VulkanRHITexture2D.h"

CVulkanRHIFrameBuffer::CVulkanRHIFrameBuffer(const FFrameBufferSpecification& spec,
                                             const TRef<CVulkanRHIDevice>& device)
    : IRHIFrameBuffer(spec), m_Device(device) {
    m_ColorAttachments.reserve(spec.ColorFormats.size());
    for (const auto& format : spec.ColorFormats) {
        FTextureSpecification texSpec;
        texSpec.Name = spec.Name + "_ColorAttachment_" + std::to_string(m_ColorAttachments.size());
        texSpec.Size = spec.Size;
        texSpec.Format = format;
        texSpec.Usage = ETextureUsage::ColorAttachment;

        m_ColorAttachments.push_back(CreateRef<CVulkanRHITexture2D>(texSpec, m_Device));
    }

    if (spec.DepthFormat != EPixelFormat::Unknown) {
        FTextureSpecification depthSpec;
        depthSpec.Name = spec.Name + "_DepthAttachment";
        depthSpec.Size = spec.Size;
        depthSpec.Format = spec.DepthFormat;
        depthSpec.Usage = ETextureUsage::DepthAttachment;

        m_DepthAttachment = CreateRef<CVulkanRHITexture2D>(depthSpec, m_Device);
    }
}

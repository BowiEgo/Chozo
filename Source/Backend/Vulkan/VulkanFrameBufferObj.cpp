#include "VulkanFrameBufferObj.hpp"
#include "Core/Header/Extent.hpp"
#include "VulkanTextureObj.hpp"

namespace CZ {

DEFINE_LOG_CATEGORY_STATIC(LogVulkanFrameBuffer, Info);

VulkanFrameBufferObj::VulkanFrameBufferObj(const VulkanDeviceObj* deviceObj,
                                           const FrameBufferSpecification& spec)
    : FrameBufferObj(spec), m_DeviceObj((deviceObj)) {
    Init();
}

VulkanFrameBufferObj::~VulkanFrameBufferObj() {}

void VulkanFrameBufferObj::Resize(const Extent2D& size) {
    Clear();
    m_Spec.Size = size;
    Init();
}

void VulkanFrameBufferObj::Init() {
    m_ColorAttachments.reserve(m_Spec.ColorFormats.size());

    for (const auto& format : m_Spec.ColorFormats) {
        TextureSpecification texSpec;
        texSpec.Name =
            m_Spec.Name + "_ColorAttachment_" + std::to_string(m_ColorAttachments.size());
        texSpec.Size   = m_Spec.Size;
        texSpec.Format = format;
        texSpec.Usage  = TextureUsage::Attachment;

        m_ColorAttachments.push_back(
            Texture(CZ_NEW(MEMORY_USAGE_RENDER, VulkanTextureObj, m_DeviceObj, texSpec)));
    }

    if (m_Spec.DepthFormat != PixelFormat::Unknown) {
        TextureSpecification texSpec;
        texSpec.Name   = m_Spec.Name + "_DepthAttachment";
        texSpec.Size   = m_Spec.Size;
        texSpec.Format = m_Spec.DepthFormat;
        texSpec.Usage  = TextureUsage::Attachment;

        m_DepthAttachment =
            Texture(CZ_NEW(MEMORY_USAGE_RENDER, VulkanTextureObj, m_DeviceObj, texSpec));
    }
}

} // namespace CZ
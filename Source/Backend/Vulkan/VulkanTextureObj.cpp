#include "VulkanTextureObj.h"
#include "Core/Log/LogMacros.h"
#include "Runtime/RHI/RHITypes.h"
#include "VulkanImageObj.h"

namespace CZ {

DEFINE_LOG_CATEGORY_STATIC(LogVulkanTexture, Info);

VulkanTextureObj::VulkanTextureObj(const VulkanDeviceObj* deviceObj,
                                   const TextureSpecification& spec)
    : TextureObj(spec), m_DeviceObj(deviceObj) {}

VulkanTextureObj::VulkanTextureObj(const VulkanDeviceObj* deviceObj,
                                   const TextureSpecification& spec, Image image)
    : TextureObj(spec, image), m_DeviceObj(deviceObj) {}

VulkanTextureObj::~VulkanTextureObj() {}

VkRenderingAttachmentInfo VulkanTextureObj::GetColorAttachmentInfo(const VkClearValue clearColor,
                                                                   const bool bClear,
                                                                   uint32_t face) {
    auto imageObj = GetImage().As<VulkanImageObj>();

    ImageViewSpecification viewSpec;

    if (GetType() == TextureType::TextureCube) {
        viewSpec.ViewType       = ImageViewType::View2D;
        viewSpec.BaseArrayLayer = face;
        viewSpec.LayerCount     = 1;
        viewSpec.BaseMipLevel   = 0;
        viewSpec.MipCount       = 1;
    }

    VkImageView imageView = imageObj->GetOrCreateVKView(viewSpec);

    VkRenderingAttachmentInfo info{};

    info.sType       = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
    info.imageView   = imageView;
    info.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    info.loadOp      = bClear ? VK_ATTACHMENT_LOAD_OP_CLEAR : VK_ATTACHMENT_LOAD_OP_LOAD;
    info.storeOp     = VK_ATTACHMENT_STORE_OP_STORE;
    info.clearValue  = clearColor;

    return info;
}

} // namespace CZ
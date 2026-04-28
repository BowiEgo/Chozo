#include "VulkanTexture.h"

#include "VulkanImage.h"
#include "VulkanSampler.h"

CVulkanTexture::CVulkanTexture(const WeakRef<IRHIDevice> device, const FTextureSpecification& spec)
    : IRHITexture(device, spec) {}

CVulkanTexture::CVulkanTexture(const WeakRef<IRHIDevice> device, const FTextureSpecification& spec,
                               TScope<IRHIImage> ownedImage)
    : IRHITexture(device, spec, std::move(ownedImage)) {}

CVulkanTexture::CVulkanTexture(const WeakRef<IRHIDevice> device, const FTextureSpecification& spec,
                               IRHIImage* borrowedImage)
    : IRHITexture(device, spec, borrowedImage) {}

CVulkanTexture::CVulkanTexture(const WeakRef<IRHIDevice> device, const FTextureSpecification& spec,
                               FBuffer& data)
    : IRHITexture(device, spec, data) {}

CVulkanTexture::~CVulkanTexture() {}

vk::DescriptorImageInfo CVulkanTexture::GetImageInfo() {
    vk::DescriptorImageInfo imageInfo{};
    imageInfo.setSampler(GetSampler().As<CVulkanSampler>()->GetVKHandle());
    imageInfo.setImageView(static_cast<CVulkanImage*>(GetImage())->GetVKView());
    imageInfo.setImageLayout(vk::ImageLayout::eShaderReadOnlyOptimal);

    return imageInfo;
}

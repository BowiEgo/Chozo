#include "RHITexture.h"

#include "RHIDevice.h"

DEFINE_LOG_CATEGORY(LogRHITexture);

IRHITexture::IRHITexture(const WeakRef<IRHIDevice> device, const FTextureSpecification& spec)
    : IRHIResource(device), m_Spec(spec), m_Image(device.lock()->CreateImage(spec.ToImageSpec())) {}

IRHITexture::IRHITexture(const WeakRef<IRHIDevice> device, const FTextureSpecification& spec,
                         const TRef<IRHIImage> image)
    : IRHIResource(device), m_Spec(spec), m_Image(image) {}

IRHITexture::~IRHITexture() {
    // CZ_LOG(LogRHITexture, Trace, "RHITexture: {} destroying...", m_Spec.Name);
}

TRef<IRHISampler> IRHITexture::GetSampler(const FSamplerSpecification spec) const {
    return m_Device.lock()->GetSampler(spec);
};

void* IRHITexture::GetDescriptorSet(TRef<IRHISetLayout> setLayout, uint32_t bindingSlot) const {
    auto device = m_Device.lock();
    if (!setLayout) setLayout = device->GetStaticSetLayout();

    FTextureDescriptorInfo info;
    info.Image       = GetImage();
    info.Sampler     = GetSampler();
    info.ImageLayout = EImageLayout::ShaderReadOnlyOptimal;

    return device->GetOrCreateDescriptorSet(info, setLayout, 0)->GetRawHandle();
}

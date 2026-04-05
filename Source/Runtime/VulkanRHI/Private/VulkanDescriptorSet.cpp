#include "VulkanDescriptorSet.h"

#include "VulkanDevice.h"
#include "VulkanImage.h"
#include "VulkanSampler.h"
#include "VulkanUtils.h"

CVulkanDescriptorSet::CVulkanDescriptorSet(const WeakRef<IRHIDevice> device,
                                           const FTextureDescriptorInfo& info,
                                           TRef<IRHISetLayout> setLayout, uint32 bindingSlot)
    : IRHIDescriptorSet(device), m_Info(info), m_Layout(setLayout), m_Slot(bindingSlot) {
    Init();
}

void CVulkanDescriptorSet::Init() {
    auto device = m_Device.lock().As<CVulkanDevice>();

    vk::DescriptorSetLayout vkLayout = m_Layout->GetVKSetLayout();
    vk::raii::DescriptorSet vkNewSet = device->AllocateSetFromPool(vkLayout);

    vk::DescriptorImageInfo imageInfo;
    imageInfo.setSampler(m_Info.Sampler.As<CVulkanSampler>()->GetVKSampler())
        .setImageView(m_Info.Image.As<CVulkanImage>()->GetVKView())
        .setImageLayout(ChozoUtils::Vulkan::ToVkImageLayout(m_Info.ImageLayout));

    vk::WriteDescriptorSet write{};
    write.setDstSet(*vkNewSet)
        .setDstBinding(m_Slot)
        .setDescriptorCount(1)
        .setDescriptorType(vk::DescriptorType::eCombinedImageSampler)
        .setPImageInfo(&imageInfo);

    device->GetRAIILogicalDevice().updateDescriptorSets(write, nullptr);

    m_RAIIHandle = std::move(vkNewSet);
}

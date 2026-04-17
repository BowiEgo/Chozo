#include "VulkanDescriptorSet.h"

#include "VulkanBuffer.h"
#include "VulkanDevice.h"
#include "VulkanSampler.h"
#include "VulkanSetLayout.h"
#include "VulkanTexture.h"
#include "VulkanUtils.h"

CVulkanDescriptorSet::CVulkanDescriptorSet(const WeakRef<IRHIDevice> device,
                                           TRef<IRHISetLayout> setLayout,
                                           const std::vector<FDescriptorBinding>& bindings)
    : IRHIDescriptorSet(device, setLayout, bindings) {
    Init();
}

void CVulkanDescriptorSet::Init() {
    auto device = m_Device.lock().As<CVulkanDevice>();

    vk::DescriptorSetLayout vkLayout  = m_SetLayout.As<CVulkanSetLayout>()->GetVKHandle();
    vk::raii::DescriptorSet vkDescSet = device->AllocateSetFromPool(vkLayout);

    // vk::DescriptorImageInfo imageInfo;
    // imageInfo.setSampler(m_Info.Sampler.As<CVulkanSampler>()->GetVKSampler())
    //     .setImageView(static_cast<CVulkanImage*>(m_Info.Image)->GetVKView())
    //     .setImageLayout(ChozoUtils::Vulkan::ToVkImageLayout(m_Info.ImageLayout));

    // vk::WriteDescriptorSet write{};
    // write.setDstSet(*vkDescSet)
    //     .setDstBinding(m_Slot)
    //     .setDescriptorCount(1)
    //     .setDescriptorType(vk::DescriptorType::eCombinedImageSampler)
    //     .setPImageInfo(&imageInfo);

    // device->GetRAIILogicalDevice().updateDescriptorSets(write, nullptr);

    // m_RAIIHandle = std::move(vkDescSet);

    std::vector<vk::WriteDescriptorSet> writes;
    for (const auto& b : m_ResourceBindings) {
        vk::WriteDescriptorSet write;
        write.setDstSet(vkDescSet).setDstBinding(b.Binding).setDescriptorCount(1);

        if (b.Type == EUniformType::UniformBuffer) {
            auto buffer = static_cast<CVulkanBuffer*>(b.Resource);

            vk::DescriptorBufferInfo bufferInfo(buffer->GetVKBuffer(), 0, buffer->GetSize());

            write.setDescriptorType(vk::DescriptorType::eUniformBuffer).setPBufferInfo(&bufferInfo);
        } else if (b.Type == EUniformType::CombinedImageSampler) {
            auto texture = static_cast<CVulkanTexture*>(b.Resource);
            auto sampler = static_cast<CVulkanSampler*>(b.Sampler);

            vk::DescriptorImageInfo imageInfo(sampler->GetVKHandle(), texture->GetVKImageView(),
                                              ChozoUtils::Vulkan::ToVkImageLayout(b.ImageLayout));

            write.setDescriptorType(vk::DescriptorType::eCombinedImageSampler)
                .setPImageInfo(&imageInfo);
        }
        writes.push_back(write);
    }

    device->GetRAIILogicalDevice().updateDescriptorSets(writes, nullptr);

    m_RAIIHandle = std::move(vkDescSet);
}

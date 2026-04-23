#include "VulkanDescriptorSet.h"

#include "VulkanBuffer.h"
#include "VulkanDevice.h"
#include "VulkanImage.h"
#include "VulkanSampler.h"
#include "VulkanSetLayout.h"
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

    std::vector<vk::WriteDescriptorSet> writes;
    for (const auto& b : m_ResourceBindings) {
        vk::WriteDescriptorSet write;
        write.setDstSet(vkDescSet).setDstBinding(b.Binding).setDescriptorCount(1);

        if (b.Type == EUniformType::UniformBuffer) {
            auto buffer = static_cast<CVulkanBuffer*>(b.Resource);

            vk::DescriptorBufferInfo bufferInfo(buffer->GetVKBuffer(), 0, buffer->GetSize());

            write.setDescriptorType(vk::DescriptorType::eUniformBuffer).setPBufferInfo(&bufferInfo);
        } else if (b.Type == EUniformType::CombinedImageSampler) {
            auto image   = static_cast<CVulkanImage*>(b.Resource);
            auto sampler = static_cast<CVulkanSampler*>(b.Sampler);

            vk::DescriptorImageInfo imageInfo(sampler->GetVKHandle(),
                                              image->GetVKView(image->GetSpec().ToImageViewSpec()),
                                              ChozoUtils::Vulkan::ToVkImageLayout(b.ImageLayout));

            write.setDescriptorType(vk::DescriptorType::eCombinedImageSampler)
                .setPImageInfo(&imageInfo);
        }
        writes.push_back(write);
    }

    device->GetRAIILogicalDevice().updateDescriptorSets(writes, nullptr);

    m_RAIIHandle = std::move(vkDescSet);
}

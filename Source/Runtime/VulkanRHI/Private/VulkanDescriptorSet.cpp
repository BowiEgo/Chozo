#include "VulkanDescriptorSet.h"

#include "VulkanBuffer.h"
#include "VulkanDevice.h"
#include "VulkanImage.h"
#include "VulkanSampler.h"
#include "VulkanSetLayout.h"
#include "VulkanUtils.h"

DEFINE_LOG_CATEGORY(LogVulkanDescriptorSet);

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

    std::vector<vk::DescriptorBufferInfo> bufferInfos;
    std::vector<vk::DescriptorImageInfo> imageInfos;
    bufferInfos.reserve(m_ResourceBindings.size());
    imageInfos.reserve(m_ResourceBindings.size());

    for (const auto& b : m_ResourceBindings) {
        vk::WriteDescriptorSet write{};
        write.setDstSet(vkDescSet).setDstBinding(b.Binding).setDescriptorCount(1);

        if (b.Type == EUniformType::UniformBuffer) {
            auto buffer = static_cast<CVulkanBuffer*>(b.Resource);

            bufferInfos.emplace_back(buffer->GetVKBuffer(), 0, buffer->GetSize());

            write.setDescriptorType(vk::DescriptorType::eUniformBuffer)
                .setPBufferInfo(&bufferInfos.back());
        } else if (b.Type == EUniformType::CombinedImageSampler) {
            auto image   = static_cast<CVulkanImage*>(b.Resource);
            auto sampler = static_cast<CVulkanSampler*>(b.Sampler);

            auto vkImageView = image->GetVKView(image->GetSpec().ToImageViewSpec());
            auto vkSampler   = sampler->GetVKHandle();

            imageInfos.emplace_back(vkSampler, vkImageView,
                                    ChozoUtils::Vulkan::ToVkImageLayout(b.ImageLayout));

            write.setDescriptorType(vk::DescriptorType::eCombinedImageSampler)
                .setPImageInfo(&imageInfos.back());
        }
        writes.push_back(write);
    }

    device->GetRAIILogicalDevice().updateDescriptorSets(writes, nullptr);

    m_RAIIHandle = std::move(vkDescSet);
}

#include "VulkanSetLayout.h"

#include "VulkanDevice.h"
#include "VulkanUtils.h"

DEFINE_LOG_CATEGORY(LogVulkanSetLayout);

CVulkanSetLayout::CVulkanSetLayout(const WeakRef<IRHIDevice> device,
                                   const FRHISetLayoutDescription& desc)
    : IRHISetLayout(device), m_SetLayout(CreateVKSetLayout(desc)) {}

CVulkanSetLayout::~CVulkanSetLayout() {
    CZ_LOG(LogVulkanSetLayout, Trace, "VulkanSetLayout destroying...");
}

vk::raii::DescriptorSetLayout
    CVulkanSetLayout::CreateVKSetLayout(const FRHISetLayoutDescription& desc) {
    auto device = m_Device.lock().As<CVulkanDevice>();
    if (!device) {
        CZ_LOG(LogVulkanPipeline, Error, "Device is no longer valid during Pipeline creation!");
        return nullptr;
    }

    const vk::raii::Device& raiiDevice = device->GetRAIILogicalDevice();

    std::vector<vk::DescriptorSetLayoutBinding> vkBindings;
    vkBindings.reserve(desc.Bindings.size());

    for (const auto& binding : desc.Bindings) {
        vk::DescriptorSetLayoutBinding b;
        b.setBinding(binding.Binding)
            .setDescriptorCount(binding.DescriptorCount)
            .setDescriptorType(ChozoUtils::Vulkan::ToVkDescType(binding.Type))
            .setStageFlags(ChozoUtils::Vulkan::StageToFlagBits(binding.StageFlags))
            .setPImmutableSamplers(nullptr);

        vkBindings.push_back(b);
    }

    vk::DescriptorSetLayoutCreateInfo layoutInfo;
    layoutInfo.setBindings(vkBindings);

    try {
        return raiiDevice.createDescriptorSetLayout(layoutInfo);
    } catch (const vk::SystemError& err) {
        CZ_CORE_ASSERT(false, "Failed to create Vulkan Descriptor Set Layout: {0}", err.what());
        return nullptr;
    }
}

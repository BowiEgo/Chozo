#include "VulkanCommandPool.h"

#include "VulkanCommandBuffer.h"
#include "VulkanDevice.h"
#include "VulkanUtils.h"

CVulkanCommandPool::CVulkanCommandPool(const TRef<CVulkanDevice>& device,
                                       const FCommandPoolSpecification& spec)
    : IRHICommandPool(spec), m_Device(device) {
    Init();
}

TRef<IRHICommandList> CVulkanCommandPool::AllocateCommandBuffer() {
    return CreateRef<CVulkanCommandBuffer>(TRef<CVulkanCommandPool>(this));
}

void CVulkanCommandPool::Init() {
    auto device = m_Device.lock();
    if (!device) {
        CZ_LOG(LogVulkanCommandPool, Error,
               "Device is no longer valid during CommandPool creation!");
        return;
    }

    const vk::raii::Device& raiiDevice = device->GetRAIILogicalDevice();

    vk::CommandPoolCreateInfo poolInfo{};
    poolInfo.flags            = ChozoUtils::Vulkan::MapCommandPoolFlags(m_Spec.Flags);
    poolInfo.queueFamilyIndex = m_Spec.QueueIndex;

    m_Handle = vk::raii::CommandPool(raiiDevice, poolInfo);
}
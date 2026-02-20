#include "VulkanRHICommandPool.h"

CVulkanRHICommandPool::CVulkanRHICommandPool(const FRHICommandPoolCreateInfo& info,
                                             const TRef<CVulkanRHIDevice>& device)
    : IRHICommandPool(info), m_Device(device) {
    Init();
}

TRef<IRHICommandList>
    CVulkanRHICommandPool::AllocateCommandList(const FRHICommandListCreateInfo& info,
                                               const TRef<IRHICommandBuffer> cmdBuffer) {
    return CreateRef<CVulkanRHICommandList>(info, cmdBuffer.As<CVulkanRHICommandBuffer>());
}

void CVulkanRHICommandPool::Init() {
    auto device = m_Device.lock();
    if (!device) {
        CZ_LOG(LogVulkanRHICommandPool, Error,
               "Device is no longer valid during CommandPool creation!");
        return;
    }

    const vk::raii::Device& raiiDevice = device->GetRAIILogicalDevice();

    vk::CommandPoolCreateInfo poolInfo;
    poolInfo.flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer;
    poolInfo.queueFamilyIndex = m_Info.QueueIndex;

    m_Handle = vk::raii::CommandPool(raiiDevice, poolInfo);
}
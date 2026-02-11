#include "VulkanRHICommandPool.h"

CVulkanRHICommandPool::CVulkanRHICommandPool(const FRHICommandPoolCreateInfo& info,
                                             const TRef<CVulkanRHIDevice> device)
    : IRHICommandPool(info), m_Device(WeakRef(device)) {

    vk::CommandPoolCreateInfo poolInfo;
    poolInfo.flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer;
    poolInfo.queueFamilyIndex = m_Info.QueueIndex;

    m_Handle = vk::raii::CommandPool(m_Device->GetLogicalDevice(), poolInfo);
}

TRef<IRHICommandList>
    CVulkanRHICommandPool::AllocateCommandList(const FRHICommandListCreateInfo& info,
                                               const TRef<IRHICommandBuffer> cmdBuffer) {
    return CreateRef<CVulkanRHICommandList>(info, cmdBuffer.As<CVulkanRHICommandBuffer>());
}

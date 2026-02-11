#include "VulkanRHICommandBuffer.h"
#include "VulkanRHICommandPool.h"

DEFINE_LOG_CATEGORY(LogVulkanRHICommandBuffer);

CVulkanRHICommandBuffer::CVulkanRHICommandBuffer(const FRHICommandBufferCreateInfo& info,
                                                 const TRef<CVulkanRHIDevice> device)
    : IRHICommandBuffer(info), m_Device(WeakRef(device)) {
    vk::CommandBufferAllocateInfo allocInfo;
    allocInfo.commandPool = m_Info.CommandPool.As<CVulkanRHICommandPool>()->GetHandle();
    allocInfo.level = vk::CommandBufferLevel::ePrimary;
    allocInfo.commandBufferCount = 1;

    vk::raii::CommandBuffers cmdBuffers(m_Device->GetLogicalDevice(), allocInfo);
    m_ActiveCmdHandle = std::move(cmdBuffers.front());
}

CVulkanRHICommandBuffer::~CVulkanRHICommandBuffer() {
    CZ_LOG(LogVulkanRHICommandBuffer, Trace, "VulkanRHICommandBuffer destroying...");
}

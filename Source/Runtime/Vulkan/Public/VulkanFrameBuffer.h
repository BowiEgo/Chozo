// In dynamic rendering, this provides AttachmentInfo arrays for the command buffer.
#pragma once

#include "RHIFrameBuffer.h"

#include "VulkanDevice.h"

#include "VulkanExport.h"

class VULKAN_API CVulkanFrameBuffer : public IRHIFrameBuffer {
public:
    CVulkanFrameBuffer(const FFrameBufferSpecification& spec, const TRef<CVulkanDevice>& device);
    virtual ~CVulkanFrameBuffer() = default;

    virtual void Resize(uint32_t width, uint32_t height) override {}

private:
    WeakRef<CVulkanDevice> m_Device;
};
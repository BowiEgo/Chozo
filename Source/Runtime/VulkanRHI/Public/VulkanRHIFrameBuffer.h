// In dynamic rendering, this provides AttachmentInfo arrays for the command buffer.
#pragma once

#include "RHIFrameBuffer.h"
#include "VulkanRHIDevice.h"

class CVulkanRHIFrameBuffer : public IRHIFrameBuffer {
public:
    CVulkanRHIFrameBuffer(const FFrameBufferSpecification& spec,
                          const TRef<CVulkanRHIDevice>& device);
    virtual ~CVulkanRHIFrameBuffer() = default;

    virtual void Resize(uint32_t width, uint32_t height) override {}

private:
    WeakRef<CVulkanRHIDevice> m_Device;
};
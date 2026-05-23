#pragma once

#include <Runtime/RHI/FrameBuffer.hpp>

#include "VulkanDeviceObj.hpp"

namespace CZ {

class VulkanFrameBufferObj : public FrameBufferObj {
public:
    VulkanFrameBufferObj(const VulkanDeviceObj* deviceObj, const FrameBufferSpecification& spec);

    ~VulkanFrameBufferObj() override;

    virtual void Resize(const Extent2D& size) override;

private:
    void Init();

    const VulkanDeviceObj* m_DeviceObj;
};
} // namespace CZ
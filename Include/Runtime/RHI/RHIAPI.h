#pragma once

#include <Runtime/RHI/Device.h>
#include <Runtime/RHI/GraphicsContext.h>
#include <Runtime/RHI/Swapchain.h>

namespace CZ {

class RHIAPI {
public:
    static RHIAPI& Get();

    bool Init(const GraphicsContextSpecification& gcSpec, std::string& err);
    void Shutdown();

    GraphicsContext GetGraphicsContext() const { return m_GraphicsContext; }

private:
    GraphicsContext CreateGraphicsContext(const GraphicsContextSpecification& spec);
    Device CreateDevice(const DeviceSpecification& spec);
    Swapchain CreateSwapchain(const SwapchainSpecification& spec);

    GraphicsContext m_GraphicsContext;
};
} // namespace CZ

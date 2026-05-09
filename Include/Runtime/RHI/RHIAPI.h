#pragma once

#include <Runtime/RHI/Device.h>
#include <Runtime/RHI/GraphicContext.h>
#include <Runtime/RHI/Swapchain.h>

namespace CZ {

class RHIAPI {
public:
    static RHIAPI& Get();

    bool Init(const GraphicContextSpecification& gcSpec, std::string& err);
    void Shutdown();

    GraphicContext GetGraphicContext() const { return m_GraphicContext; }

private:
    GraphicContext CreateGraphicContext(const GraphicContextSpecification& spec);
    Device CreateDevice(const DeviceSpecification& spec);
    Swapchain CreateSwapchain(const SwapchainSpecification& spec);

    GraphicContext m_GraphicContext;
    Device m_Device;
    Swapchain m_Swapchain;
};
} // namespace CZ

#include <Runtime/RHI/RHIAPI.h>

#include <Runtime/RHI/GraphicContext.h>

namespace CZ {

DEFINE_LOG_CATEGORY_STATIC(LogRHIAPI, Info);

RHIAPI& RHIAPI::Get() {
    static RHIAPI Instance;
    return Instance;
}

bool RHIAPI::Init(const GraphicContextSpecification& gcSpec, std::string& err) {
    auto& registry = BackendRegistry::Get();
    if (!registry.LoadBackend("vulkan", "./libCZVulkan.dylib")) {
        err = "Cannot load Vulkan backend.";
        return false;
    }

    m_GraphicContext = CreateGraphicContext(gcSpec);

    {
        DeviceSpecification spec;
        spec.AppName        = "Chozo Engine";
        spec.AppVersion     = 1;
        spec.GraphicContext = m_GraphicContext;

        m_Device = CreateDevice(spec);
    }

    {
        SwapchainSpecification spec;
        spec.FrameBufferSize = gcSpec.FrameBufferSize;
        spec.NativeWindow    = gcSpec.NativeWindow;
        m_Swapchain          = CreateSwapchain(spec);
    }

    return true;
}

void RHIAPI::Shutdown() {
    // Swapchain::Destroy(m_Swapchain);
    // Device::Destroy(m_Device);
    // GraphicContext::Destroy(m_GraphicContext);
}

GraphicContext RHIAPI::CreateGraphicContext(const GraphicContextSpecification& spec) {
    auto& registry = BackendRegistry::Get();

    auto createFn =
        registry.GetFunction<GraphicContextObj* (*)(const GraphicContextSpecification&)>(
            "vulkan", "CreateVulkanGraphicContextObj");
    if (!createFn) {
        CZ_LOG(LogRHIAPI, Error, "CreateVulkanGraphicContextObj not found in backend.");
        return {};
    }

    GraphicContextObj* obj = createFn(spec);
    if (!obj) {
        CZ_LOG(LogRHIAPI, Error, "Backend failed to create GraphicContextObj.");
        return {};
    }

    CZ_LOG(LogRHIAPI, Info, "GraphicContext created via backend 'vulkan'.");
    return GraphicContext(obj);
}

Device RHIAPI::CreateDevice(const DeviceSpecification& spec) {
    auto& registry = BackendRegistry::Get();

    auto createFn = registry.GetFunction<DeviceObj* (*)(const DeviceSpecification&)>(
        "vulkan", "CreateVulkanDeviceObj");
    if (!createFn) {
        CZ_LOG(LogRHIAPI, Error, "CreateVulkanDeviceObj not found in backend.");
        return {};
    }

    DeviceObj* obj = createFn(spec);
    if (!obj) {
        CZ_LOG(LogRHIAPI, Error, "Backend failed to create DeviceObj.");
        return {};
    }

    CZ_LOG(LogRHIAPI, Info, "Device created via backend 'vulkan'.");
    return Device(obj);
}

Swapchain RHIAPI::CreateSwapchain(const SwapchainSpecification& spec) {
    auto& registry = BackendRegistry::Get();

    auto createFn =
        registry.GetFunction<SwapchainObj* (*)(const Device device, const SwapchainSpecification&)>(
            "vulkan", "CreateVulkanSwapchainObj");
    if (!createFn) {
        CZ_LOG(LogRHIAPI, Error, "CreateVulkanSwapchainObj not found in backend.");
        return {};
    }

    SwapchainObj* obj = createFn(m_Device, spec);
    if (!obj) {
        CZ_LOG(LogRHIAPI, Error, "Backend failed to create SwapchainObj.");
        return {};
    }

    CZ_LOG(LogRHIAPI, Info, "Swapchain created via backend 'vulkan'.");
    return Swapchain(obj);
}

} // namespace CZ
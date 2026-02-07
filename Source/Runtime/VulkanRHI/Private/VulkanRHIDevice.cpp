#include "VulkanRHIDevice.h"

#include "VulkanRHIPipeline.h"
#include "VulkanRHIShader.h"
#include "VulkanUtils.h"

DEFINE_LOG_CATEGORY(LogVulkanRHIDevice);

CVulkanRHIDevice::CVulkanRHIDevice(const FRHIDeviceCreateInfo& info,
                                   const vk::raii::Instance& instance,
                                   const vk::raii::SurfaceKHR& surface)
    : IRHIDevice(info) {
    PickPhysicalDevice(instance);
    CreateLogicalDevice(surface);
}

CVulkanRHIDevice::~CVulkanRHIDevice() {
    CZ_LOG(LogVulkanRHIDevice, Trace, "Destroying Vulkan Device...");
}

void CVulkanRHIDevice::PickPhysicalDevice(const vk::raii::Instance& instance) {
    auto devices = instance.enumeratePhysicalDevices();

    if (devices.empty()) {
        CZ_LOG(LogVulkanRHIDevice, Fatal,
               "Failed to find GPUs with Vulkan support");
    }

    for (const auto& device : devices) {
        m_PhysicalDevice = device;
        break;
    }

    auto deviceProperties = m_PhysicalDevice.getProperties();
    auto deviceFeatures = m_PhysicalDevice.getFeatures();

    ChozoUtils::Vulkan::LogPhysicalDeviceInfo(deviceProperties);
    ChozoUtils::Vulkan::LogMemoryBudget(m_PhysicalDevice);
}

void CVulkanRHIDevice::CreateLogicalDevice(
    const vk::raii::SurfaceKHR& surface) {

    FQueueFamilyIndices indices =
        ChozoUtils::Vulkan::FindQueueFamilies(m_PhysicalDevice, surface);

    std::set<uint32_t> uniqueQueueFamilies;
    if (indices.Graphics.has_value())
        uniqueQueueFamilies.insert(indices.Graphics.value());
    if (indices.Present.has_value())
        uniqueQueueFamilies.insert(indices.Present.value());
    if (indices.Compute.has_value())
        uniqueQueueFamilies.insert(indices.Compute.value());

    std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos;
    float queuePriority = 1.0f; // range is [0.0, 1.0]
    for (uint32_t queueFamily : uniqueQueueFamilies) {
        queueCreateInfos.push_back({{}, queueFamily, 1, &queuePriority});
    }

    // Enable Dynamic State (Viewport/Scissor/etc.) to avoid pipeline
    // bloat
    vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT
        extendedDynamicStateFeatures;
    extendedDynamicStateFeatures.extendedDynamicState = vk::True;

    // Enable Modern Rendering (Vulkan 1.3)
    vk::PhysicalDeviceVulkan13Features vulkan13Features;
    vulkan13Features.dynamicRendering = vk::True;
    vulkan13Features.pNext = &extendedDynamicStateFeatures;

    // Link features to DeviceCreateInfo
    std::vector<const char*> deviceExtensions = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME};

    vk::DeviceCreateInfo deviceCreateInfo({}, queueCreateInfos, {},
                                          deviceExtensions);
    deviceCreateInfo.pNext = &vulkan13Features; // Hook the feature chain here

    // Create the logical device
    try {
        m_LogicalDevice = vk::raii::Device(m_PhysicalDevice, deviceCreateInfo);

        // Safely fetch queue handles
        if (indices.Graphics.has_value())
            m_GraphicsQueue =
                vk::raii::Queue(m_LogicalDevice, indices.Graphics.value(), 0);
        if (indices.Present.has_value())
            m_PresentQueue =
                vk::raii::Queue(m_LogicalDevice, indices.Present.value(), 0);
        if (indices.Compute.has_value())
            m_ComputeQueue =
                vk::raii::Queue(m_LogicalDevice, indices.Compute.value(), 0);

        CZ_LOG(LogVulkanRHIDevice, Info,
               "Queue Family Indices -> Graphics: {}, Present: {}, Compute: {}",
               indices.Graphics.value(), indices.Present.value(),
               indices.Compute.value());

    } catch (const std::exception& e) {
        CZ_LOG(LogVulkanRHIDevice, Error, "Vulkan Device Creation Failed: {0}",
               e.what());
    }

    CZ_LOG(LogVulkanRHIDevice, Info, "Vulkan Logical Device Created.");
}

TRef<IRHIShader>
    CVulkanRHIDevice::CreateShader(const FRHIShaderCreateInfo& info,
                                   const std::vector<uint32_t>* binary) const {
    return TRef<CVulkanRHIShader>::Create(info, binary,
                                          TRef<CVulkanRHIDevice>(this));
}

TRef<IRHIPipeline>
    CVulkanRHIDevice::CreatePipeline(const FRHIPipelineCreateInfo& info) const {
    return TRef<CVulkanRHIPipeline>::Create(info, TRef<CVulkanRHIDevice>(this));
}
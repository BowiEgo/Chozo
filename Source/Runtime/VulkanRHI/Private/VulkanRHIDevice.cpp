#include "VulkanRHIDevice.h"

#include "VulkanRHIPipeline.h"
#include "VulkanRHIShader.h"
#include "VulkanRHISyncObject.h"
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
    std::vector<vk::raii::PhysicalDevice> devices = instance.enumeratePhysicalDevices();
    const auto devIter = std::ranges::find_if(devices, [&](auto const& device) {
        // Check if the device supports the Vulkan 1.3 API version
        bool supportsVulkan1_3 = device.getProperties().apiVersion >= VK_API_VERSION_1_3;

        // Check if any of the queue families support graphics operations
        auto queueFamilies = device.getQueueFamilyProperties();
        bool supportsGraphics = std::ranges::any_of(queueFamilies, [](auto const& qfp) {
            return !!(qfp.queueFlags & vk::QueueFlagBits::eGraphics);
        });

        // Check if all required device extensions are available
        auto availableDeviceExtensions = device.enumerateDeviceExtensionProperties();
        bool supportsAllRequiredExtensions = std::ranges::all_of(
            m_RequiredDeviceExtension,
            [&availableDeviceExtensions](auto const& m_RequiredDeviceExtension) {
                return std::ranges::any_of(
                    availableDeviceExtensions,
                    [m_RequiredDeviceExtension](auto const& availableDeviceExtension) {
                        return strcmp(availableDeviceExtension.extensionName,
                                      m_RequiredDeviceExtension) == 0;
                    });
            });

        auto features =
            device.template getFeatures2<vk::PhysicalDeviceFeatures2,
                                         vk::PhysicalDeviceVulkan11Features,
                                         vk::PhysicalDeviceVulkan13Features,
                                         vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT>();
        bool supportsRequiredFeatures =
            features.template get<vk::PhysicalDeviceVulkan11Features>().shaderDrawParameters &&
            features.template get<vk::PhysicalDeviceVulkan13Features>().synchronization2 &&
            features.template get<vk::PhysicalDeviceVulkan13Features>().dynamicRendering &&
            features.template get<vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT>()
                .extendedDynamicState;

        return supportsVulkan1_3 && supportsGraphics && supportsAllRequiredExtensions &&
               supportsRequiredFeatures;
    });
    if (devIter != devices.end()) {
        m_PhysicalDevice = *devIter;
        auto deviceProperties = m_PhysicalDevice.getProperties();
        auto deviceFeatures = m_PhysicalDevice.getFeatures();

        ChozoUtils::Vulkan::LogPhysicalDeviceInfo(deviceProperties);
        ChozoUtils::Vulkan::LogMemoryBudget(m_PhysicalDevice);
    } else {
        CZ_LOG(LogVulkanRHIDevice, Fatal, "Failed to find GPUs with Vulkan support");
    }
}

void CVulkanRHIDevice::CreateLogicalDevice(const vk::raii::SurfaceKHR& surface) {
    FQueueFamilyIndices indices = ChozoUtils::Vulkan::FindQueueFamilies(m_PhysicalDevice, surface);

    std::set<uint32_t> uniqueQueueFamilies;
    if (indices.Graphics.has_value())
        uniqueQueueFamilies.insert(indices.Graphics.value());
    if (indices.Present.has_value())
        uniqueQueueFamilies.insert(indices.Present.value());
    if (indices.Compute.has_value())
        uniqueQueueFamilies.insert(indices.Compute.value());

    vk::PhysicalDeviceFeatures2 features2;
    vk::PhysicalDeviceVulkan11Features features11;
    features11.shaderDrawParameters = true;

    vk::PhysicalDeviceVulkan13Features features13;
    features13.synchronization2 = true;
    features13.dynamicRendering = true;

    vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT extendedFeatures;
    extendedFeatures.extendedDynamicState = true;

    vk::StructureChain<vk::PhysicalDeviceFeatures2, vk::PhysicalDeviceVulkan11Features,
                       vk::PhysicalDeviceVulkan13Features,
                       vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT>
        featureChain(features2, features11, features13, extendedFeatures);

    float queuePriority = 0.5f;
    vk::DeviceQueueCreateInfo deviceQueueCreateInfo;
    deviceQueueCreateInfo.queueFamilyIndex = indices.Graphics.value();
    deviceQueueCreateInfo.queueCount = 1;
    deviceQueueCreateInfo.pQueuePriorities = &queuePriority;

    // Link features to DeviceCreateInfo
    vk::DeviceCreateInfo deviceCreateInfo;
    deviceCreateInfo.pNext = &featureChain.get<vk::PhysicalDeviceFeatures2>();
    deviceCreateInfo.queueCreateInfoCount = 1;
    deviceCreateInfo.pQueueCreateInfos = &deviceQueueCreateInfo;
    deviceCreateInfo.enabledExtensionCount =
        static_cast<uint32_t>(m_RequiredDeviceExtension.size());
    deviceCreateInfo.ppEnabledExtensionNames = m_RequiredDeviceExtension.data();

    // Create the logical device
    try {
        m_LogicalDevice = vk::raii::Device(m_PhysicalDevice, deviceCreateInfo);

        // Safely fetch queue handles
        if (indices.Graphics.has_value()) {
            m_GraphicsQueue = vk::raii::Queue(m_LogicalDevice, indices.Graphics.value(), 0);
            m_GraphicsQueueIndex = indices.Graphics.value();
        }
        if (indices.Present.has_value())
            m_PresentQueue = vk::raii::Queue(m_LogicalDevice, indices.Present.value(), 0);
        if (indices.Compute.has_value())
            m_ComputeQueue = vk::raii::Queue(m_LogicalDevice, indices.Compute.value(), 0);

        CZ_LOG(LogVulkanRHIDevice, Info,
               "Queue Family Indices -> Graphics: {}, Present: {}, Compute: {}",
               indices.Graphics.value(), indices.Present.value(), indices.Compute.value());

    } catch (const std::exception& e) {
        CZ_LOG(LogVulkanRHIDevice, Error, "Vulkan Device Creation Failed: {0}", e.what());
    }

    CZ_LOG(LogVulkanRHIDevice, Info, "Vulkan Logical Device Created.");
}

TRef<IRHIShader> CVulkanRHIDevice::CreateShader(const FRHIShaderCreateInfo& info,
                                                const std::vector<uint32_t>* binary) const {
    return TRef<CVulkanRHIShader>::Create(info, binary, TRef<CVulkanRHIDevice>(this));
}

TRef<IRHIPipeline> CVulkanRHIDevice::CreatePipeline(const FRHIPipelineCreateInfo& info) const {
    return TRef<CVulkanRHIPipeline>::Create(info, TRef<CVulkanRHIDevice>(this));
}

TRef<IRHISyncObject> CVulkanRHIDevice::CreateSyncObject() const {
    return TRef<CVulkanRHISyncObject>::Create(TRef<CVulkanRHIDevice>(this));
}

void CVulkanRHIDevice::WaitIdle() { m_LogicalDevice.waitIdle(); }

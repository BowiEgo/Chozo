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
    InitGlobalDescriptorPool();
}

CVulkanRHIDevice::~CVulkanRHIDevice() {
    CZ_LOG(LogVulkanRHIDevice, Trace, "Destroying Vulkan Device...");
    for (auto& [type, layout] : m_LayoutCache) {
        if (layout) {
            vk::Device rawDevice = *m_LogicalDevice;
            rawDevice.destroyDescriptorSetLayout(layout);
        }
    }
    m_LayoutCache.clear();
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

        // [Note] Get all available extensions for this specific physical device
        auto availableDeviceExtensions = device.enumerateDeviceExtensionProperties();
        // [Note] Required check: If "VK_KHR_portability_subset" exists, it MUST be enabled
        bool needsPortability = std::ranges::any_of(availableDeviceExtensions, [](auto const& ext) {
            return strcmp(ext.extensionName, "VK_KHR_portability_subset") == 0;
        });
        // [Note] Update member variable m_RequiredDeviceExtension for this specific device
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

        // [Note] Finalize the extension list for the chosen device
        auto availableExts = m_PhysicalDevice.enumerateDeviceExtensionProperties();
        for (auto const& ext : availableExts) {
            if (strcmp(ext.extensionName, "VK_KHR_portability_subset") == 0) {
                m_RequiredDeviceExtension.push_back("VK_KHR_portability_subset");
                break;
            }
        }

        ChozoUtils::Vulkan::LogPhysicalDeviceInfo(m_PhysicalDevice.getProperties());
        ChozoUtils::Vulkan::LogMemoryBudget(m_PhysicalDevice);
    } else {
        CZ_LOG(LogVulkanRHIDevice, Fatal, "Failed to find GPUs with Vulkan support");
    }
}

void CVulkanRHIDevice::CreateLogicalDevice(const vk::raii::SurfaceKHR& surface) {
    FQueueFamilyIndices indices = ChozoUtils::Vulkan::FindQueueFamilies(m_PhysicalDevice, surface);

    std::set<uint32> uniqueQueueFamilies;
    if (indices.Graphics.has_value()) uniqueQueueFamilies.insert(indices.Graphics.value());
    if (indices.Present.has_value()) uniqueQueueFamilies.insert(indices.Present.value());
    if (indices.Compute.has_value()) uniqueQueueFamilies.insert(indices.Compute.value());

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
    deviceCreateInfo.enabledExtensionCount = static_cast<uint32>(m_RequiredDeviceExtension.size());
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

void CVulkanRHIDevice::InitGlobalDescriptorPool() {
    std::vector<vk::DescriptorPoolSize> poolSizes = {
        { vk::DescriptorType::eCombinedImageSampler, 1000 },
        { vk::DescriptorType::eSampledImage, 1000 },
        { vk::DescriptorType::eStorageImage, 1000 },
        { vk::DescriptorType::eUniformBuffer, 1000 }
    };

    m_GlobalDescriptorPool = CreateDescriptorPool(1000, poolSizes);
}

vk::raii::DescriptorPool
    CVulkanRHIDevice::CreateDescriptorPool(uint32 maxSets,
                                           const std::vector<vk::DescriptorPoolSize>& poolSizes) {
    vk::raii::DescriptorPool result = nullptr;

    vk::DescriptorPoolCreateInfo poolInfo;
    poolInfo.flags = vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet;
    poolInfo.maxSets = maxSets;
    poolInfo.setPoolSizes(poolSizes);

    try {
        result = vk::raii::DescriptorPool(m_LogicalDevice, poolInfo);
        CZ_LOG(LogVulkanRHIDevice, Info, "Vulkan Descriptor Pool Created with max sets: {}",
               maxSets);
    } catch (const std::exception& e) {
        CZ_LOG(LogVulkanRHIDevice, Error, "Failed to create Vulkan Descriptor Pool: {0}", e.what());
    }

    return result;
}

uint32 CVulkanRHIDevice::FindMemoryType(uint32 typeFilter,
                                        vk::MemoryPropertyFlags properties) const {
    vk::PhysicalDeviceMemoryProperties memProperties = m_PhysicalDevice.getMemoryProperties();

    for (uint32 i = 0; i < memProperties.memoryTypeCount; i++) {
        // Check if the memory type bit is set in the filter
        // AND if it matches the required property flags.
        if ((typeFilter & (1 << i)) &&
            (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
            return i;
        }
    }

    CZ_CORE_ASSERT(false, "Failed to find suitable memory type!");
    return 0;
}

TRef<IRHIShader> CVulkanRHIDevice::CreateShader(const FRHIShaderCreateInfo& info,
                                                const std::vector<uint32>* binary) const {
    return TRef<CVulkanRHIShader>::Create(info, binary, TRef<CVulkanRHIDevice>(this));
}

TRef<IRHIPipeline> CVulkanRHIDevice::CreatePipeline(const FRHIPipelineCreateInfo& info) const {
    return TRef<CVulkanRHIPipeline>::Create(info, TRef<CVulkanRHIDevice>(this));
}

TRef<IRHISyncObject> CVulkanRHIDevice::CreateSyncObject() const {
    return TRef<CVulkanRHISyncObject>::Create(TRef<CVulkanRHIDevice>(this));
}

void CVulkanRHIDevice::WaitIdle() { m_LogicalDevice.waitIdle(); }

vk::DescriptorSetLayout CVulkanRHIDevice::GetDescriptorSetLayout(EDescriptorLayoutType type) {
    // Return cached layout if available.
    if (m_LayoutCache.contains(type)) {
        return m_LayoutCache[type];
    }

    vk::DescriptorSetLayoutCreateInfo layoutInfo;
    std::vector<vk::DescriptorSetLayoutBinding> bindings;

    switch (type) {
        case EDescriptorLayoutType::CombinedImageSampler: {
            // Standard binding for a texture and its sampler.
            vk::DescriptorSetLayoutBinding binding;
            binding.setBinding(0)
                .setDescriptorType(vk::DescriptorType::eCombinedImageSampler)
                .setDescriptorCount(1)
                .setStageFlags(vk::ShaderStageFlagBits::eFragment) // Typically for UI/Fragment
                .setPImmutableSamplers(nullptr);
            bindings.push_back(binding);
            break;
        }
        case EDescriptorLayoutType::UniformBuffer: {
            vk::DescriptorSetLayoutBinding binding;
            binding.setBinding(0)
                .setDescriptorType(vk::DescriptorType::eUniformBuffer)
                .setDescriptorCount(1)
                .setStageFlags(vk::ShaderStageFlagBits::eVertex |
                               vk::ShaderStageFlagBits::eFragment);
            bindings.push_back(binding);
            break;
        }
        case EDescriptorLayoutType::StorageImage: {
            // vk::DescriptorSetLayoutBinding binding;
            // binding.setBinding(0)
            //     .setDescriptorType(vk::DescriptorType::eUniformBuffer)
            //     .setDescriptorCount(1)
            //     .setStageFlags(vk::ShaderStageFlagBits::eVertex |
            //     vk::ShaderStageFlagBits::eFragment);
            // bindings.push_back(binding);
            break;
        }
    }

    layoutInfo.setBindings(bindings);

    // Create the layout and store it in the cache.
    vk::Device rawDevice = *m_LogicalDevice;
    vk::DescriptorSetLayout newLayout = rawDevice.createDescriptorSetLayout(layoutInfo);
    m_LayoutCache[type] = newLayout;

    return newLayout;
}

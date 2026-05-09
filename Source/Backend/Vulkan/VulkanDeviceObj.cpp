#include "VulkanDeviceObj.h"

#include "VulkanGraphicContextObj.h"
#include "VulkanUtils.h"

#include <Core/Memory/Memory.h>

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wnullability-completeness"
#pragma clang diagnostic ignored "-Wunused-parameter"
#pragma clang diagnostic ignored "-Wunused-variable"
#pragma clang diagnostic ignored "-Wmissing-field-initializers"
#pragma clang diagnostic ignored "-Wunused-private-field"
#pragma clang diagnostic ignored "-Wunused-function"
#define VMA_IMPLEMENTATION
#include "vk_mem_alloc.h"
#pragma clang diagnostic pop

#include <set>

namespace CZ {

DEFINE_LOG_CATEGORY_STATIC(LogVulkanDevice, Info);

extern "C" {

DeviceObj* CreateVulkanDeviceObj(const DeviceSpecification& spec) {
    return New<VulkanDeviceObj>(MEMORY_USAGE_RENDER, spec);
}

void DestroyVulkanDeviceObj(DeviceObj* obj) { Delete(obj); }

} // extern "C"

VulkanDeviceObj::VulkanDeviceObj(const DeviceSpecification& spec) : DeviceObj(spec) {
    PickPhysicalDevice(spec.GraphicContext);
    CreateLogicalDevice(spec.GraphicContext);
    CreateVmaAllocator(spec.GraphicContext);
    CZ_LOG(LogVulkanDevice, Info, "VulkanDeviceObj created.");
}

VulkanDeviceObj::~VulkanDeviceObj() {
    if (m_VkDevice) {
        vkDestroyDevice(m_VkDevice, nullptr);
    }

    if (m_VmaAllocator) {
        vmaDestroyAllocator(m_VmaAllocator);
    }

    CZ_LOG(LogVulkanDevice, Info, "VulkanDeviceObj destroyed.");
}

// --- Public ---

void VulkanDeviceObj::WaitIdle() { vkDeviceWaitIdle(m_VkDevice); }

bool VulkanDeviceObj::IsExtensionSupported(const std::string& extensionName) const {
    uint32_t extCount = 0;
    vkEnumerateDeviceExtensionProperties(m_VkPhysicalDevice, nullptr, &extCount, nullptr);
    std::vector<VkExtensionProperties> extensions(extCount);
    vkEnumerateDeviceExtensionProperties(m_VkPhysicalDevice, nullptr, &extCount, extensions.data());

    for (const auto& ext : extensions) {
        if (ext.extensionName == extensionName) {
            return true;
        }
    }
    return false;
}

VkDescriptorPool
    VulkanDeviceObj::CreateDescriptorPool(uint32_t maxSets,
                                          const std::vector<VkDescriptorPoolSize>& poolSizes) {
    VkDescriptorPool result = VK_NULL_HANDLE;

    VkDescriptorPoolCreateInfo poolInfo{};
    poolInfo.sType         = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.flags         = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
    poolInfo.maxSets       = maxSets;
    poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
    poolInfo.pPoolSizes    = poolSizes.data();

    VkResult res = vkCreateDescriptorPool(m_VkDevice, &poolInfo, nullptr, &result);
    if (res == VK_SUCCESS) {
        CZ_LOG(LogVulkanDevice, Info, "Vulkan Descriptor Pool Created with max sets: {}", maxSets);
    } else {
        CZ_LOG(LogVulkanDevice, Error, "Failed to create Vulkan Descriptor Pool: %s",
               VulkanUtils::VkResultToString(res));
    }

    return result;
}
// --- Private ---

void VulkanDeviceObj::PickPhysicalDevice(GraphicContext context) {
    auto* ctxObj          = static_cast<VulkanGraphicContextObj*>(context.Unwrap());
    VkInstance vkInstance = ctxObj->GetVKInstance();

    // 1. Enumerate physical devices
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(vkInstance, &deviceCount, nullptr);
    if (deviceCount == 0) {
        CZ_LOG(LogVulkanDevice, Fatal, "No Vulkan-capable GPUs found.");
        return;
    }
    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(vkInstance, &deviceCount, devices.data());

    VkPhysicalDevice selectedDevice = VK_NULL_HANDLE;
    for (uint32_t i = 0; i < deviceCount; ++i) {
        VkPhysicalDevice device = devices[i];

        // 2. Check API version
        VkPhysicalDeviceProperties props;
        vkGetPhysicalDeviceProperties(device, &props);
        bool supportsVulkan1_3 = (props.apiVersion >= VK_API_VERSION_1_3);

        // 3. Check queueFamily
        uint32_t queueFamilyCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);
        std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());
        bool supportsGraphics = false;
        for (const auto& qf : queueFamilies) {
            if (qf.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
                supportsGraphics = true;
                break;
            }
        }

        // 4. Enumerate device extensions and check extensions required
        uint32_t extCount = 0;
        vkEnumerateDeviceExtensionProperties(device, nullptr, &extCount, nullptr);
        std::vector<VkExtensionProperties> availableExts(extCount);
        vkEnumerateDeviceExtensionProperties(device, nullptr, &extCount, availableExts.data());

        bool supportsAllRequiredExtensions = true;
        for (uint32_t i = 0; i < m_RequiredDeviceExtensionCount; ++i) {
            const char* reqExt = m_RequiredDeviceExtensions[i];
            bool found         = false;
            for (const auto& avail : availableExts) {
                if (strcmp(reqExt, avail.extensionName) == 0) {
                    found = true;
                    break;
                }
            }
            if (!found) {
                supportsAllRequiredExtensions = false;
                break;
            }
        }

        VkPhysicalDeviceExtendedDynamicStateFeaturesEXT extDynamicState{};
        extDynamicState.sType =
            VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTENDED_DYNAMIC_STATE_FEATURES_EXT;

        VkPhysicalDeviceVulkan13Features vulkan13Features{};
        vulkan13Features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES;
        vulkan13Features.pNext = &extDynamicState;

        VkPhysicalDeviceVulkan11Features vulkan11Features{};
        vulkan11Features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES;
        vulkan11Features.pNext = &vulkan13Features;

        VkPhysicalDeviceFeatures2 features2{};
        features2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
        features2.pNext = &vulkan11Features;
        vkGetPhysicalDeviceFeatures2(device, &features2);

        bool supportsRequiredFeatures = vulkan11Features.shaderDrawParameters != VK_FALSE &&
                                        vulkan13Features.synchronization2 != VK_FALSE &&
                                        vulkan13Features.dynamicRendering != VK_FALSE &&
                                        extDynamicState.extendedDynamicState != VK_FALSE;

        if (supportsVulkan1_3 && supportsGraphics && supportsAllRequiredExtensions &&
            supportsRequiredFeatures) {
            selectedDevice = device;
            for (const auto& avail : availableExts) {
                if (strcmp(avail.extensionName, VK_KHR_PORTABILITY_SUBSET_EXTENSION_NAME) == 0) {
                    if (m_RequiredDeviceExtensionCount < MAX_DEVICE_EXTENSIONS) {
                        m_RequiredDeviceExtensions[m_RequiredDeviceExtensionCount++] =
                            VK_KHR_PORTABILITY_SUBSET_EXTENSION_NAME;
                    }
                    break;
                }
            }
            break;
        }
    }

    if (selectedDevice == VK_NULL_HANDLE) {
        CZ_LOG(LogVulkanDevice, Fatal, "Failed to find a suitable GPU with required features.");
        return;
    }

    m_VkPhysicalDevice = selectedDevice;

    VulkanUtils::LogPhysicalDeviceInfo(m_VkPhysicalDevice);
    VulkanUtils::LogMemoryBudget(m_VkPhysicalDevice);
}

void VulkanDeviceObj::CreateLogicalDevice(GraphicContext context) {
    auto* ctxObj           = static_cast<VulkanGraphicContextObj*>(context.Unwrap());
    VkSurfaceKHR vkSurface = ctxObj->GetVKSurface();

    QueueFamilyIndices indices = VulkanUtils::FindQueueFamilies(m_VkPhysicalDevice, vkSurface);

    std::set<uint32_t> uniqueQueueFamilies;
    if (indices.Graphics.has_value()) uniqueQueueFamilies.insert(indices.Graphics.value());
    if (indices.Present.has_value()) uniqueQueueFamilies.insert(indices.Present.value());
    if (indices.Compute.has_value()) uniqueQueueFamilies.insert(indices.Compute.value());

    // ===== Check and prepare extensions =====
    bool hasSwapchainMaintenance =
        IsExtensionSupported(VK_EXT_SWAPCHAIN_MAINTENANCE_1_EXTENSION_NAME);
    bool hasSurfaceMaintenance = IsExtensionSupported(VK_EXT_SURFACE_MAINTENANCE_1_EXTENSION_NAME);
    bool hasDynamicState3 = IsExtensionSupported(VK_EXT_EXTENDED_DYNAMIC_STATE_3_EXTENSION_NAME);
    bool hasMemoryBudget  = IsExtensionSupported(VK_EXT_MEMORY_BUDGET_EXTENSION_NAME);

    auto addExtensionIfNeeded = [this](const char* extName) {
        bool found = false;
        for (uint32_t i = 0; i < m_RequiredDeviceExtensionCount; ++i) {
            if (strcmp(m_RequiredDeviceExtensions[i], extName) == 0) {
                found = true;
                break;
            }
        }
        if (!found && m_RequiredDeviceExtensionCount < MAX_DEVICE_EXTENSIONS) {
            m_RequiredDeviceExtensions[m_RequiredDeviceExtensionCount++] = extName;
        }
    };

    addExtensionIfNeeded(VK_KHR_SWAPCHAIN_EXTENSION_NAME);

    if (hasSwapchainMaintenance && hasSurfaceMaintenance) {
        CZ_LOG(LogVulkanDevice, Info,
               "VK_EXT_swapchain_maintenance1 and VK_EXT_surface_maintenance1 are supported");
        addExtensionIfNeeded(VK_EXT_SWAPCHAIN_MAINTENANCE_1_EXTENSION_NAME);
        addExtensionIfNeeded(VK_EXT_SURFACE_MAINTENANCE_1_EXTENSION_NAME);
    }

    if (hasDynamicState3) {
        CZ_LOG(LogVulkanDevice, Info, "VK_EXT_extended_dynamic_state_3 is supported");
        addExtensionIfNeeded(VK_EXT_EXTENDED_DYNAMIC_STATE_3_EXTENSION_NAME);
    }

    if (hasMemoryBudget) {
        CZ_LOG(LogVulkanDevice, Info, "VK_EXT_memory_budget is supported");
        addExtensionIfNeeded(VK_EXT_MEMORY_BUDGET_EXTENSION_NAME);
    }

    // ----- Device features -----
    VkPhysicalDeviceFeatures deviceFeatures{};

    // pNext chain
    VkPhysicalDeviceSwapchainMaintenance1FeaturesEXT swapchainMaintenanceFeatures{};
    swapchainMaintenanceFeatures.sType =
        VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SWAPCHAIN_MAINTENANCE_1_FEATURES_EXT;
    if (hasSwapchainMaintenance) {
        swapchainMaintenanceFeatures.swapchainMaintenance1 = VK_TRUE;
    }

    VkPhysicalDeviceExtendedDynamicState3FeaturesEXT dynamicState3Features{};
    dynamicState3Features.sType =
        VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTENDED_DYNAMIC_STATE_3_FEATURES_EXT;
    if (hasDynamicState3) {
        dynamicState3Features.extendedDynamicState3PolygonMode     = VK_TRUE;
        dynamicState3Features.extendedDynamicState3DepthClipEnable = VK_TRUE;
    }
    dynamicState3Features.pNext = &swapchainMaintenanceFeatures;

    VkPhysicalDeviceExtendedDynamicStateFeaturesEXT extendedFeatures{};
    extendedFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTENDED_DYNAMIC_STATE_FEATURES_EXT;
    extendedFeatures.extendedDynamicState = VK_TRUE;
    extendedFeatures.pNext                = &dynamicState3Features;

    VkPhysicalDeviceVulkan13Features features13{};
    features13.sType            = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES;
    features13.synchronization2 = VK_TRUE;
    features13.dynamicRendering = VK_TRUE;
    features13.pNext            = &extendedFeatures;

    VkPhysicalDeviceVulkan11Features features11{};
    features11.sType                = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES;
    features11.shaderDrawParameters = VK_TRUE;
    features11.pNext                = &features13;

    VkPhysicalDeviceFeatures2 features2{};
    features2.sType                     = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
    features2.features.fillModeNonSolid = VK_TRUE;
    features2.pNext                     = &features11;

    vkGetPhysicalDeviceFeatures2(m_VkPhysicalDevice, &features2);

    // ----- Queue create info -----
    float queuePriority = 0.5f;
    VkDeviceQueueCreateInfo queueCreateInfo{};
    queueCreateInfo.sType            = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queueCreateInfo.queueFamilyIndex = indices.Graphics.value();
    queueCreateInfo.queueCount       = 1;
    queueCreateInfo.pQueuePriorities = &queuePriority;

    // ----- Device create info -----
    VkDeviceCreateInfo deviceCreateInfo{};
    deviceCreateInfo.sType                   = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    deviceCreateInfo.pNext                   = &features2;
    deviceCreateInfo.queueCreateInfoCount    = 1;
    deviceCreateInfo.pQueueCreateInfos       = &queueCreateInfo;
    deviceCreateInfo.enabledExtensionCount   = m_RequiredDeviceExtensionCount;
    deviceCreateInfo.ppEnabledExtensionNames = m_RequiredDeviceExtensions;
    deviceCreateInfo.pEnabledFeatures        = nullptr; // for VkPhysicalDeviceFeatures if needed

    // Create logical device
    VkResult result = vkCreateDevice(m_VkPhysicalDevice, &deviceCreateInfo, nullptr, &m_VkDevice);
    if (result != VK_SUCCESS) {
        CZ_LOG(LogVulkanDevice, Error, "Vulkan Device Creation Failed: {}",
               VulkanUtils::VkResultToString(result));
        return;
    }

    // Get queues
    if (indices.Graphics.has_value()) {
        vkGetDeviceQueue(m_VkDevice, indices.Graphics.value(), 0, &m_GraphicsQueue);
        m_GraphicsQueueIndex = indices.Graphics.value();
    }
    if (indices.Present.has_value())
        vkGetDeviceQueue(m_VkDevice, indices.Present.value(), 0, &m_PresentQueue);
    if (indices.Compute.has_value())
        vkGetDeviceQueue(m_VkDevice, indices.Compute.value(), 0, &m_ComputeQueue);

    LoadDynamicState3Functions();

    CZ_LOG(LogVulkanDevice, Info, "Queue Family Indices -> Graphics: {}, Present: {}, Compute: {}",
           indices.Graphics.value(), indices.Present.value(), indices.Compute.value());

    CZ_LOG(LogVulkanDevice, Info, "Vulkan Logical Device Created.");
}

void VulkanDeviceObj::CreateVmaAllocator(GraphicContext context) {
    auto* ctxObj          = static_cast<VulkanGraphicContextObj*>(context.Unwrap());
    VkInstance vkInstance = ctxObj->GetVKInstance();

    VmaAllocatorCreateInfo allocatorInfo = {};
    allocatorInfo.physicalDevice         = m_VkPhysicalDevice;
    allocatorInfo.device                 = m_VkDevice;
    allocatorInfo.instance               = vkInstance;
    allocatorInfo.flags                  = VMA_ALLOCATOR_CREATE_EXT_MEMORY_BUDGET_BIT;
    vmaCreateAllocator(&allocatorInfo, &m_VmaAllocator);
}

void VulkanDeviceObj::InitGlobalDescriptorPool() {
    uint32_t poolCapacity                       = 10000;
    std::vector<VkDescriptorPoolSize> poolSizes = {
        { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, poolCapacity },
        { VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, poolCapacity },
        { VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, poolCapacity },
        { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, poolCapacity }
    };

    m_GlobalDescriptorPool = CreateDescriptorPool(poolCapacity, poolSizes);
}

void VulkanDeviceObj::LoadDynamicState3Functions() {
    if (IsExtensionSupported(VK_EXT_EXTENDED_DYNAMIC_STATE_3_EXTENSION_NAME)) {
#define LOAD_EXT_FUNC(name)                                                                        \
    m_DynamicState3Functions.name = (PFN_##name)vkGetDeviceProcAddr(m_VkDevice, #name);

        LOAD_EXT_FUNC(vkCmdSetPolygonModeEXT);
        LOAD_EXT_FUNC(vkCmdSetCullModeEXT);
        LOAD_EXT_FUNC(vkCmdSetDepthBoundsTestEnableEXT);
        LOAD_EXT_FUNC(vkCmdSetDepthCompareOpEXT);
        LOAD_EXT_FUNC(vkCmdSetDepthTestEnableEXT);
        LOAD_EXT_FUNC(vkCmdSetDepthWriteEnableEXT);
        LOAD_EXT_FUNC(vkCmdSetFrontFaceEXT);
        LOAD_EXT_FUNC(vkCmdSetPrimitiveRestartEnableEXT);
        LOAD_EXT_FUNC(vkCmdSetPrimitiveTopologyEXT);
        LOAD_EXT_FUNC(vkCmdSetRasterizerDiscardEnableEXT);
        LOAD_EXT_FUNC(vkCmdSetScissorWithCountEXT);
        LOAD_EXT_FUNC(vkCmdSetStencilOpEXT);
        LOAD_EXT_FUNC(vkCmdSetStencilTestEnableEXT);
        LOAD_EXT_FUNC(vkCmdSetViewportWithCountEXT);
        LOAD_EXT_FUNC(vkCmdSetColorBlendEnableEXT);
        LOAD_EXT_FUNC(vkCmdSetColorBlendEquationEXT);
        LOAD_EXT_FUNC(vkCmdSetColorWriteMaskEXT);
        LOAD_EXT_FUNC(vkCmdSetDepthClampEnableEXT);
        LOAD_EXT_FUNC(vkCmdSetLogicOpEXT);
        LOAD_EXT_FUNC(vkCmdSetPatchControlPointsEXT);
        LOAD_EXT_FUNC(vkCmdSetTessellationDomainOriginEXT);

#undef LOAD_EXT_FUNC
    } else {
        CZ_LOG(LogVulkanDevice, Warning, "VK_EXT_extended_dynamic_state_3 not supported");
    }
}

} // namespace CZ

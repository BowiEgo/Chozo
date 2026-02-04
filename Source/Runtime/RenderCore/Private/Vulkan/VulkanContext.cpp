#include "VulkanContext.h"
#include "ShaderManager.h"
#include "VulkanUtils.h"

#include "vulkan/vulkan.hpp"
#include "vulkan/vulkan_raii.hpp"

static VKAPI_ATTR VkBool32 VKAPI_CALL
    DebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                  VkDebugUtilsMessageTypeFlagsEXT messageType,
                  const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
                  void *pUserData) {

    // Log the validation layer message based on its severity
    if (messageSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT) {
        CZ_LOG(LogVulkan, Error, "Validation Layer: {0}",
               pCallbackData->pMessage);
    } else {
        CZ_LOG(LogVulkan, Warning, "Validation Layer: {0}",
               pCallbackData->pMessage);
    }
    return VK_FALSE; // indicates that the Vulkan call that triggered the
                     // validation layer message should not be aborted
}

CVulkanContext::CVulkanContext(IRendererWindow *windowHandle)
    : CGraphicsContext(windowHandle) {
    // Initialize Vulkan with the provided window handle
}

void CVulkanContext::Init() {
    // Set up Vulkan context
    CreateVKInstance();
    SetupDebugMessenger();
    CreateVKSurface();
    PickPhysicalDevice();
    CreateLogicalDevice();
    CreateSwapchain();
}

void CVulkanContext::SwapBuffers() {
    // Present the rendered image to the screen
}

void CVulkanContext::CreateVKInstance() {
    // Check validation layer support
    if (ChozoUtils::Vulkan::EnableValidationLayers &&
        !ChozoUtils::Vulkan::CheckValidationLayerSupport(m_Context)) {
        CZ_LOG(LogVulkanContext, Warning,
               "Validation layers requested, but not available!");
    }

    // Get required extensions from GLFW
    auto extensions = m_Window->GetRequiredExtensions();

    if (ChozoUtils::Vulkan::EnableValidationLayers) {
        extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }

    // Check if the required GLFW extensions are supported by the Vulkan
    // implementation.
    if (!ChozoUtils::Vulkan::CheckInstanceExtensions(m_Context, extensions)) {
        throw std::runtime_error(
            "Required Vulkan extensions are not supported!");
    }

    // Fill in ApplicationInfo and InstanceCreateInfo
    const vk::ApplicationInfo appInfo =
        vk::ApplicationInfo()
            .setPApplicationName("Chozo Engine")
            .setApplicationVersion(VK_MAKE_VERSION(1, 0, 0))
            .setPEngineName("Chozo")
            .setEngineVersion(VK_MAKE_VERSION(1, 0, 0))
            .setApiVersion(vk::ApiVersion14);

    vk::InstanceCreateInfo createInfo =
        vk::InstanceCreateInfo()
            .setPApplicationInfo(&appInfo)
            .setEnabledExtensionCount(static_cast<uint32_t>(extensions.size()))
            .setPpEnabledExtensionNames(extensions.data());

    // Add validation layers if enabled
    if (ChozoUtils::Vulkan::EnableValidationLayers) {
        createInfo
            .setEnabledLayerCount(static_cast<uint32_t>(
                ChozoUtils::Vulkan::ValidationLayers.size()))
            .setPpEnabledLayerNames(
                ChozoUtils::Vulkan::ValidationLayers.data());
    }

    // Create RAII Instance
    try {
        m_Instance = vk::raii::Instance(m_Context, createInfo);
        CZ_LOG(LogVulkanContext, Info, "Vulkan RAII Instance created.");
    } catch (const vk::SystemError &err) {
        CZ_LOG(LogVulkanContext, Fatal, "Vulkan RAII System Error: {0}",
               err.what());
    } catch (const std::exception &e) {
        CZ_LOG(LogVulkanContext, Fatal, "Vulkan RAII Error: {0}", e.what());
    }
}

void CVulkanContext::SetupDebugMessenger() {
    if (!ChozoUtils::Vulkan::EnableValidationLayers)
        return;

    vk::DebugUtilsMessageSeverityFlagsEXT severityFlags(
        vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose |
        vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning |
        vk::DebugUtilsMessageSeverityFlagBitsEXT::eError);

    vk::DebugUtilsMessageTypeFlagsEXT messageTypeFlags(
        vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral |
        vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance |
        vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation);

    vk::DebugUtilsMessengerCreateInfoEXT messengerInfo =
        vk::DebugUtilsMessengerCreateInfoEXT()
            .setMessageSeverity(severityFlags)
            .setMessageType(messageTypeFlags)
            .setPfnUserCallback(
                DebugCallback); // Static method for handling debug messages

    m_DebugMessenger =
        vk::raii::DebugUtilsMessengerEXT(m_Instance, messengerInfo);
}

void CVulkanContext::CreateVKSurface() {
    VkSurfaceKHR surfaceHandle;
    VkResult result;

    auto rawHandle = m_Window->GetNativeHandle();

    try {
#ifdef CHOZO_PLATFORM_WINDOWS
        /* Using Vulkan-Hpp Win32 structure */
        vk::Win32SurfaceCreateInfoKHR createInfo({}, GetModuleHandle(nullptr),
                                                 (HWND)rawHandle);

        // Directly initialize the RAII wrapper
        m_Surface = vk::raii::SurfaceKHR(m_Instance, createInfo);

#elif defined(CHOZO_PLATFORM_LINUX)
        // Implement Xlib/Wayland logic here...
#elif defined(CHOZO_PLATFORM_MACOS)
        // Implement Metal/Cocoa logic here...
#endif

        CZ_LOG(LogVulkanContext, Info, "Vulkan RAII Surface created.");
    } catch (const std::exception &e) {
        CZ_LOG(LogVulkanContext, Fatal, "Failed to create Window Surface: {0}",
               e.what());
    }
}

void CVulkanContext::PickPhysicalDevice() {
    auto devices = m_Instance.enumeratePhysicalDevices();

    if (devices.empty()) {
        CZ_LOG(LogVulkanContext, Fatal,
               "Failed to find GPUs with Vulkan support");
    }

    for (const auto &device : devices) {
        m_PhysicalDevice = device;
        break;
    }

    auto deviceProperties = m_PhysicalDevice.getProperties();
    auto deviceFeatures = m_PhysicalDevice.getFeatures();

    ChozoUtils::Vulkan::LogPhysicalDeviceInfo(deviceProperties);
    ChozoUtils::Vulkan::LogMemoryBudget(m_PhysicalDevice);
}

void CVulkanContext::CreateLogicalDevice() {
    FQueueFamilyIndices indices =
        ChozoUtils::Vulkan::FindQueueFamilies(m_PhysicalDevice, m_Surface);

    std::set<uint32_t> uniqueQueueFamilies = {indices.Graphics.value(),
                                              indices.Present.value(),
                                              indices.Compute.value()};

    std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos;
    float queuePriority = 1.0f; // range is [0.0, 1.0]
    for (uint32_t queueFamily : uniqueQueueFamilies) {
        queueCreateInfos.push_back({{}, queueFamily, 1, &queuePriority});
    }

    // Configure Vulkan 1.3 and Extension features
    // IMPORTANT: Keep these structures in scope until m_LogicalDevice is
    // created
    vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT
        extendedDynamicStateFeatures;
    extendedDynamicStateFeatures.extendedDynamicState = vk::True;

    vk::PhysicalDeviceVulkan13Features vulkan13Features;
    vulkan13Features.dynamicRendering = vk::True;
    vulkan13Features.pNext = &extendedDynamicStateFeatures;

    // Link features to DeviceCreateInfo
    std::vector<const char *> deviceExtensions = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME};
    vk::DeviceCreateInfo deviceCreateInfo({}, queueCreateInfos, {},
                                          deviceExtensions);
    deviceCreateInfo.pNext = &vulkan13Features; // Hook the feature chain here

    // Create the logical device
    m_LogicalDevice = vk::raii::Device(m_PhysicalDevice, deviceCreateInfo);

    // Get queue handles for each operation type
    m_GraphicsQueue =
        vk::raii::Queue(m_LogicalDevice, indices.Graphics.value(), 0);
    m_PresentQueue =
        vk::raii::Queue(m_LogicalDevice, indices.Present.value(), 0);
    m_ComputeQueue =
        vk::raii::Queue(m_LogicalDevice, indices.Compute.value(), 0);

    CZ_LOG(LogVulkanContext, Info,
           "Queue Family Indices -> Graphics: {}, Present: {}, Compute: {}",
           indices.Graphics.value(), indices.Present.value(),
           indices.Compute.value());
}

void CVulkanContext::CreateSwapchain() {
    ChozoUtils::Vulkan::SwapchainSupportDetails details =
        ChozoUtils::Vulkan::QuerySwapchainSupport(m_PhysicalDevice, m_Surface);

    int pixelWidth, pixelHeight;
    m_Window->GetFramebufferSize(&pixelWidth, &pixelHeight);

    vk::SurfaceFormatKHR surfaceFormat =
        ChozoUtils::Vulkan::ChooseSwapSurfaceFormat(details.formats);
    vk::PresentModeKHR presentMode =
        ChozoUtils::Vulkan::ChooseSwapPresentMode(details.presentModes);
    vk::Extent2D extent = ChozoUtils::Vulkan::ChooseSwapExtent(
        details.capabilities, pixelWidth, pixelHeight);

    // Determine image count (Minimum + 1 for triple buffering)
    uint32_t imageCount = details.capabilities.minImageCount + 1;
    if (details.capabilities.maxImageCount > 0 &&
        imageCount > details.capabilities.maxImageCount) {
        imageCount = details.capabilities.maxImageCount;
    }

    vk::SwapchainCreateInfoKHR createInfo;
    createInfo.surface = *m_Surface;
    createInfo.minImageCount = imageCount;
    createInfo.imageFormat = surfaceFormat.format;
    createInfo.imageColorSpace = surfaceFormat.colorSpace;
    createInfo.imageExtent = extent;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = vk::ImageUsageFlagBits::eColorAttachment;

    // If indices are different, use Concurrent mode; otherwise use Exclusive
    FQueueFamilyIndices indices =
        ChozoUtils::Vulkan::FindQueueFamilies(m_PhysicalDevice, m_Surface);
    uint32_t queueFamilyIndices[] = {indices.Graphics.value(),
                                     indices.Present.value()};
    if (indices.Graphics != indices.Present) {
        createInfo.imageSharingMode = vk::SharingMode::eConcurrent;
        createInfo.queueFamilyIndexCount = 2;
        createInfo.pQueueFamilyIndices = queueFamilyIndices;
    } else {
        createInfo.imageSharingMode = vk::SharingMode::eExclusive;
    }

    // Use the current transform of the surface to avoid unintended rotation
    if (details.capabilities.supportedTransforms &
        vk::SurfaceTransformFlagBitsKHR::eIdentity) {
        createInfo.preTransform = vk::SurfaceTransformFlagBitsKHR::eIdentity;
    } else {
        createInfo.preTransform = details.capabilities.currentTransform;
    }

    createInfo.compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque;
    createInfo.presentMode = presentMode;
    createInfo.clipped = true;
    createInfo.oldSwapchain = nullptr;

    m_Swapchain = vk::raii::SwapchainKHR(m_LogicalDevice, createInfo);

    // Retrieve the images created by the swapchain
    m_SwapchainImages = m_Swapchain.getImages();
    m_SwapchainImageFormat = surfaceFormat.format;
    m_SwapchainExtent = extent;
}

void CVulkanContext::CreateImageViews() {
    m_SwapchainImageViews.clear();

    vk::ImageViewCreateInfo createInfo;
    createInfo.viewType = vk::ImageViewType::e2D;
    createInfo.format = m_SwapchainImageFormat;

    createInfo.components.r = vk::ComponentSwizzle::eIdentity;
    createInfo.components.g = vk::ComponentSwizzle::eIdentity;
    createInfo.components.b = vk::ComponentSwizzle::eIdentity;
    createInfo.components.a = vk::ComponentSwizzle::eIdentity;

    createInfo.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
    createInfo.subresourceRange.baseMipLevel = 0;
    createInfo.subresourceRange.levelCount = 1;
    createInfo.subresourceRange.baseArrayLayer = 0;
    createInfo.subresourceRange.layerCount = 1;

    for (auto image : m_SwapchainImages) {
        createInfo.image = image;
        m_SwapchainImageViews.emplace_back(m_LogicalDevice, createInfo);
    }
}

// Consolidates all state into a single immutable pipeline object
void CVulkanContext::CreateGraphicsPipeline() {
    TRef<CShader> myShader = CShaderManager::Get()->Load(
        FShaderCreateInfo("Test", "shaders://Test.glsl"));
    // // 1. Load Shaders (Assuming you used the automated build we discussed)
    // auto vertShaderCode = ReadShaderFile("shaders/simple.vert.spv");
    // auto fragShaderCode = ReadShaderFile("shaders/simple.frag.spv");

    // vk::raii::ShaderModule vertModule =
    //     CreateShaderModule(myShader->GetVertSpv());
    // vk::raii::ShaderModule fragModule =
    //     CreateShaderModule(myShader->GetVertSpv());

    // // 2. Shader Stages
    // vk::PipelineShaderStageCreateInfo shaderStages[] = {
    //     {{}, vk::ShaderStageFlagBits::eVertex, *vertModule, "main"},
    //     {{}, vk::ShaderStageFlagBits::eFragment, *fragModule, "main"}};

    // auto shader =

    //     // 3. Vertex Input (Empty for hardcoded triangle)
    //     vk::PipelineVertexInputStateCreateInfo vertexInputInfo{};

    // // 4. Input Assembly
    // vk::PipelineInputAssemblyStateCreateInfo inputAssembly(
    //     {}, vk::PrimitiveTopology::eTriangleList, vk::False);

    // // 5. Viewport & Scissor (Setup as Dynamic States)
    // vk::PipelineViewportStateCreateInfo viewportState({}, 1, nullptr, 1,
    //                                                   nullptr);

    // // 6. Rasterizer
    // vk::PipelineRasterizationStateCreateInfo rasterizer{};
    // rasterizer.setPolygonMode(vk::PolygonMode::eFill);
    // rasterizer.setLineWidth(1.0f);
    // rasterizer.setCullMode(vk::CullModeFlagBits::eBack);
    // rasterizer.setFrontFace(vk::FrontFace::eClockwise);

    // // 7. Multisampling (Disabled)
    // vk::PipelineMultisampleStateCreateInfo multisampling(
    //     {}, vk::SampleCountFlagBits::e1);

    // // 8. Color Blending (Standard opaque)
    // vk::PipelineColorBlendAttachmentState colorBlendAttachment{};
    // colorBlendAttachment.setColorWriteMask(
    //     vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG |
    //     vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA);
    // colorBlendAttachment.setBlendEnable(vk::False);

    // vk::PipelineColorBlendStateCreateInfo colorBlending(
    //     {}, vk::False, vk::LogicOp::eCopy, 1, &colorBlendAttachment);

    // // 9. Dynamic States
    // std::vector<vk::DynamicState> dynamicStates =
    // {vk::DynamicState::eViewport,
    //                                                vk::DynamicState::eScissor};
    // vk::PipelineDynamicStateCreateInfo dynamicStateInfo({}, dynamicStates);

    // // 10. Final Assembly
    // vk::GraphicsPipelineCreateInfo pipelineInfo{};
    // pipelineInfo.setStages(shaderStages);
    // pipelineInfo.setPVertexInputState(&vertexInputInfo);
    // pipelineInfo.setPInputAssemblyState(&inputAssembly);
    // pipelineInfo.setPViewportState(&viewportState);
    // pipelineInfo.setPRasterizationState(&rasterizer);
    // pipelineInfo.setPMultisampleState(&multisampling);
    // pipelineInfo.setPColorBlendState(&colorBlending);
    // pipelineInfo.setPDynamicState(&dynamicStateInfo);
    // pipelineInfo.setLayout(*m_PipelineLayout);
    // pipelineInfo.setRenderPass(*m_RenderPass);
    // pipelineInfo.setSubpass(0);

    // // Create the monolithic pipeline object
    // m_GraphicsPipeline =
    //     m_LogicalDevice.createGraphicsPipeline(nullptr, pipelineInfo);
}
void CVulkanContext::CreateCommandPool() {}
void CVulkanContext::CreateVertexBuffer() {}
void CVulkanContext::CreateCommandBuffers() {}
void CVulkanContext::CreateSyncObjects() {}
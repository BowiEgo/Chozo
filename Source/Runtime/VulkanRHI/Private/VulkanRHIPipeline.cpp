#include "VulkanRHIPipeline.h"

#include "VulkanRHIDevice.h"
#include "VulkanRHIShader.h"
#include "VulkanUtils.h"

CVulkanRHIPipeline::CVulkanRHIPipeline(const FRHIPipelineCreateInfo& info,
                                       const TRef<CVulkanRHIDevice> device)
    : IRHIPipeline(info), m_Device(device) {
    Init();
}

void CVulkanRHIPipeline::Init() {
    // Load ShaderModule
    // vk::raii::ShaderModule vertModule =
    //     CreateShaderModule(m_Info.Shader->GetVertSpv());
    // vk::raii::ShaderModule fragModule =
    //     CreateShaderModule(m_Info.Shader->GetFragSpv());

    // Shader Stages
    std::vector<vk::PipelineShaderStageCreateInfo> shaderStages;
    shaderStages.reserve(m_Info.RHIShaders.size());

    for (auto RHIShader : m_Info.RHIShaders) {
        TRef<CVulkanRHIShader> vulkanShader = RHIShader.As<CVulkanRHIShader>();

        shaderStages.push_back(
            {{},
             ChozoUtils::Vulkan::StageToFlagBits(RHIShader->GetStage()),
             vulkanShader->GetModule(),
             RHIShader->GetEntryPoint().c_str()});
    }

    // Vertex Input
    vk::VertexInputBindingDescription bindingDescription(
        0,                 // binding index
        sizeof(float) * 6, // stride (example: vec3 pos + vec3 color)
        vk::VertexInputRate::eVertex);

    std::vector<vk::VertexInputAttributeDescription> attributeDescriptions = {
        // Location 0: Position (vec3)
        {0, 0, vk::Format::eR32G32B32Sfloat, 0},
        // Location 1: Color/Extra (vec3)
        {1, 0, vk::Format::eR32G32B32Sfloat, sizeof(float) * 3}};

    vk::PipelineVertexInputStateCreateInfo vertexInputInfo(
        {}, 1, &bindingDescription,                          // Bindings
        static_cast<uint32_t>(attributeDescriptions.size()), // Attributes
        attributeDescriptions.data());

    // Input Assembly
    vk::PipelineInputAssemblyStateCreateInfo inputAssembly(
        {}, vk::PrimitiveTopology::eTriangleList, vk::False);

    // Viewport & Scissor (Setup as Dynamic States)
    vk::PipelineViewportStateCreateInfo viewportState({}, 1, nullptr, 1,
                                                      nullptr);

    // Rasterizer
    vk::PipelineRasterizationStateCreateInfo rasterizer{};
    rasterizer.setPolygonMode(vk::PolygonMode::eFill);
    rasterizer.setLineWidth(1.0f);
    rasterizer.setCullMode(vk::CullModeFlagBits::eBack);
    rasterizer.setFrontFace(vk::FrontFace::eClockwise);

    // Multisampling (Disabled)
    vk::PipelineMultisampleStateCreateInfo multisampling(
        {}, vk::SampleCountFlagBits::e1);

    // Color Blending (Standard opaque)
    vk::PipelineColorBlendAttachmentState colorBlendAttachment{};
    colorBlendAttachment.setColorWriteMask(
        vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG |
        vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA);
    colorBlendAttachment.setBlendEnable(vk::False);

    vk::PipelineColorBlendStateCreateInfo colorBlending(
        {}, vk::False, vk::LogicOp::eCopy, 1, &colorBlendAttachment);

    // Dynamic States
    std::vector<vk::DynamicState> dynamicStates = {vk::DynamicState::eViewport,
                                                   vk::DynamicState::eScissor};
    vk::PipelineDynamicStateCreateInfo dynamicStateInfo({}, dynamicStates);

    // Rendering
    // 3. Dynamic Rendering Setup (Vulkan 1.3+)
    std::vector<vk::Format> vkColorFormats;
    for (auto f : m_Info.ColorFormats) {
        vkColorFormats.push_back(ChozoUtils::Vulkan::ToVulkanFormat(f));
    }
    vk::Format vkDepthFormat =
        ChozoUtils::Vulkan::ToVulkanFormat(m_Info.DepthFormat);
    vk::PipelineDepthStencilStateCreateInfo depthStencil({}, vk::True, vk::True,
                                                         vk::CompareOp::eLess);
    vk::PipelineRenderingCreateInfo renderingInfo(0, vkColorFormats,
                                                  vkDepthFormat);

    // Pipeline Layout
    vk::PipelineLayoutCreateInfo pipelineLayoutInfo(
        {},      // flags
        0,       // setLayoutCount
        nullptr, // pSetLayouts
        0,       // pushConstantRangeCount
        nullptr  // pPushConstantRanges
    );

    m_PipelineLayout = vk::raii::PipelineLayout(m_Device->GetVKLogicalDevice(),
                                                pipelineLayoutInfo);

    // Final Assembly
    vk::GraphicsPipelineCreateInfo pipelineInfo{};
    pipelineInfo
        .setPNext(&renderingInfo) // [Note] Link to dynamic rendering info
        .setPDepthStencilState(&depthStencil)
        .setStages(shaderStages)
        .setPVertexInputState(&vertexInputInfo)
        .setPInputAssemblyState(&inputAssembly)
        .setPViewportState(&viewportState)
        .setPRasterizationState(&rasterizer)
        .setPMultisampleState(&multisampling)
        .setPColorBlendState(&colorBlending)
        .setPDynamicState(&dynamicStateInfo)
        .setLayout(*m_PipelineLayout) // [Note] Use * for RAII objects
        .setRenderPass(
            nullptr); // [Note] Must be null when using pNext renderingInfo

    // Create the monolithic pipeline object
    m_GraphicsPipeline = m_Device->GetVKLogicalDevice().createGraphicsPipeline(
        nullptr, pipelineInfo);

    CZ_LOG(LogVulkanRHIPipeline, Info, "Vulkan Pipeline created");
}
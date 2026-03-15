#include "VulkanPipeline.h"

#include "VulkanDevice.h"
#include "VulkanShader.h"
#include "VulkanUtils.h"

DEFINE_LOG_CATEGORY(LogVulkanPipeline);

CVulkanPipeline::CVulkanPipeline(const FPipelineSpecification& spec,
                                 const TRef<CVulkanDevice>& device)
    : IRHIPipeline(spec), m_Device(device) {
    Init();
}

CVulkanPipeline::~CVulkanPipeline() {
    CZ_LOG(LogVulkanPipeline, Trace, "VulkanPipeline destroying...");
}

void CVulkanPipeline::Init() {
    auto device = m_Device.lock();
    if (!device) {
        CZ_LOG(LogVulkanPipeline, Error, "Device is no longer valid during Pipeline creation!");
        return;
    }

    const vk::raii::Device& raiiDevice = device->GetRAIILogicalDevice();

    // ===== 1. Get Descriptor Set Layouts =====
    std::vector<vk::DescriptorSetLayout> descriptorSetLayouts;

    // Get Uniform Buffer Layout (set 0)
    vk::DescriptorSetLayout uniformLayout =
        device->GetDescriptorSetLayout(EDescriptorLayoutType::UniformBuffer);
    descriptorSetLayouts.push_back(uniformLayout);

    // If have another set，keep on pushing
    // descriptorSetLayouts.push_back(anotherLayout);

    // ===== 2. Shader Stages =====
    std::vector<vk::PipelineShaderStageCreateInfo> shaderStages;
    shaderStages.reserve(m_Spec.RHIShaders.size());

    for (auto RHIShader : m_Spec.RHIShaders) {
        TRef<CVulkanShader> vulkanShader = RHIShader.As<CVulkanShader>();

        shaderStages.push_back({ {},
                                 ChozoUtils::Vulkan::StageToFlagBits(RHIShader->GetStage()),
                                 vulkanShader->GetModule(),
                                 RHIShader->GetEntryPoint().c_str() });
    }

    // ===== 3. Vertex Input =====
    // vk::VertexInputBindingDescription bindingDescription(
    //     0,                 // binding index
    //     sizeof(float) * 6, // stride (example: vec3 pos + vec3 color)
    //     vk::VertexInputRate::eVertex);

    // std::vector<vk::VertexInputAttributeDescription> attributeDescriptions = {
    //     // Location 0: Position (vec3)
    //     {0, 0, vk::Format::eR32G32B32Sfloat, 0},
    //     // Location 1: Color/Extra (vec3)
    //     {1, 0, vk::Format::eR32G32B32Sfloat, sizeof(float) * 3}};

    // vk::PipelineVertexInputStateCreateInfo vertexInputInfo(
    //     {}, 1, &bindingDescription,                          // Bindings
    //     static_cast<uint32_t>(attributeDescriptions.size()), // Attributes
    //     attributeDescriptions.data());

    vk::PipelineVertexInputStateCreateInfo vertexInputInfo;
    vertexInputInfo.vertexBindingDescriptionCount = 0;
    vertexInputInfo.pVertexBindingDescriptions = nullptr;
    vertexInputInfo.vertexAttributeDescriptionCount = 0;
    vertexInputInfo.pVertexAttributeDescriptions = nullptr;

    // ===== 4. Input Assembly =====
    vk::PipelineInputAssemblyStateCreateInfo inputAssembly({}, vk::PrimitiveTopology::eTriangleList,
                                                           vk::False);

    // ===== 5. Viewport & Scissor (Dynamic States) =====
    vk::PipelineViewportStateCreateInfo viewportState({}, 1, nullptr, 1, nullptr);

    // ===== 6. Rasterizer =====
    vk::PipelineRasterizationStateCreateInfo rasterizer{};
    rasterizer.setPolygonMode(vk::PolygonMode::eFill);
    rasterizer.setLineWidth(1.0f);
    rasterizer.setCullMode(vk::CullModeFlagBits::eBack);
    rasterizer.setFrontFace(vk::FrontFace::eClockwise);

    // ===== 7. Multisampling (Disabled) =====
    vk::PipelineMultisampleStateCreateInfo multisampling({}, vk::SampleCountFlagBits::e1);

    // ===== 8. Depth Stencil =====
    vk::Format vkDepthFormat = ChozoUtils::Vulkan::ToVKFormat(m_Spec.DepthFormat);
    vk::PipelineDepthStencilStateCreateInfo depthStencil({}, vk::True, vk::True,
                                                         vk::CompareOp::eLess);

    // ===== 9. Color Blending (Standard opaque) =====
    vk::PipelineColorBlendAttachmentState colorBlendAttachment{};
    colorBlendAttachment.setColorWriteMask(
        vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG |
        vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA);
    colorBlendAttachment.setBlendEnable(vk::False);

    vk::PipelineColorBlendStateCreateInfo colorBlending({}, vk::False, vk::LogicOp::eCopy, 1,
                                                        &colorBlendAttachment);

    // ===== 10. Dynamic States =====
    std::vector<vk::DynamicState> dynamicStates = { vk::DynamicState::eViewport,
                                                    vk::DynamicState::eScissor };
    vk::PipelineDynamicStateCreateInfo dynamicStateInfo({}, dynamicStates);

    // ===== 11. Dynamic Rendering Setup (Vulkan 1.3+) =====
    std::vector<vk::Format> vkColorFormats;
    for (auto f : m_Spec.ColorFormats) {
        vkColorFormats.push_back(ChozoUtils::Vulkan::ToVKFormat(f));
    }
    vk::PipelineRenderingCreateInfo renderingInfo(0, vkColorFormats, vkDepthFormat);

    // ===== 12. Pipeline Layout =====
    vk::PipelineLayoutCreateInfo pipelineLayoutInfo(
        {},                                                 // flags
        static_cast<uint32_t>(descriptorSetLayouts.size()), // setLayoutCount
        descriptorSetLayouts.data(),                        // pSetLayouts
        0,                                                  // pushConstantRangeCount
        nullptr                                             // pPushConstantRanges
    );
    m_PipelineLayout = vk::raii::PipelineLayout(raiiDevice, pipelineLayoutInfo);

    // ===== 13. Final Assembly =====
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
        .setRenderPass(nullptr);      // [Note] Must be null when using pNext renderingInfo

    // ===== 14. Create Pipeline =====
    m_Pipeline = raiiDevice.createGraphicsPipeline(nullptr, pipelineInfo);

    m_Spec.RHIShaders.clear();

    CZ_LOG(LogVulkanPipeline, Info, "Vulkan Pipeline created with {} descriptor set layouts",
           descriptorSetLayouts.size());
}

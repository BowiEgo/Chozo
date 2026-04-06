#include "VulkanPipeline.h"

#include "VulkanDevice.h"
#include "VulkanSetLayout.h"
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

    FRHIPipelineLayoutDescription pipelineLayoutDesc =
        ChozoUtils::RHI::GeneratePipelineLayoutDesc(m_Spec.RHIShaders);

    auto rhiLayouts = device->CreateDescriptorSetLayout(pipelineLayoutDesc);

    m_DescriptorSetLayouts.clear();
    m_DescriptorSetLayouts.reserve(rhiLayouts.size());

    for (const auto& rhiLayout : rhiLayouts) {
        m_DescriptorSetLayouts.push_back(rhiLayout.As<CVulkanSetLayout>()->GetVKSetLayout());
    }

    // ===== Push Constant Range =====
    std::vector<vk::PushConstantRange> pushConstantRanges;
    vk::PushConstantRange vertPushRange(vk::ShaderStageFlagBits::eVertex,    // stageFlags
                                        m_Spec.PushConstantRanges[0].Offset, // offset
                                        m_Spec.PushConstantRanges[0].Size    // size
    );
    pushConstantRanges.push_back(vertPushRange);

    // ===== Shader Stages =====
    std::vector<vk::PipelineShaderStageCreateInfo> shaderStages;
    shaderStages.reserve(m_Spec.RHIShaders.size());

    for (auto RHIShader : m_Spec.RHIShaders) {
        TRef<CVulkanShader> vulkanShader = RHIShader.As<CVulkanShader>();

        shaderStages.push_back({ {},
                                 ChozoUtils::Vulkan::StageToFlagBits(RHIShader->GetStage()),
                                 vulkanShader->GetModule(),
                                 RHIShader->GetEntryPoint().c_str() });
    }

    // ===== Pipeline Layout =====
    vk::PipelineLayoutCreateInfo pipelineLayoutInfo({}, m_DescriptorSetLayouts, pushConstantRanges);
    m_PipelineLayout = vk::raii::PipelineLayout(raiiDevice, pipelineLayoutInfo);

    // ===== Vertex Input =====
    std::vector<vk::VertexInputBindingDescription> bindingDescs;
    std::vector<vk::VertexInputAttributeDescription> attributeDescs;

    if (m_Spec.VertexLayout.GetElements().size() > 0) {
        vk::VertexInputBindingDescription bindingDesc;
        bindingDesc.setBinding(0)
            .setStride(m_Spec.VertexLayout.GetStride())
            .setInputRate(vk::VertexInputRate::eVertex);
        bindingDescs.push_back(bindingDesc);

        uint32_t location = 0;
        for (const auto& element : m_Spec.VertexLayout) {
            vk::VertexInputAttributeDescription attrDesc;
            attrDesc.setLocation(location++)
                .setBinding(0)
                .setFormat(ChozoUtils::Vulkan::ShaderDataTypeToVkFormat(element.Type))
                .setOffset(element.Offset);
            attributeDescs.push_back(attrDesc);
        }
    }

    vk::PipelineVertexInputStateCreateInfo vertexInputInfo;
    vertexInputInfo.setVertexBindingDescriptionCount(bindingDescs.size())
        .setPVertexBindingDescriptions(bindingDescs.data())
        .setVertexAttributeDescriptionCount(attributeDescs.size())
        .setPVertexAttributeDescriptions(attributeDescs.data());

    // ===== Input Assembly =====
    vk::PipelineInputAssemblyStateCreateInfo inputAssembly({}, vk::PrimitiveTopology::eTriangleList,
                                                           vk::False);

    // ===== Viewport & Scissor (Dynamic States) =====
    vk::PipelineViewportStateCreateInfo viewportState({}, 1, nullptr, 1, nullptr);

    // ===== Rasterizer =====
    vk::PipelineRasterizationStateCreateInfo rasterizer{};
    rasterizer.setPolygonMode(vk::PolygonMode::eFill);
    rasterizer.setLineWidth(1.0f);
    rasterizer.setCullMode(vk::CullModeFlagBits::eBack);
    rasterizer.setFrontFace(vk::FrontFace::eClockwise);

    // ===== Multisampling (Disabled) =====
    vk::PipelineMultisampleStateCreateInfo multisampling({}, vk::SampleCountFlagBits::e1);

    // ===== Depth Stencil =====
    vk::Format vkDepthFormat = ChozoUtils::Vulkan::ToVkFormat(m_Spec.DepthFormat);
    vk::PipelineDepthStencilStateCreateInfo depthStencil({}, vk::True, vk::True,
                                                         vk::CompareOp::eLess);

    // ===== Color Blending (Standard opaque) =====
    vk::PipelineColorBlendAttachmentState colorBlendAttachment{};
    colorBlendAttachment.setColorWriteMask(
        vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG |
        vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA);
    colorBlendAttachment.setBlendEnable(vk::False);

    vk::PipelineColorBlendStateCreateInfo colorBlending({}, vk::False, vk::LogicOp::eCopy, 1,
                                                        &colorBlendAttachment);

    // ===== Dynamic States =====
    std::vector<vk::DynamicState> dynamicStates = { vk::DynamicState::eViewport,
                                                    vk::DynamicState::eScissor,
                                                    vk::DynamicState::ePolygonModeEXT };
    vk::PipelineDynamicStateCreateInfo dynamicStateInfo({}, dynamicStates);

    // ===== Dynamic Rendering Setup (Vulkan 1.3+) =====
    std::vector<vk::Format> vkColorFormats;
    for (auto f : m_Spec.ColorFormats) {
        vkColorFormats.push_back(ChozoUtils::Vulkan::ToVkFormat(f));
    }
    vk::PipelineRenderingCreateInfo renderingInfo(0, vkColorFormats, vkDepthFormat);

    // ===== Final Assembly =====
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

    // ===== Create Pipeline =====
    m_Pipeline = raiiDevice.createGraphicsPipeline(nullptr, pipelineInfo);

    m_Spec.RHIShaders.clear();

    CZ_LOG(LogVulkanPipeline, Info, "Vulkan Pipeline created with {} descriptor set layouts",
           m_DescriptorSetLayouts.size());
}

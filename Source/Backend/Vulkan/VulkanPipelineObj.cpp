#include "VulkanPipelineObj.hpp"
#include "VulkanDeviceObj.hpp"
#include "VulkanSetLayoutObj.hpp"
#include "VulkanShaderResObj.hpp"
#include "VulkanUtils.hpp"

namespace CZ {

VulkanPipelineObj::VulkanPipelineObj(VulkanDeviceObj* deviceObj, const PipelineSpecification& spec)
    : PipelineObj(spec), m_DeviceObj(deviceObj) {}

VulkanPipelineObj::~VulkanPipelineObj() {
    CZ_CORE_LOG(Trace, "VulkanPipeline destroying...");
    if (m_VkPipeline) {
        vkDestroyPipeline(m_DeviceObj->GetLogicalDevice(), m_VkPipeline, nullptr);
        m_VkPipeline = VK_NULL_HANDLE;
    }
    if (m_VkPipelineLayout) {
        vkDestroyPipelineLayout(m_DeviceObj->GetLogicalDevice(), m_VkPipelineLayout, nullptr);
        m_VkPipelineLayout = VK_NULL_HANDLE;
    }
}

VkResult VulkanPipelineObj::Init(const std::vector<ShaderRes>& shaders,
                                 const ShaderReflection& reflection) {
    VkResult result;
    VkDevice logicalDevice = m_DeviceObj->GetLogicalDevice();

    if (!logicalDevice) {
        CZ_CORE_LOG(Error, "Logical device is not valid during pipeline initialization!");
        return VK_ERROR_INITIALIZATION_FAILED;
    }

    // ===== Push Constants =====
    VkPushConstantRange* pushConstantRanges = NULL;
    uint32_t pushConstantRangeCount         = 0;
    if (reflection.PushConstants.size() > 0) {
        pushConstantRangeCount = reflection.PushConstants.size();
        pushConstantRanges =
            (VkPushConstantRange*)malloc(pushConstantRangeCount * sizeof(VkPushConstantRange));
        for (uint32_t i = 0; i < pushConstantRangeCount; i++) {
            pushConstantRanges[i].stageFlags =
                VulkanUtils::StageToFlagBits(reflection.PushConstants[i].StageFlags);
            pushConstantRanges[i].offset = reflection.PushConstants[i].Offset;
            pushConstantRanges[i].size   = reflection.PushConstants[i].Size;
        }
    }

    // ===== Descriptor Set Layouts =====
    m_SetLayouts = m_DeviceObj->CreateSetLayouts(reflection.ResourceBindings);
    if (m_SetLayouts.empty()) {
        free(pushConstantRanges);
        CZ_CORE_LOG(Error,
                    "Failed to create pipeline: No set layouts created from shader reflection");
        return VK_ERROR_INITIALIZATION_FAILED;
    }
    std::vector<VkDescriptorSetLayout> vkSetLayouts(m_SetLayouts.size());
    for (size_t i = 0; i < m_SetLayouts.size(); i++) {
        vkSetLayouts[i] = m_SetLayouts[i].As<VulkanSetLayoutObj>()->GetVkSetLayout();
    }

    // ===== Pipeline Layout =====
    VkPipelineLayoutCreateInfo pipelineLayoutInfo = {
        .sType                  = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
        .pNext                  = NULL,
        .flags                  = 0,
        .setLayoutCount         = (uint32_t)m_SetLayouts.size(),
        .pSetLayouts            = vkSetLayouts.data(),
        .pushConstantRangeCount = pushConstantRangeCount,
        .pPushConstantRanges    = pushConstantRanges
    };

    result = vkCreatePipelineLayout(logicalDevice, &pipelineLayoutInfo, NULL, &m_VkPipelineLayout);
    free(pushConstantRanges);
    if (result != VK_SUCCESS) {
        CZ_CORE_LOG(Error, "Failed to create pipeline layout");
        return result;
    }

    // ===== Shader Stages =====
    VkPipelineShaderStageCreateInfo* shaderStages = NULL;
    uint32_t stageCount                           = 0;
    if (shaders.size() > 0) {
        stageCount   = shaders.size();
        shaderStages = (VkPipelineShaderStageCreateInfo*)malloc(
            stageCount * sizeof(VkPipelineShaderStageCreateInfo));
        for (uint32_t i = 0; i < stageCount; i++) {
            shaderStages[i] = (VkPipelineShaderStageCreateInfo){
                .sType               = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
                .pNext               = NULL,
                .flags               = 0,
                .stage               = shaders[i].As<VulkanShaderResObj>()->GetVkStage(),
                .module              = shaders[i].As<VulkanShaderResObj>()->GetVkShaderModule(),
                .pName               = shaders[i]->GetEntryPoint().c_str(),
                .pSpecializationInfo = NULL
            };
        }
    }

    // ===== Vertex Input =====
    VkVertexInputBindingDescription* bindingDescs     = NULL;
    VkVertexInputAttributeDescription* attributeDescs = NULL;
    uint32_t bindingCount                             = 0;
    uint32_t attributeCount                           = 0;

    if (!reflection.VertexBufferLayout.GetElements().empty()) {
        bindingCount = 1;
        bindingDescs =
            (VkVertexInputBindingDescription*)malloc(sizeof(VkVertexInputBindingDescription));
        bindingDescs[0] =
            (VkVertexInputBindingDescription){ .binding = 0,
                                               .stride  = reflection.VertexBufferLayout.GetStride(),
                                               .inputRate = VK_VERTEX_INPUT_RATE_VERTEX };

        attributeCount = reflection.VertexBufferLayout.GetElements().size();
        attributeDescs = (VkVertexInputAttributeDescription*)malloc(
            attributeCount * sizeof(VkVertexInputAttributeDescription));
        for (uint32_t i = 0; i < attributeCount; i++) {
            const auto& elem  = reflection.VertexBufferLayout.GetElements()[i];
            attributeDescs[i] = (VkVertexInputAttributeDescription){
                .location = elem.Location,
                .binding  = 0, // Assuming Offset is used as binding
                .format   = VulkanUtils::ShaderDataTypeToVkFormat(
                    elem.Type), // Assuming Type is used as format
                .offset = elem.Offset
            };
        }
    }

    VkPipelineVertexInputStateCreateInfo vertexInputInfo = {
        .sType                         = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
        .pNext                         = NULL,
        .flags                         = 0,
        .vertexBindingDescriptionCount = bindingCount,
        .pVertexBindingDescriptions    = bindingDescs,
        .vertexAttributeDescriptionCount = attributeCount,
        .pVertexAttributeDescriptions    = attributeDescs
    };

    // ===== Input Assembly =====
    VkPipelineInputAssemblyStateCreateInfo inputAssembly = {
        .sType                  = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
        .pNext                  = NULL,
        .flags                  = 0,
        .topology               = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
        .primitiveRestartEnable = VK_FALSE
    };

    // ===== Viewport and Scissor (Dynamic States) =====
    VkPipelineViewportStateCreateInfo viewportState = {
        .sType         = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
        .pNext         = NULL,
        .flags         = 0,
        .viewportCount = 1,
        .pViewports    = NULL,
        .scissorCount  = 1,
        .pScissors     = NULL
    };

    // ===== Rasterizer =====
    VkPipelineRasterizationStateCreateInfo rasterizer = {
        .sType                   = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
        .pNext                   = NULL,
        .flags                   = 0,
        .depthClampEnable        = VK_FALSE,
        .rasterizerDiscardEnable = VK_FALSE,
        .polygonMode             = VK_POLYGON_MODE_FILL,
        .cullMode                = VulkanUtils::ToVkCullMode(m_Spec.CullMode),
        .frontFace               = VK_FRONT_FACE_CLOCKWISE,
        .depthBiasEnable         = VK_FALSE,
        .depthBiasConstantFactor = 0.0f,
        .depthBiasClamp          = 0.0f,
        .depthBiasSlopeFactor    = 0.0f,
        .lineWidth               = 1.0f
    };

    // ===== Multisampling (Disabled) =====
    VkPipelineMultisampleStateCreateInfo multisampling = {
        .sType                 = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
        .pNext                 = NULL,
        .flags                 = 0,
        .rasterizationSamples  = VK_SAMPLE_COUNT_1_BIT,
        .sampleShadingEnable   = VK_FALSE,
        .minSampleShading      = 1.0f,
        .pSampleMask           = NULL,
        .alphaToCoverageEnable = VK_FALSE,
        .alphaToOneEnable      = VK_FALSE
    };

    // ===== Depth Stencil =====
    VkPipelineDepthStencilStateCreateInfo depthStencil = {
        .sType                 = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
        .pNext                 = NULL,
        .flags                 = 0,
        .depthTestEnable       = m_Spec.bDepthTestEnable,
        .depthWriteEnable      = m_Spec.bDepthWriteEnable,
        .depthCompareOp        = VulkanUtils::ToVkCompareOp(m_Spec.DepthCompareOp),
        .depthBoundsTestEnable = VK_FALSE,
        .stencilTestEnable     = VK_FALSE,
        .front = {
            VK_STENCIL_OP_KEEP,      // failOp
            VK_STENCIL_OP_KEEP,      // passOp
            VK_STENCIL_OP_KEEP,      // depthFailOp
            VK_COMPARE_OP_NEVER,     // compareOp
            0,                       // compareMask
            0,                       // writeMask
            0                        // reference
        },
        .back = {
            VK_STENCIL_OP_KEEP,
            VK_STENCIL_OP_KEEP,
            VK_STENCIL_OP_KEEP,
            VK_COMPARE_OP_NEVER,
            0, 0, 0
        },
        .minDepthBounds        = 0.0f,
        .maxDepthBounds        = 1.0f
    };

    // ===== Color Blending (Standard opaque) =====
    VkPipelineColorBlendAttachmentState colorBlendAttachment = {
        .blendEnable         = VK_FALSE,
        .srcColorBlendFactor = VK_BLEND_FACTOR_ONE,
        .dstColorBlendFactor = VK_BLEND_FACTOR_ZERO,
        .colorBlendOp        = VK_BLEND_OP_ADD,
        .srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE,
        .dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO,
        .alphaBlendOp        = VK_BLEND_OP_ADD,
        .colorWriteMask      = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
                               VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT
    };

    VkPipelineColorBlendStateCreateInfo colorBlending = {
        .sType           = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
        .pNext           = NULL,
        .flags           = 0,
        .logicOpEnable   = VK_FALSE,
        .logicOp         = VK_LOGIC_OP_COPY,
        .attachmentCount = 1,
        .pAttachments    = &colorBlendAttachment,
        .blendConstants  = { 0.0f, 0.0f, 0.0f, 0.0f }
    };

    // ===== Dynamic States =====
    VkDynamicState dynamicStates[] = {
        VK_DYNAMIC_STATE_VIEWPORT,
        VK_DYNAMIC_STATE_SCISSOR,
        // VK_DYNAMIC_STATE_POLYGON_MODE_EXT
    };
    VkPipelineDynamicStateCreateInfo dynamicStateInfo = {
        .sType             = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
        .pNext             = NULL,
        .flags             = 0,
        .dynamicStateCount = sizeof(dynamicStates) / sizeof(dynamicStates[0]),
        .pDynamicStates    = dynamicStates
    };

    // ===== Dynamic Rendering Setup (Vulkan 1.3+) =====
    VkFormat* colorFormats        = NULL;
    uint32_t colorAttachmentCount = m_Spec.ColorFormats.size();
    if (colorAttachmentCount > 0) {
        colorFormats = (VkFormat*)malloc(colorAttachmentCount * sizeof(VkFormat));

        for (uint32_t i = 0; i < colorAttachmentCount; i++) {
            colorFormats[i] = VulkanUtils::ToVkFormat(m_Spec.ColorFormats[i]);
        }
    }
    VkPipelineRenderingCreateInfo renderingInfo = {
        .sType                   = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO,
        .pNext                   = NULL,
        .viewMask                = 0,
        .colorAttachmentCount    = colorAttachmentCount,
        .pColorAttachmentFormats = colorFormats,
        .depthAttachmentFormat   = VulkanUtils::ToVkFormat(m_Spec.DepthFormat),
        .stencilAttachmentFormat = VK_FORMAT_UNDEFINED
    };

    // ===== Final Assembly =====
    VkGraphicsPipelineCreateInfo pipelineInfo = {
        .sType               = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
        .pNext               = &renderingInfo,
        .flags               = 0,
        .stageCount          = stageCount,
        .pStages             = shaderStages,
        .pVertexInputState   = &vertexInputInfo,
        .pInputAssemblyState = &inputAssembly,
        .pTessellationState  = NULL,
        .pViewportState      = &viewportState,
        .pRasterizationState = &rasterizer,
        .pMultisampleState   = &multisampling,
        .pDepthStencilState  = &depthStencil,
        .pColorBlendState    = &colorBlending,
        .pDynamicState       = &dynamicStateInfo,
        .layout              = m_VkPipelineLayout,
        .renderPass          = VK_NULL_HANDLE,
        .subpass             = 0,
        .basePipelineHandle  = VK_NULL_HANDLE,
        .basePipelineIndex   = -1
    };

    // ===== Create Pipeline =====
    result = vkCreateGraphicsPipelines(logicalDevice, VK_NULL_HANDLE, 1, &pipelineInfo, NULL,
                                       &m_VkPipeline);

    // ===== Free Temporary Resources =====
    free(shaderStages);
    free(bindingDescs);
    free(attributeDescs);
    free(colorFormats);

    if (result != VK_SUCCESS) {
        if (m_VkPipeline != VK_NULL_HANDLE) {
            vkDestroyPipeline(logicalDevice, m_VkPipeline, NULL);
        }
        if (m_VkPipelineLayout != VK_NULL_HANDLE) {
            vkDestroyPipelineLayout(logicalDevice, m_VkPipelineLayout, NULL);
        }
        // if (m_DescriptorSetLayouts) {
        //     for (uint32_t i = 0; i < m_DescriptorSetLayoutCount; i++) {
        //         vkDestroyDescriptorSetLayout(logicalDevice, m_DescriptorSetLayouts[i], NULL);
        //     }
        //     free(m_DescriptorSetLayouts);
        // }

        return result;
    }

    return VK_SUCCESS;
}

} // namespace CZ
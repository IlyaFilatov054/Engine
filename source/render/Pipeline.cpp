#include "render/Pipeline.h"
#include "render/Vertex.h"
#include "render/VkUtils.h"
#include <cstdint>
#include <vulkan/vulkan_core.h>

Pipeline::Pipeline(const VkContext* context, const Swapchain* swapchain, 
    const VkRenderPass renderPass, const ShaderManager* shaderManager, 
    const std::vector<VkDescriptorSetLayout> usedLayouts)
: m_context(context),
  m_swapchain(swapchain),  
  m_renderPass(renderPass), 
  m_shaderManager(shaderManager) {

    auto vertexShader = m_shaderManager->getShaderModule("vert");
    auto fragmentShader = m_shaderManager->getShaderModule("frag");

    VkPipelineShaderStageCreateInfo shaderStages[] = {
        {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
            .stage = VK_SHADER_STAGE_VERTEX_BIT,
            .module = vertexShader,
            .pName = "main"
        },
        {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
            .stage = VK_SHADER_STAGE_FRAGMENT_BIT,
            .module = fragmentShader,
            .pName = "main"
        }
    };

    auto vertexBindingDescription = Vertex::bindingDescription();
    auto vertexAttributeDesriptions = Vertex::attributeDescriptions();
    VkPipelineVertexInputStateCreateInfo vertexinput {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
        .vertexBindingDescriptionCount = 1,
        .pVertexBindingDescriptions = &vertexBindingDescription,
        .vertexAttributeDescriptionCount = vertexAttributeDesriptions.size(),
        .pVertexAttributeDescriptions = vertexAttributeDesriptions.data(),
    };

    VkPipelineInputAssemblyStateCreateInfo inputAssembly {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
        .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
        .primitiveRestartEnable = VK_FALSE
    };

    VkViewport viewport {
        .x = 0.0f,
        .y = (float)m_swapchain->extent().height, 
        .width = (float)m_swapchain->extent().width,
        .height = -(float)m_swapchain->extent().height,
        .minDepth = 0.0f,
        .maxDepth = 1.0f,
    };

    VkRect2D scissor {
        .offset = {0, 0},
        .extent = m_swapchain->extent()
    };

    VkPipelineViewportStateCreateInfo viewportState {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
        .viewportCount = 1,
        .pViewports = &viewport,
        .scissorCount = 1,
        .pScissors = &scissor
    };
    
    VkPipelineRasterizationStateCreateInfo rasterizer {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
        .depthClampEnable = VK_FALSE,
        .rasterizerDiscardEnable = VK_FALSE,
        .polygonMode = VK_POLYGON_MODE_FILL,
        .cullMode = VK_CULL_MODE_NONE,
        .frontFace = VK_FRONT_FACE_CLOCKWISE,
        .depthBiasEnable  = VK_FALSE,
        .lineWidth = 1.0f,
    };

    VkPipelineMultisampleStateCreateInfo multisampling {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
        .rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
        .sampleShadingEnable = VK_FALSE,
    };

    VkPipelineColorBlendAttachmentState colorBlendAttachment {
        .blendEnable = VK_FALSE,
        .colorWriteMask = 
            VK_COLOR_COMPONENT_R_BIT 
            | VK_COLOR_COMPONENT_G_BIT
            | VK_COLOR_COMPONENT_B_BIT
            | VK_COLOR_COMPONENT_A_BIT,
    };

    VkPipelineColorBlendStateCreateInfo colorBlend {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
        .logicOpEnable = VK_FALSE,
        .attachmentCount = 1,
        .pAttachments = &colorBlendAttachment
    };

    VkPipelineDepthStencilStateCreateInfo depth {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
        .depthTestEnable = VK_TRUE,
        .depthWriteEnable = VK_TRUE,
        .depthCompareOp = VK_COMPARE_OP_LESS,
        .depthBoundsTestEnable = VK_FALSE,
        .stencilTestEnable = VK_FALSE
    };

    VkPushConstantRange pushRange {
        .stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
        .offset = 0,
        .size = sizeof(uint32_t),
    };

    VkPipelineLayoutCreateInfo pipelaneLayoutInfo {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
        .setLayoutCount = static_cast<uint32_t>(usedLayouts.size()),
        .pSetLayouts = usedLayouts.data(),
        .pushConstantRangeCount = 1,
        .pPushConstantRanges = &pushRange
    };

    auto res = vkCreatePipelineLayout(m_context->device(), &pipelaneLayoutInfo, nullptr, &m_layout);
    validateVkResult(res, "vkCreatePipelineLayout");

    VkGraphicsPipelineCreateInfo pipelineInfo {
        .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
        .stageCount = 2,
        .pStages = shaderStages,
        .pVertexInputState = &vertexinput,
        .pInputAssemblyState = &inputAssembly,
        .pViewportState = &viewportState,
        .pRasterizationState = &rasterizer,
        .pMultisampleState = &multisampling,
        .pDepthStencilState = &depth,
        .pColorBlendState = &colorBlend,
        .layout = m_layout,
        .renderPass = m_renderPass,
        .subpass = 0,
    };

    res = vkCreateGraphicsPipelines(m_context->device(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &m_pipeline);
    validateVkResult(res, "vkCreateGraphicsPipelines");
}

Pipeline::~Pipeline() {
    vkDestroyPipeline(m_context->device(), m_pipeline, nullptr);
    vkDestroyPipelineLayout(m_context->device(), m_layout, nullptr);
}

const VkPipelineLayout& Pipeline::layout() const {
    return m_layout;
}

const VkPipeline& Pipeline::pipeline() const {
    return m_pipeline;
}
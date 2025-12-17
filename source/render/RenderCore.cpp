#include "render/RenderCore.h"
#include <cmath>
#include <cstdint>
#include <vector>
#include <vulkan/vulkan_core.h>
#include "render/RenderSynchronization.h"
#include "render/StagedBuffer.h"
#include "render/Vertex.h"
#include "render/VkUtils.h"
#include "core/Utils.h"

RenderCore::RenderCore(const VkContext* context, const Swapchain* swapchain) {
    m_context = context;
    m_swapchain = swapchain;
    
    createRenderPass();
    createFramebuffers();
    m_synchronization = new RenderSynchronization(m_context, m_framebuffers.size());
    createDescriptors();
    createPipeline();
    createCommandPool();
    createCommandBuffers();
    
    m_buffer = new StagedBuffer(m_context, m_verticies.size() * sizeof(Vertex), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_commandPool);
    m_buffer->setData(m_verticies.data());
    camera = new Camera(m_context, m_cameraDescriptorSet);
}

RenderCore::~RenderCore(){
    vkQueueWaitIdle(m_context->graphicsQueue());
    delete camera;
    delete m_buffer;
    //vkFreeDescriptorSets(m_context->device(), m_descriptorPool, 1, &m_cameraDescriptorSet);
    vkDestroyDescriptorPool(m_context->device(), m_descriptorPool, nullptr);
    vkDestroyDescriptorSetLayout(m_context->device(), m_descriptorSetLayout, nullptr);

    delete m_synchronization;

    vkFreeCommandBuffers(m_context->device(), m_commandPool, m_commandBuffers.size(), m_commandBuffers.data());
    vkDestroyCommandPool(m_context->device(), m_commandPool, nullptr);
    for(const auto& s : m_shaders){
        vkDestroyShaderModule(m_context->device(), s, nullptr);
    }

    vkDestroyPipeline(m_context->device(), m_pipeline, nullptr);
    vkDestroyPipelineLayout(m_context->device(), m_pipelineLayout, nullptr);

    for(const auto& f : m_framebuffers){
        vkDestroyFramebuffer(m_context->device(), f, nullptr);
    }

    vkDestroyRenderPass(m_context->device(), m_renderPass, nullptr);
}

float c = 0;
void RenderCore::drawFrame() {
    c += 0.001f;
    //camera->position.y += 5 * std::sin(c);
    camera->position.x = 4 * std::cos(c);
    camera->position.z = 4 * std::sin(c);
    camera->update();

    m_synchronization->waitCurrentFence();

    uint32_t imageIndex;
    auto res = vkAcquireNextImageKHR(
        m_context->device(),
        m_swapchain->swapchain(),
        UINT64_MAX,
        m_synchronization->imageAvailable(),
        VK_NULL_HANDLE,
        &imageIndex);
    //validateVkResult(res, "vkAcquireNextImageKHR");
    
    vkResetCommandBuffer(m_commandBuffers[m_synchronization->currentFrame()], VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT);
    recordCommandBuffer(m_commandBuffers[m_synchronization->currentFrame()], m_framebuffers[imageIndex]);

    VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    VkSubmitInfo submitInfo {
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = &m_synchronization->imageAvailable(),
        .pWaitDstStageMask = waitStages,
        .commandBufferCount = 1,
        .pCommandBuffers = &m_commandBuffers[m_synchronization->currentFrame()],
        .signalSemaphoreCount = 1,
        .pSignalSemaphores = &m_synchronization->renderFinishedSemaphore(imageIndex)
    };
    vkQueueSubmit(m_context->graphicsQueue(), 1, &submitInfo, m_synchronization->gpuReadyFence());

    VkPresentInfoKHR presentInfo {
        .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = &m_synchronization->renderFinishedSemaphore(imageIndex),
        .swapchainCount = 1,
        .pSwapchains =  &m_swapchain->swapchain(),
        .pImageIndices = &imageIndex
    };
    vkQueuePresentKHR(m_context->graphicsQueue(), &presentInfo);

    m_synchronization->nextFrame();
}

void RenderCore::createRenderPass() {
    VkAttachmentDescription colorAttachment {
        .format = m_swapchain->format().format,
        .samples = VK_SAMPLE_COUNT_1_BIT,
        .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
        .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
        .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
        .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        .finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
    };

    VkAttachmentReference colorAttachmentReference {
        .attachment = 0,
        .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
    };

    VkSubpassDescription subpassDescription {
        .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
        .colorAttachmentCount = 1,
        .pColorAttachments = &colorAttachmentReference
    };

    VkSubpassDependency subpassDependency {
        .srcSubpass = VK_SUBPASS_EXTERNAL,
        .dstSubpass = 0,
        .srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
        .dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
        .srcAccessMask = 0,
        .dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT
    };

    VkRenderPassCreateInfo renderPassCreateInfo {
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
        .attachmentCount = 1,
        .pAttachments = &colorAttachment,
        .subpassCount = 1,
        .pSubpasses = &subpassDescription,
        .dependencyCount = 1,
        .pDependencies = &subpassDependency
    };

    auto res = vkCreateRenderPass(m_context->device(), &renderPassCreateInfo, nullptr, &m_renderPass);
    validateVkResult(res, "vkCreateRenderPass");
}

void RenderCore::createFramebuffers() {
    const auto& imageViews = m_swapchain->imageViews();
    m_framebuffers.resize(imageViews.size());
    for(uint32_t i = 0; i < imageViews.size(); i++){
        VkFramebufferCreateInfo framebufferInfo {
            .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
            .renderPass = m_renderPass,
            .attachmentCount = 1,
            .pAttachments = &imageViews[i],
            .width = m_swapchain->extent().width,
            .height = m_swapchain->extent().height,
            .layers = 1,
        };
        auto res = vkCreateFramebuffer(m_context->device(), &framebufferInfo, nullptr, &m_framebuffers[i]);
        validateVkResult(res, "vkCreateFramebuffer");
    }
}

VkShaderModule RenderCore::createShaderModule(const std::vector<char> code) {
    VkShaderModuleCreateInfo shaderInfo {
        .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
        .codeSize = code.size(),
        .pCode = reinterpret_cast<const uint32_t*>(code.data()),
    };

    VkShaderModule shaderModule;
    auto res = vkCreateShaderModule(m_context->device(), &shaderInfo, nullptr, &shaderModule);
    validateVkResult(res, "vkCreateShaderModule");

    m_shaders.push_back(shaderModule);
    return shaderModule;
}

void RenderCore::createPipeline() {
    //TODO: make shader manager
    auto vertexShader = createShaderModule(readFile("shaders/main.vert.spv"));
    auto fragmentShader = createShaderModule(readFile("shaders/main.frag.spv"));

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

    VkPipelineLayoutCreateInfo pipelaneLayoutInfo {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
        .setLayoutCount = 1,
        .pSetLayouts = &m_descriptorSetLayout,
        .pushConstantRangeCount = 0,
    };

    auto res = vkCreatePipelineLayout(m_context->device(), &pipelaneLayoutInfo, nullptr, &m_pipelineLayout);
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
        .pColorBlendState = &colorBlend,
        .layout = m_pipelineLayout,
        .renderPass = m_renderPass,
        .subpass = 0
    };

    res = vkCreateGraphicsPipelines(m_context->device(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &m_pipeline);
    validateVkResult(res, "vkCreateGraphicsPipelines");
}

void RenderCore::createCommandPool() {
    VkCommandPoolCreateInfo commandPoolInfo {
        .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
        .queueFamilyIndex = m_context->graphicsQueueIndex(),
    };
    vkCreateCommandPool(m_context->device(), &commandPoolInfo, nullptr, &m_commandPool);
}

void RenderCore::createCommandBuffers() {
    m_commandBuffers.resize(m_synchronization->maxFrames());
    VkCommandBufferAllocateInfo commandBufferInfo {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .commandPool = m_commandPool,
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = (uint32_t)m_commandBuffers.size(),
    };
    vkAllocateCommandBuffers(m_context->device(), &commandBufferInfo, m_commandBuffers.data());
}

void RenderCore::recordCommandBuffer(VkCommandBuffer buffer, const VkFramebuffer framebuffer) {
    VkCommandBufferBeginInfo beginInfo {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO
    };
    vkBeginCommandBuffer(buffer, &beginInfo);
    
    VkClearValue clearColor = {0.0f, 0.0f, 0.0f, 1.0f};
    VkRenderPassBeginInfo renderPassInfo {
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
        .renderPass = m_renderPass,
        .framebuffer = framebuffer,
        .renderArea = {.offset = {0, 0}, .extent = m_swapchain->extent()},
        .clearValueCount = 1,
        .pClearValues = &clearColor
    };
    vkCmdBeginRenderPass(buffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
    vkCmdBindPipeline(buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline);
    
    vkCmdBindDescriptorSets(buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipelineLayout, 0, 1, &m_cameraDescriptorSet, 0, nullptr);
    VkDeviceSize offsets[] = {0};
    vkCmdBindVertexBuffers(buffer, 0, 1, &m_buffer->buffer(), offsets);
    vkCmdDraw(buffer, 3, 1, 0, 0);
    
    vkCmdEndRenderPass(buffer);
    vkEndCommandBuffer(buffer);
}

void RenderCore::createDescriptors() {
    VkDescriptorSetLayoutBinding layoutBinding {
        .binding = 0,
        .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
        .descriptorCount = 1,
        .stageFlags = VK_SHADER_STAGE_VERTEX_BIT
    };

    VkDescriptorSetLayoutCreateInfo layoutInfo {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
        .bindingCount = 1,
        .pBindings = &layoutBinding
    };
    vkCreateDescriptorSetLayout(m_context->device(), &layoutInfo, nullptr, &m_descriptorSetLayout);

    VkDescriptorPoolSize poolSize {
        .type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
        .descriptorCount = 1,
    };

    VkDescriptorPoolCreateInfo poolInfo {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
        .maxSets = 1,
        .poolSizeCount = 1,
        .pPoolSizes = &poolSize,
    };
    vkCreateDescriptorPool(m_context->device(), &poolInfo, nullptr, &m_descriptorPool);

    VkDescriptorSetAllocateInfo setAllocInfo {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
        .descriptorPool = m_descriptorPool,
        .descriptorSetCount = 1,
        .pSetLayouts = &m_descriptorSetLayout
    };
    vkAllocateDescriptorSets(m_context->device(), &setAllocInfo, &m_cameraDescriptorSet);
}
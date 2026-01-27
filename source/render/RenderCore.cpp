#include "render/RenderCore.h"
#include <cmath>
#include <cstdint>
#include <glm/ext/matrix_transform.hpp>
#include <glm/fwd.hpp>
#include <glm/trigonometric.hpp>
#include <random>
#include <vector>
#include <vulkan/vulkan_core.h>
#include "render/AttachmentResources.h"
#include "render/DescriptorManager.h"
#include "render/FrameManager.h"
#include "render/Pipeline.h"
#include "render/RenderGraph.h"
#include "render/RenderPass.h"
#include "render/ResourceManager.h"
#include "render/ShaderManager.h"
#include "core/Utils.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

RenderCore::RenderCore(const VkContext* context, const Swapchain* swapchain) {
    m_context = context;
    m_swapchain = swapchain;

    m_descriptorManager = new DescriptorManager(m_context);
    
    m_frameManager = new FrameManager(m_context, m_swapchain->images().size());
    m_frameManager->createFrameResources();
    m_frameManager->createImageResources();

    std::vector<VkDescriptorSetLayoutBinding> ssboBindings = {
    VkDescriptorSetLayoutBinding {
            .binding = 0,
            .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
            .descriptorCount = 1,
            .stageFlags = VK_SHADER_STAGE_VERTEX_BIT
        },
    };
    auto ssboLayout = m_descriptorManager->createLayout(ssboBindings);
    auto ssboDescriptors = m_descriptorManager->allocateSets(ssboLayout, m_frameManager->imageCount());

    std::vector<VkDescriptorSetLayoutBinding> textureBindings = {
        VkDescriptorSetLayoutBinding {
            .binding = 0,
            .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
            .descriptorCount = 16,
            .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT
        },
    };
    auto texturesLayout = m_descriptorManager->createLayout(textureBindings);
    auto textureDescriptor = m_descriptorManager->allocateSet(texturesLayout);

    std::vector<VkDescriptorSetLayoutBinding> cameraBindings = {
        VkDescriptorSetLayoutBinding {
            .binding = 0,
            .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
            .descriptorCount = 1,
            .stageFlags = VK_SHADER_STAGE_VERTEX_BIT
        },
    };
    auto cameraLayout = m_descriptorManager->createLayout(cameraBindings);
    auto camera1Descriptors = m_descriptorManager->allocateSets(cameraLayout, m_frameManager->imageCount());
    auto camera2Descriptors = m_descriptorManager->allocateSets(cameraLayout, m_frameManager->imageCount());

    std::vector<VkDescriptorSetLayoutBinding> offscreenBindings = {
        VkDescriptorSetLayoutBinding {
            .binding = 0,
            .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
            .descriptorCount = 1,
            .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT
        },
    };
    auto offscreenLayout = m_descriptorManager->createLayout(offscreenBindings);
    auto offscreenDescriptors = m_descriptorManager->allocateSets(offscreenLayout, m_frameManager->imageCount());

    std::vector<VkDescriptorSetLayout> firstPassUsedDescriptorSetLayouts = {
        m_descriptorManager->layout(texturesLayout),
        m_descriptorManager->layout(ssboLayout),
        m_descriptorManager->layout(cameraLayout),
    };

    std::vector<VkDescriptorSetLayout> secondPassUsedDescriptorSetLayouts = {
        m_descriptorManager->layout(offscreenLayout),
        m_descriptorManager->layout(ssboLayout),
        m_descriptorManager->layout(cameraLayout)
    };

    m_resourceManager = new ResourceManager(m_context, textureDescriptor);
    m_resourceManager->addMesh(m_vertices, m_indices);
    m_resourceManager->addTexture("/home/ilya/Pictures/Wallpapers/landscapes/Rainnight.jpg");
    
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> dis(-10.0f, 10.0f);
    for(uint32_t i = 0; i < 10; i++) {
        auto id = m_resourceManager->addRenderData();
        m_resourceManager->renderData(id).model = glm::rotate
        (
            glm::translate(
                glm::scale(glm::mat4(1.0f), glm::vec3(0.1f, 0.1f, 0.1f)),
                glm::vec3(static_cast<glm::float32>(dis(gen)), static_cast<glm::float32>(dis(gen)), static_cast<glm::float32>(dis(gen)))
            ),
            glm::radians(static_cast<glm::float32>(dis(gen) * 18)), 
            glm::vec3(static_cast<glm::float32>(dis(gen)), static_cast<glm::float32>(dis(gen)), static_cast<glm::float32>(dis(gen)))
        );
        m_renderObjects.push_back({.mesh = 0, .renderData = id});
    }
    auto id = m_resourceManager->addRenderData();
    m_resourceManager->renderData(id).model = glm::mat4(1.0f);
    m_renderObjects.push_back({.mesh = 0, .renderData = id});

    camera1 = new Camera(m_context);
    camera1->aspect = (float)m_swapchain->extent().width / (float)m_swapchain->extent().height;

    camera2 = new Camera(m_context);
    camera2->aspect = (float)m_swapchain->extent().width / (float)m_swapchain->extent().height;

    m_shaderManager = new ShaderManager(m_context);
    m_shaderManager->createShaderModule(readFile("shaders/main.vert.spv"), "vert");
    m_shaderManager->createShaderModule(readFile("shaders/main.frag.spv"), "frag");
    m_shaderManager->createShaderModule(readFile("shaders/test.vert.spv"), "testvert");
    m_shaderManager->createShaderModule(readFile("shaders/test.frag.spv"), "testfrag");
    auto vertexShader = m_shaderManager->getShaderModule("vert");
    auto fragmentShader = m_shaderManager->getShaderModule("frag");
    auto testVertexShader = m_shaderManager->getShaderModule("testvert");
    auto testFragmentShader = m_shaderManager->getShaderModule("testfrag");
    std::vector<ShaderDescription> shaderDescriptions = {
        {
            .module = vertexShader,
            .stage = VK_SHADER_STAGE_VERTEX_BIT,
        },
        {
            .module = fragmentShader,
            .stage = VK_SHADER_STAGE_FRAGMENT_BIT,
        }
    };
    std::vector<ShaderDescription> shaderDescriptions2 = {
        {
            .module = vertexShader,
            .stage = VK_SHADER_STAGE_VERTEX_BIT,
        },
        {
            .module = testFragmentShader,
            .stage = VK_SHADER_STAGE_FRAGMENT_BIT,
        }
    };

    m_renderGraph = new RenderGraph(m_context);

    std::vector<VkAttachmentDescription> renderPassAttachments {
        {
            .format = m_swapchain->format().format,
            .samples = VK_SAMPLE_COUNT_1_BIT,
            .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
            .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
            .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
            .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
            .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
            .finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
        },
        {
            .format = VK_FORMAT_D32_SFLOAT,
            .samples = VK_SAMPLE_COUNT_1_BIT,
            .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
            .storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
            .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
            .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
            .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
            .finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
        }
    };
    std::vector<VkImageLayout> renderPassAttachmentLayouts {
        VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
    };
    std::vector<VkAttachmentDescription> renderPass2Attachments {
        {
            .format = m_swapchain->format().format,
            .samples = VK_SAMPLE_COUNT_1_BIT,
            .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
            .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
            .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
            .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
            .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
            .finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
        },
        {
            .format = VK_FORMAT_D32_SFLOAT,
            .samples = VK_SAMPLE_COUNT_1_BIT,
            .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
            .storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
            .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
            .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
            .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
            .finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
        }
    };
    std::vector<VkImageLayout> renderPass2AttachmentLayouts {
        VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
    };
    RenderPassHandle renderPass = m_renderGraph->addRenderPass(renderPassAttachments, renderPassAttachmentLayouts, m_swapchain->extent(), shaderDescriptions, firstPassUsedDescriptorSetLayouts);
    RenderPassHandle secondRenderPass = m_renderGraph->addRenderPass(renderPass2Attachments, renderPass2AttachmentLayouts, m_swapchain->extent(), shaderDescriptions2, secondPassUsedDescriptorSetLayouts);

    ImageAttachmentDescription swapchainImageDescription {
        .externalImages = m_swapchain->images()
    };
    auto swapchainImageHandle = m_renderGraph->addImageAttachment(swapchainImageDescription);
    
    ImageAttachmentDescription depthImageDescription {
        .format = VK_FORMAT_D32_SFLOAT,
        .usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
        .extent = {m_swapchain->extent().width, m_swapchain->extent().height, 1},
        .aspect = VK_IMAGE_ASPECT_DEPTH_BIT
    };
    auto offscreenDepthImageHandle = m_renderGraph->addImageAttachment(depthImageDescription);
    auto presentDepthImageHandle = m_renderGraph->addImageAttachment(depthImageDescription);

    ImageAttachmentDescription offscreenColorImageDescription {
        .format = m_swapchain->format().format,
        .usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
        .extent = {m_swapchain->extent().width, m_swapchain->extent().height, 1},
        .aspect = VK_IMAGE_ASPECT_COLOR_BIT
    };
    auto offscreenColorHandle = m_renderGraph->addImageAttachment(offscreenColorImageDescription);

    WriteAttachmentDescription presentAttachmentDescription {
        .renderPass = secondRenderPass,
        .extent = m_swapchain->extent(),
        .imageAttachments = {swapchainImageHandle, presentDepthImageHandle}
    };
    auto presentAttachment = m_renderGraph->addWriteAttachment(presentAttachmentDescription);

    WriteAttachmentDescription offscreenWriteAttachmentDescription {
        .renderPass = renderPass,
        .extent = m_swapchain->extent(),
        .imageAttachments = {offscreenColorHandle, offscreenDepthImageHandle}
    };
    auto offscreenWriteAttachment = m_renderGraph->addWriteAttachment(offscreenWriteAttachmentDescription);

    ReadAttachmentDescription offscreenReadAttachmentDescription {
        .perFrameDescriptors = offscreenDescriptors,
        .imageAttachments = {offscreenColorHandle},
    };
    auto offscreenReadAttachment = m_renderGraph->addReadAttachment(offscreenReadAttachmentDescription);

    DescriptorAttachmentDescription camera1AttachmentDescription {
        .perFrameDescriptors = camera1Descriptors,
        .type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 
        .usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, 
        .bufferSize = camera1->dataSize(),
        .updateSource = [&](){ return camera1->data(); },
    };
    auto camera1Attachment = m_renderGraph->addDescriptorAttachment(camera1AttachmentDescription);

DescriptorAttachmentDescription camera2AttachmentDescription {
        .perFrameDescriptors = camera2Descriptors,
        .type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 
        .usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, 
        .bufferSize = camera1->dataSize(),
        .updateSource = [&](){ return camera2->data(); },
    };
    auto camera2Attachment = m_renderGraph->addDescriptorAttachment(camera2AttachmentDescription);

    DescriptorAttachmentDescription ssboAttachmentDescription {
        .perFrameDescriptors = ssboDescriptors,
        .type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 
        .usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, 
        .bufferSize = 1024,
        .updateSource = [&](){ return m_resourceManager->renderData(); },
    };
    auto ssboAttachment = m_renderGraph->addDescriptorAttachment(ssboAttachmentDescription);
    
    m_renderGraph->createAttachmentResources(m_frameManager->imageCount());

    m_renderGraph->addNode(
        {
            .renderPass = renderPass,
            .outputAttachment = offscreenWriteAttachment,
            .constDescriptors = {
                m_resourceManager->texturesDescriptor()
            },
            .descriptorAttachments = {ssboAttachment, camera1Attachment},
            .clearValues = {
                {
                    {.color = {1.0f, 0.0f, 0.0f, 1.0f}},
                    {.depthStencil = {1.0f, 0}},
                }
            },
            .frameRenderData = 0
        }, 0
    );
    m_renderGraph->addNode(
        {
            .renderPass = secondRenderPass,
            .inputAttachments = {offscreenReadAttachment},
            .outputAttachment = presentAttachment,
            .descriptorAttachments = {ssboAttachment, camera2Attachment},
            .descriptorOrder = {DescriptorStage::Input, DescriptorStage::Frame, DescriptorStage::Const},
            .clearValues = {
                {
                    {.color = {0.0f, 1.0f, 0.0f, 1.0f}},
                    {.depthStencil = {1.0f, 0}},
                }
            },
            .frameRenderData = 1
        }, 1
    );
}

RenderCore::~RenderCore(){
    vkQueueWaitIdle(m_context->graphicsQueue());
    
    delete camera1;
    delete camera2;
    delete m_resourceManager;
    delete m_shaderManager;
    delete m_frameManager;
    delete m_renderGraph;
    delete m_descriptorManager;
}

float c = 0;
void RenderCore::drawFrame() {
    c += 0.0001f;
    //camera->position.y += 5 * std::sin(c);
    camera2->position.x = 3 * std::cos(c);
    camera2->position.z = 3 * std::sin(c);
    camera1->position.x = 4 * std::cos(c);
    camera1->position.z = 4 * std::sin(c);
    //camera->position.y = 4 * std::sin(0.333f * c);

    m_frameManager->currentFrameResources()->waitFence();

    uint32_t imageIndex;
    auto res = vkAcquireNextImageKHR(
        m_context->device(),
        m_swapchain->swapchain(),
        UINT64_MAX,
        m_frameManager->currentFrameResources()->imageAcquiredSemaphore(),
        VK_NULL_HANDLE,
        &imageIndex);
    //validateVkResult(res, "vkAcquireNextImageKHR");
    
    vkResetCommandBuffer(m_frameManager->currentFrameResources()->commandBuffer(), 0);
    recordCommandBuffer(m_frameManager->currentFrameResources()->commandBuffer(), imageIndex);

    VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    VkSubmitInfo submitInfo {
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = &m_frameManager->currentFrameResources()->imageAcquiredSemaphore(),
        .pWaitDstStageMask = waitStages,
        .commandBufferCount = 1,
        .pCommandBuffers = &m_frameManager->currentFrameResources()->commandBuffer(),
        .signalSemaphoreCount = 1,
        .pSignalSemaphores = &m_frameManager->imageResources(imageIndex)->renderFinishedSemaphore()
    };
    vkQueueSubmit(m_context->graphicsQueue(), 1, &submitInfo, m_frameManager->currentFrameResources()->submitFence());

    VkPresentInfoKHR presentInfo {
        .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = &m_frameManager->imageResources(imageIndex)->renderFinishedSemaphore(),
        .swapchainCount = 1,
        .pSwapchains =  &m_swapchain->swapchain(),
        .pImageIndices = &imageIndex
    };
    vkQueuePresentKHR(m_context->graphicsQueue(), &presentInfo);

    m_frameManager->nextFrame();
}

void RenderCore::recordCommandBuffer(const VkCommandBuffer buffer, uint32_t image) {
    VkCommandBufferBeginInfo beginInfo {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO
    };
    vkBeginCommandBuffer(buffer, &beginInfo);

    for(const auto& r : m_renderObjects) {
        if(r.renderData == 10) continue;
        m_renderGraph->addDrawCall(0, [&](
            const VkCommandBuffer commandBuffer,
            const RenderPass* renderPass,
            const AttachmentResources* attachments
        ) {
            vkCmdPushConstants(commandBuffer, renderPass->pipeline().layout(), VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(uint32_t), &r.renderData);
            VkDeviceSize offsets[] = {0};
            VkBuffer buffers[] = {m_resourceManager->mesh(r.mesh)->vertexBuffer()};
            vkCmdBindVertexBuffers(commandBuffer, 0, 1, buffers, offsets);
            vkCmdBindIndexBuffer(commandBuffer, m_resourceManager->mesh(r.mesh)->indexBuffer(), 0, VK_INDEX_TYPE_UINT32);
            vkCmdDrawIndexed(commandBuffer, m_resourceManager->mesh(r.mesh)->indicesCount(), 1, 0, 0, 0);
        });
    }

    m_renderGraph->addDrawCall(1, [&](
        const VkCommandBuffer commandBuffer,
        const RenderPass* renderPass,
        const AttachmentResources* attachments
    ) {
        uint32_t pushConstants[] = {10};
        vkCmdPushConstants(commandBuffer, renderPass->pipeline().layout(), VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(uint32_t), pushConstants);
        VkDeviceSize offsets[] = {0};
        VkBuffer buffers[] = {m_resourceManager->mesh(0)->vertexBuffer()};
        vkCmdBindVertexBuffers(commandBuffer, 0, 1, buffers, offsets);
        vkCmdBindIndexBuffer(commandBuffer, m_resourceManager->mesh(0)->indexBuffer(), 0, VK_INDEX_TYPE_UINT32);
        vkCmdDrawIndexed(commandBuffer, m_resourceManager->mesh(0)->indicesCount(), 1, 0, 0, 0);
    });

    m_renderGraph->execute(buffer, image);
    vkEndCommandBuffer(buffer);
}
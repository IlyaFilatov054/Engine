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
#include "render/MeshBuffer.h"
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
    std::vector<VkDescriptorSetLayoutBinding> ssboBindings = {
    VkDescriptorSetLayoutBinding {
            .binding = 0,
            .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
            .descriptorCount = 1,
            .stageFlags = VK_SHADER_STAGE_VERTEX_BIT
        },
    };
    uint32_t ssboLayout = m_descriptorManager->createLayout(ssboBindings);

    std::vector<VkDescriptorSetLayoutBinding> textureBindings = {
        VkDescriptorSetLayoutBinding {
            .binding = 0,
            .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
            .descriptorCount = 16,
            .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT
        },
    };
    uint32_t texturesLayout = m_descriptorManager->createLayout(textureBindings);
    m_resourceManager = new ResourceManager(m_context, m_descriptorManager->allocateSet(texturesLayout));
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

    std::vector<VkDescriptorSetLayoutBinding> cameraBindings = {
        VkDescriptorSetLayoutBinding {
            .binding = 0,
            .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
            .descriptorCount = 1,
            .stageFlags = VK_SHADER_STAGE_VERTEX_BIT
        },
    };
    uint32_t cameraLayout = m_descriptorManager->createLayout(cameraBindings);
    camera = new Camera(m_context, m_descriptorManager->allocateSet(cameraLayout));
    camera->aspect = (float)m_swapchain->extent().width / (float)m_swapchain->extent().height;

    std::vector<VkDescriptorSetLayout> usedLayouts = {
        m_descriptorManager->layout(cameraLayout),
        m_descriptorManager->layout(texturesLayout),
        m_descriptorManager->layout(ssboLayout),
    };

    m_shaderManager = new ShaderManager(m_context);
    m_shaderManager->createShaderModule(readFile("shaders/main.vert.spv"), "vert");
    m_shaderManager->createShaderModule(readFile("shaders/main.frag.spv"), "frag");
    auto vertexShader = m_shaderManager->getShaderModule("vert");
    auto fragmentShader = m_shaderManager->getShaderModule("frag");
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
    m_renderGraph = new RenderGraph(m_context);
    uint32_t renderPass = m_renderGraph->addRenderPass(m_swapchain->format().format, m_swapchain->extent(), shaderDescriptions, usedLayouts);
    
    m_frameManager->createFrameResources();
    m_frameManager->createImageResources();
    m_frameManager->createAttachmentResources();

    auto ssboSets = m_descriptorManager->allocateSets(ssboLayout, m_frameManager->imageCount());
    uint32_t framebuffer = 0;
    uint32_t ssbo = 0;
    for(uint32_t i = 0; i < m_frameManager->imageCount(); i++) {
        auto color = m_frameManager->attachmentResources(i)->addImageAttachment(m_swapchain->images()[i]);
        auto depth = m_frameManager->attachmentResources(i)->addImageAttachment(
            VK_FORMAT_D32_SFLOAT,
            VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
            { m_swapchain->extent().width, m_swapchain->extent().height, 1},
            VK_IMAGE_ASPECT_DEPTH_BIT
        );
        m_frameManager->attachmentResources(i)->addWriteAttachment(m_renderGraph->renderPass(renderPass)->renderPass(), {m_swapchain->extent().width, m_swapchain->extent().height}, {color, depth});
        ssbo = m_frameManager->attachmentResources(i)->addDescriptorAttachment(ssboSets[i], VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, 1024);
    }
    
    m_renderGraph->addNode(
        {
            .renderPass = renderPass,
            .outputFramebuffer = framebuffer,
            .constDescriptors = {
                camera->descriptorSet(),
                m_resourceManager->texturesDescriptor()           
            },
            .frameDescriptors = {ssbo},
            .clearDrawCalls = true,
        }, 0);
}

RenderCore::~RenderCore(){
    vkQueueWaitIdle(m_context->graphicsQueue());
    
    delete camera;
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
    camera->position.x = 4 * std::cos(c);
    camera->position.z = 4 * std::sin(c);
    //camera->position.y = 4 * std::sin(0.333f * c);
    camera->update();

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
    recordCommandBuffer(m_frameManager->currentFrameResources(), m_frameManager->imageResources(imageIndex), m_frameManager->attachmentResources(imageIndex));

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

void RenderCore::recordCommandBuffer(const FrameResources* frameResources, const ImageResources* imageResources, const AttachmentResources* attachmentResources) {
    VkCommandBufferBeginInfo beginInfo {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO
    };
    vkBeginCommandBuffer(frameResources->commandBuffer(), &beginInfo);
    
    attachmentResources->descriptorAttachment(0).buffer->stagingBuffer().setData(m_resourceManager->renderData(), m_resourceManager->renderDataSize(), 0);
    attachmentResources->descriptorAttachment(0).buffer->flush(frameResources->commandBuffer());

    for(const auto& r : m_renderObjects) {
        m_renderGraph->addDrawCall(0, {
            .pushConstant = r.renderData,
            .mesh = m_resourceManager->mesh(r.mesh)
        });
    }
    m_renderGraph->execute(frameResources->commandBuffer(), attachmentResources);
    vkEndCommandBuffer(frameResources->commandBuffer());
}
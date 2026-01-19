#include "render/RenderCore.h"
#include <cmath>
#include <cstdint>
#include <glm/ext/matrix_transform.hpp>
#include <glm/fwd.hpp>
#include <glm/trigonometric.hpp>
#include <random>
#include <vector>
#include <vulkan/vulkan_core.h>
#include "render/DescriptorManager.h"
#include "render/FrameManager.h"
#include "render/MeshBuffer.h"
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

    m_shaderManager = new ShaderManager(m_context);
    m_shaderManager->createShaderModule(readFile("shaders/main.vert.spv"), "vert");
    m_shaderManager->createShaderModule(readFile("shaders/main.frag.spv"), "frag");
    
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
    const auto sets = m_descriptorManager->allocateSets(ssboLayout, m_frameManager->maxFrames());
    m_frameManager->createFrameResources(sets);
    m_frameManager->createImageResources();
    uint32_t colorAttachment = m_frameManager->addImageAttachments(m_swapchain->images());
    uint32_t depthAttachment = m_frameManager->addImageAttachment(VK_FORMAT_D32_SFLOAT,
        VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
        {m_swapchain->extent().width, m_swapchain->extent().height, 1},
        VK_IMAGE_ASPECT_DEPTH_BIT);
    
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
    std::uniform_real_distribution<> dis(-100.0f, 100.0f);
    for(uint32_t i = 0; i < 1000; i++) {
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
        m_descriptorManager->layout(ssboLayout),
        m_descriptorManager->layout(texturesLayout),
    };
    m_renderPass = new RenderPass(m_context, m_swapchain, m_shaderManager, usedLayouts);
}

RenderCore::~RenderCore(){
    vkQueueWaitIdle(m_context->graphicsQueue());
    
    delete camera;
    delete m_resourceManager;
    delete m_shaderManager;
    delete m_frameManager;
    delete m_renderPass;
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

    m_frameManager->currentFrameResources().waitFence();

    uint32_t imageIndex;
    auto res = vkAcquireNextImageKHR(
        m_context->device(),
        m_swapchain->swapchain(),
        UINT64_MAX,
        m_frameManager->currentFrameResources().imageAcquiredSemaphore(),
        VK_NULL_HANDLE,
        &imageIndex);
    //validateVkResult(res, "vkAcquireNextImageKHR");
    
    vkResetCommandBuffer(m_frameManager->currentFrameResources().commandBuffer(), 0);
    recordCommandBuffer(m_frameManager->currentFrameResources(), m_frameManager->imageResources(imageIndex));

    VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    VkSubmitInfo submitInfo {
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = &m_frameManager->currentFrameResources().imageAcquiredSemaphore(),
        .pWaitDstStageMask = waitStages,
        .commandBufferCount = 1,
        .pCommandBuffers = &m_frameManager->currentFrameResources().commandBuffer(),
        .signalSemaphoreCount = 1,
        .pSignalSemaphores = &m_frameManager->imageResources(imageIndex).renderFinishedSemaphore()
    };
    vkQueueSubmit(m_context->graphicsQueue(), 1, &submitInfo, m_frameManager->currentFrameResources().submitFence());

    VkPresentInfoKHR presentInfo {
        .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = &m_frameManager->imageResources(imageIndex).renderFinishedSemaphore(),
        .swapchainCount = 1,
        .pSwapchains =  &m_swapchain->swapchain(),
        .pImageIndices = &imageIndex
    };
    vkQueuePresentKHR(m_context->graphicsQueue(), &presentInfo);

    m_frameManager->nextFrame();
}

void RenderCore::recordCommandBuffer(const FrameResources& frameResources, const ImageResources& imageResources) {
    VkCommandBufferBeginInfo beginInfo {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO
    };
    vkBeginCommandBuffer(frameResources.commandBuffer(), &beginInfo);
    
    frameResources.setSsboData(frameResources.commandBuffer(), m_resourceManager->renderData(), m_resourceManager->renderDataSize());

    VkClearValue clearValues[2];
    clearValues[0].color = {0.0f, 0.0f, 0.0f, 1.0f};
    clearValues[1].depthStencil = {1.0f, 0};
    VkRenderPassBeginInfo renderPassInfo {
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
        .renderPass = m_renderPass->renderPass(),
        .framebuffer = imageResources.framebuffer(),
        .renderArea = {.offset = {0, 0}, .extent = m_swapchain->extent()},
        .clearValueCount = 2,
        .pClearValues = clearValues
    };
    vkCmdBeginRenderPass(frameResources.commandBuffer(), &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
    vkCmdBindPipeline(frameResources.commandBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, m_renderPass->pipeline().pipeline());
    
    VkDescriptorSet descriptors[] = {
        camera->descriptorSet(), 
        frameResources.ssboDescriptor(),
        m_resourceManager->texturesDescriptor(),
    };
    vkCmdBindDescriptorSets(frameResources.commandBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, m_renderPass->pipeline().layout(), 0, 3, descriptors, 0, nullptr);

    for(const auto& r : m_renderObjects) {
        vkCmdPushConstants(frameResources.commandBuffer(), m_renderPass->pipeline().layout(), VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(uint32_t), &r.renderData);
        VkDeviceSize offsets[] = {0};
        vkCmdBindVertexBuffers(frameResources.commandBuffer(), 0, 1, &m_resourceManager->mesh(r.mesh)->vertexBuffer(), offsets);
        vkCmdBindIndexBuffer(frameResources.commandBuffer(), m_resourceManager->mesh(r.mesh)->indexBuffer(), 0, VK_INDEX_TYPE_UINT32);
        vkCmdDrawIndexed(frameResources.commandBuffer(), m_resourceManager->mesh(r.mesh)->indicesCount(), 1, 0, 0, 0);
    }

    vkCmdEndRenderPass(frameResources.commandBuffer());
    vkEndCommandBuffer(frameResources.commandBuffer());
}
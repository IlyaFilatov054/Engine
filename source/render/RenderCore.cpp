#include "render/RenderCore.h"
#include <cmath>
#include <cstdint>
#include <glm/ext/matrix_transform.hpp>
#include <glm/fwd.hpp>
#include <glm/trigonometric.hpp>
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

    m_renderPass = new RenderPass(m_context, m_swapchain, m_shaderManager, m_descriptorManager);
    m_frameManager = new FrameManager(m_context, m_swapchain, m_renderPass->renderPass());
    
    m_resourceManager = new ResourceManager(m_context, m_descriptorManager->ssboSet(), m_descriptorManager->texturesSet());
    m_resourceManager->addMesh(m_vertices, m_indices);
    m_resourceManager->addTexture("/home/ilya/Pictures/Wallpapers/landscapes/Rainnight.jpg");
    m_resourceManager->addRenderData();

    camera = new Camera(m_context, m_descriptorManager->cameraSet());
    camera->aspect = (float)m_swapchain->extent().width / (float)m_swapchain->extent().height;
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

    auto& model = m_resourceManager->renderData(0)->model;
    model = glm::translate(glm::mat4(1.0f), {0.0f, std::sin(c), 0.0f});
    model = glm::scale(glm::mat4(1.0f), {1.0f, 1.0f, std::sin(c * 5)});
    m_resourceManager->renderData(0)->textureIndex = 1;
    m_resourceManager->flushSsbo();

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
    recordCommandBuffer(m_frameManager->currentFrameResources().commandBuffer(), m_frameManager->imageResources(imageIndex).framebuffer());

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

void RenderCore::recordCommandBuffer(VkCommandBuffer buffer, const VkFramebuffer framebuffer) {
    VkCommandBufferBeginInfo beginInfo {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO
    };
    vkBeginCommandBuffer(buffer, &beginInfo);
    
    VkClearValue clearValues[2];
    clearValues[0].color = {0.0f, 0.0f, 0.0f, 1.0f};
    clearValues[1].depthStencil = {1.0f, 0};
    VkRenderPassBeginInfo renderPassInfo {
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
        .renderPass = m_renderPass->renderPass(),
        .framebuffer = framebuffer,
        .renderArea = {.offset = {0, 0}, .extent = m_swapchain->extent()},
        .clearValueCount = 2,
        .pClearValues = clearValues
    };
    vkCmdBeginRenderPass(buffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
    vkCmdBindPipeline(buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_renderPass->pipeline().pipeline());
    
    VkDescriptorSet descriptors[] = {
        m_descriptorManager->cameraSet(), 
        m_descriptorManager->ssboSet(), 
        m_descriptorManager->texturesSet()
    };
    vkCmdBindDescriptorSets(buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_renderPass->pipeline().layout(), 0, 3, descriptors, 0, nullptr);
    
    VkDeviceSize offsets[] = {0};
    vkCmdBindVertexBuffers(buffer, 0, 1, &m_resourceManager->mesh(0)->vertexBuffer(), offsets);
    vkCmdBindIndexBuffer(buffer, m_resourceManager->mesh(0)->indexBuffer(), 0, VK_INDEX_TYPE_UINT32);
    
    uint32_t pushConstants[] = {0};
    vkCmdPushConstants(buffer, m_renderPass->pipeline().layout(), VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(uint32_t), pushConstants);
    
    vkCmdDrawIndexed(buffer, m_resourceManager->mesh(0)->indicesCount(), 1, 0, 0, 0);
    
    vkCmdEndRenderPass(buffer);
    vkEndCommandBuffer(buffer);
}
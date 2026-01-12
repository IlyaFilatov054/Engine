#include "render/RenderCore.h"
#include <cmath>
#include <cstdint>
#include <vector>
#include <vulkan/vulkan_core.h>
#include "render/DescriptorManager.h"
#include "render/FrameManager.h"
#include "render/MeshBuffer.h"
#include "render/RenderPass.h"
#include "render/ShaderManager.h"
#include "render/Vertex.h"
#include "core/Utils.h"

RenderCore::RenderCore(const VkContext* context, const Swapchain* swapchain) {
    m_context = context;
    m_swapchain = swapchain;

    m_descriptorManager = new DescriptorManager(m_context);

    m_shaderManager = new ShaderManager(m_context);
    m_shaderManager->createShaderModule(readFile("shaders/main.vert.spv"), "vert");
    m_shaderManager->createShaderModule(readFile("shaders/main.frag.spv"), "frag");

    m_renderPass = new RenderPass(m_context, m_swapchain, m_shaderManager, m_descriptorManager);
    m_frameManager = new FrameManager(m_context, m_swapchain, m_renderPass->renderPass());
    
    m_buffer = new MeshBuffer(m_context, m_vertices.size() * sizeof(Vertex), m_indices.size());
    m_buffer->setVertexData(m_vertices.data());
    m_buffer->setIndexData(m_indices.data());
    camera = new Camera(m_context, m_descriptorManager->cameraDescriptorSet());
    camera->aspect = (float)m_swapchain->extent().width / (float)m_swapchain->extent().height;
}

RenderCore::~RenderCore(){
    vkQueueWaitIdle(m_context->graphicsQueue());
    
    delete camera;
    delete m_buffer;
    delete m_shaderManager;
    delete m_frameManager;
    delete m_renderPass;
    delete m_descriptorManager;
}

float c = 0;
void RenderCore::drawFrame() {
    c += 0.001f;
    //camera->position.y += 5 * std::sin(c);
    camera->position.x = 4 * std::cos(c);
    camera->position.z = 4 * std::sin(c);
    camera->position.y = 4 * std::sin(0.333f * c);
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
    
    vkCmdBindDescriptorSets(buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_renderPass->pipeline().layout(), 0, 1, &m_descriptorManager->cameraDescriptorSet(), 0, nullptr);
    VkDeviceSize offsets[] = {0};
    vkCmdBindVertexBuffers(buffer, 0, 1, &m_buffer->vertexBuffer(), offsets);
    vkCmdBindIndexBuffer(buffer, m_buffer->indexBuffer(), 0, VK_INDEX_TYPE_UINT32);
    vkCmdDrawIndexed(buffer, m_buffer->indicesCount(), 1, 0, 0, 0);
    
    vkCmdEndRenderPass(buffer);
    vkEndCommandBuffer(buffer);
}
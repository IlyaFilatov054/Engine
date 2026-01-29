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
#include "render/RenderSetup.h"
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

    m_shaderManager = new ShaderManager(m_context);
    m_shaderManager->createShaderModule(readFile("shaders/main.vert.spv"), "vert");
    m_shaderManager->createShaderModule(readFile("shaders/main.frag.spv"), "frag");
    m_shaderManager->createShaderModule(readFile("shaders/test.vert.spv"), "testvert");
    m_shaderManager->createShaderModule(readFile("shaders/test.frag.spv"), "testfrag");

    m_renderGraph = new RenderGraph(m_context);

    RenderSetup renderSetup {
        .descriptorSetLayouts {
            DescriptorSetLayoutSetup {
                .handle = 0,
                .bindings = {
                    VkDescriptorSetLayoutBinding {
                        .binding = 0,
                        .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
                        .descriptorCount = 1,
                        .stageFlags = VK_SHADER_STAGE_VERTEX_BIT
                    },            
                }
            },
            DescriptorSetLayoutSetup {
                .handle = 1,
                .bindings = {
                    VkDescriptorSetLayoutBinding {
                        .binding = 0,
                        .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                        .descriptorCount = 16,
                        .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT
                    },            
                }
            },
            DescriptorSetLayoutSetup {
                .handle = 2,
                .bindings = {
                    VkDescriptorSetLayoutBinding {
                        .binding = 0,
                        .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                        .descriptorCount = 1,
                        .stageFlags = VK_SHADER_STAGE_VERTEX_BIT
                    },            
                }
            },
            DescriptorSetLayoutSetup {
                .handle = 3,
                .bindings = {
                    VkDescriptorSetLayoutBinding {
                        .binding = 0,
                        .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                        .descriptorCount = 1,
                        .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT
                    },            
                }
            },
        },
        .descriptorSets = {
            DescriptorSetSetup {
                .handle = 0,
                .layout = 0,
                .perFrame = true
            },
            DescriptorSetSetup {
                .handle = 1,
                .layout = 1,
                .perFrame = false
            },
            DescriptorSetSetup {
                .handle = 2,
                .layout = 2,
                .perFrame = true
            },
            DescriptorSetSetup {
                .handle = 3,
                .layout = 2,
                .perFrame = true
            },
            DescriptorSetSetup {
                .handle = 4,
                .layout = 3,
                .perFrame = true
            },
        },
        .renderPasses {
            RenderPassSetup {
                .hadle = 0,
                .swapchainExtent = true,
                .descriptorSetLayouts = {1, 0, 2},
                .shaders = {
                    ShaderDescriptionSetup {
                        .shaderName = "vert",
                        .stage = VK_SHADER_STAGE_VERTEX_BIT
                    },
                    ShaderDescriptionSetup {
                        .shaderName = "frag",
                        .stage = VK_SHADER_STAGE_FRAGMENT_BIT
                    },
                },
                .attachments {
                    RenderPassAttachmentSetup {
                        .swapchainFormat = true,
                        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
                        .finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                        .referenceLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
                    },
                    RenderPassAttachmentSetup {
                        .format =  VK_FORMAT_D32_SFLOAT,
                        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
                        .finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
                        .referenceLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
                        .depth = true,
                    },
                },
            },
            RenderPassSetup {
                .hadle = 1,
                .swapchainExtent = true,
                .descriptorSetLayouts = {3, 0, 2},
                .shaders = {
                    ShaderDescriptionSetup {
                        .shaderName = "vert",
                        .stage = VK_SHADER_STAGE_VERTEX_BIT
                    },
                    ShaderDescriptionSetup {
                        .shaderName = "testfrag",
                        .stage = VK_SHADER_STAGE_FRAGMENT_BIT
                    },
                },
                .attachments {
                    RenderPassAttachmentSetup {
                        .swapchainFormat = true,
                        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
                        .finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
                        .referenceLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
                    },
                    RenderPassAttachmentSetup {
                        .format = VK_FORMAT_D32_SFLOAT,
                        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
                        .finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
                        .referenceLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
                        .depth = true
                    },
                },
            },
        },
        .imageAttachments {
            ImageAttachmentSetup {
                .handle = 0,
                .swapchainImage = true,
            },
            ImageAttachmentSetup {
                .handle = 1,
                .format = VK_FORMAT_D32_SFLOAT,
                .usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
                .swapchainExtent = true,
                .aspect = VK_IMAGE_ASPECT_DEPTH_BIT 
            },
            ImageAttachmentSetup {
                .handle = 2,
                .format = VK_FORMAT_D32_SFLOAT,
                .usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
                .swapchainExtent = true,
                .aspect = VK_IMAGE_ASPECT_DEPTH_BIT 
            },
            ImageAttachmentSetup {
                .handle = 3,
                .swapchainFormat = true,
                .usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
                .swapchainExtent = true,
                .aspect = VK_IMAGE_ASPECT_COLOR_BIT 
            },
        },
        .writeAttachments {
            WriteAttachmentSetup {
                .handle = 0,
                .renderPass = 1,
                .swapchainExtent = true,
                .imageAttachments = {0, 1}
            },
            WriteAttachmentSetup {
                .handle = 1,
                .renderPass = 0,
                .swapchainExtent = true,
                .imageAttachments = {3, 2}
            },
        },
        .readAttachments {
            ReadAttachmentSetup {
                .handle = 0,
                .descriptorSet = 4,
                .imageAttachments = {3}
            },
        },
        .descriptorAttachments {
            DescriptorAttachmentSetup {
                .handle = 0,
                .descriptorSet = 2,
                .type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                .usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                .bufferSize = sizeof(glm::mat4) * 2
            },
            DescriptorAttachmentSetup {
                .handle = 1,
                .descriptorSet = 3,
                .type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                .usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                .bufferSize = sizeof(glm::mat4) * 2
            },
            DescriptorAttachmentSetup {
                .handle = 2,
                .descriptorSet = 0,
                .type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
                .usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
                .bufferSize = 1024
            }
        },
        .renderGraphNodes {
            RenderGraphNodeSetup {
                .handle = 0,
                .renderPass = 0,
                .outputAttachment = 1,
                .externalDescriptorSets = {1},
                .descriptorAttachments = {2, 0},
                .clearValues = {
                    {.color = {1.0f, 0.0f, 0.0f, 1.0f}},
                    {.depthStencil = {1.0f, 0}},
                },
                .frameRenderData = 0,
                .step = 0
            },
            RenderGraphNodeSetup {
                .handle = 1,
                .renderPass = 1,
                .inputAttachments = {0},
                .outputAttachment = 0,
                .descriptorAttachments = {2, 1},
                .clearValues = {
                    {.color = {0.0f, 0.0f, 0.0f, 1.0f}},
                    {.depthStencil = {1.0f, 0}},
                },
                .frameRenderData = 1,
                .step = 1
            },
        }
    };

    for(auto descriptorSetLayout : renderSetup.descriptorSetLayouts) {
        m_descriptorManager->createLayout(
            descriptorSetLayout.handle,
            descriptorSetLayout.bindings
        );
    }
    for(auto descriptorSet : renderSetup.descriptorSets) {
        m_descriptorManager->allocateSets(
            descriptorSet.handle,
            descriptorSet.layout,
            descriptorSet.perFrame ? m_frameManager->imageCount() : 1
        );
    }
    for(auto renderPass : renderSetup.renderPasses) {
        std::vector<DescriptorSetLayoutReference> descriptorSetLayoutReferences;
        for(auto layout : renderPass.descriptorSetLayouts) {
            DescriptorSetLayoutReference descriptorSetLayoutReference {
                .handle = layout,
                .layout = m_descriptorManager->layout(layout)
            };
            descriptorSetLayoutReferences.push_back(descriptorSetLayoutReference);
        }
        
        std::vector<ShaderDescription> shaderDescriptions;
        for(auto shader : renderPass.shaders) {
            ShaderDescription shaderDescription {
                .module = m_shaderManager->getShaderModule(shader.shaderName),
                .stage = shader.stage
            };
            shaderDescriptions.push_back(shaderDescription);
        }

        std::vector<RenderPassAttachmentDescription> attachmentDescriptions;
        for(auto attachment : renderPass.attachments) {
            RenderPassAttachmentDescription attachmentDescription {
                .format = attachment.swapchainFormat ? m_swapchain->format().format : attachment.format,
                .initialLayout = attachment.initialLayout,
                .finalLayout = attachment.finalLayout,
                .referenceLayout = attachment.referenceLayout,
                .depth = attachment.depth
            };
            attachmentDescriptions.push_back(attachmentDescription);
        }

        RenderPassDescription renderPassDescription {
            .extent = renderPass.swapchainExtent ? m_swapchain->extent() : renderPass.extent,
            .attachments = attachmentDescriptions,
            .shaders = shaderDescriptions,
            .descriptorSetLayouts = descriptorSetLayoutReferences
        };
        m_renderGraph->addRenderPass(
            renderPass.hadle,
            renderPassDescription
        );
    }
    for(auto imageAttachment : renderSetup.imageAttachments) {
        ImageAttachmentDescription imageAttachmentDescription {
            .handle = imageAttachment.handle,
            .format = imageAttachment.swapchainFormat ? m_swapchain->format().format : imageAttachment.format,
            .usage = imageAttachment.usage,
            .extent = imageAttachment.swapchainExtent ? VkExtent3D{m_swapchain->extent().width, m_swapchain->extent().height, 1} : imageAttachment.extent,
            .aspect = imageAttachment.aspect,
        };
        if(imageAttachment.swapchainImage) imageAttachmentDescription.externalImages = m_swapchain->images();
        m_renderGraph->addImageAttachment(imageAttachmentDescription);
    }
    for(auto writeAttachment : renderSetup.writeAttachments) {
        WriteAttachmentDescription writeAttachmentDescription {
            .handle = writeAttachment.handle,
            .renderPass = writeAttachment.renderPass,
            .extent = writeAttachment.swapchainExtent ? m_swapchain->extent() : writeAttachment.extent,
            .imageAttachments = writeAttachment.imageAttachments
        };
        m_renderGraph->addWriteAttachment(writeAttachmentDescription);
    }
    for(auto readAttachment : renderSetup.readAttachments) {
        ReadAttachmentDescription readAttachmentDescription {
            .handle = readAttachment.handle,
            .perFrameDescriptorSets = m_descriptorManager->sets(readAttachment.descriptorSet),
            .imageAttachments = readAttachment.imageAttachments
        };
        m_renderGraph->addReadAttachment(readAttachmentDescription);
    }
    for(auto descriptorAttachment : renderSetup.descriptorAttachments) {
        DescriptorAttachmentDescription descriptorAttachmentDescription {
            .handle = descriptorAttachment.handle,
            .perFrameDescriptorSets = m_descriptorManager->sets(descriptorAttachment.descriptorSet),
            .type = descriptorAttachment.type,
            .usage = descriptorAttachment.usage,
            .bufferSize = descriptorAttachment.bufferSize
        };
        m_renderGraph->addDescriptorAttachment(descriptorAttachmentDescription);
    }
    m_renderGraph->createAttachmentResources(m_frameManager->imageCount());
    for(auto nodeSetup : renderSetup.renderGraphNodes) {
        std::vector<DescriptorSet> externalDescriptors;
        for(auto d : nodeSetup.externalDescriptorSets){
            externalDescriptors.push_back(m_descriptorManager->sets(d)[0]);
        }
        RenderGraphNode node {
            .renderPass = nodeSetup.renderPass,
            .inputAttachments = nodeSetup.inputAttachments,
            .outputAttachment = nodeSetup.outputAttachment,
            .externalDescriptors = externalDescriptors,
            .descriptorAttachments = nodeSetup.descriptorAttachments,
            .clearValues = nodeSetup.clearValues,
            .frameRenderData = nodeSetup.frameRenderData
        };
        m_renderGraph->addNode(nodeSetup.handle, node, nodeSetup.step);
    }
    
    m_resourceManager = new ResourceManager(m_context, m_descriptorManager->sets(1).front().descriptor);
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
    m_renderGraph->bindDescriptorAttachmentDataSource(0, [&](){return camera1->data();});


    camera2 = new Camera(m_context);
    camera2->aspect = (float)m_swapchain->extent().width / (float)m_swapchain->extent().height;
    m_renderGraph->bindDescriptorAttachmentDataSource(1, [&](){return camera2->data();});

    m_renderGraph->bindDescriptorAttachmentDataSource(2, [&](){return m_resourceManager->renderData();});
}

RenderCore::~RenderCore(){
    vkQueueWaitIdle(m_context->graphicsQueue());
    
    delete m_renderGraph;
    delete m_resourceManager;
    delete m_frameManager;
    delete m_descriptorManager;
    delete m_shaderManager;
    
    delete camera1;
    delete camera2;
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
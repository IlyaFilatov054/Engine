#include "render/RenderGraph.h"
#include "render/AttachmentResources.h"
#include "render/RenderPass.h"
#include <cstdint>
#include <stdexcept>
#include <vector>
#include <vulkan/vulkan_core.h>

RenderGraph::RenderGraph(const VkContext* context) : m_context(context) {

}

RenderGraph::~RenderGraph() {
    for(const auto& p : m_renderPasses){
        delete p;
    }
    for(const auto& a : m_attachmentResources){
        delete a;
    }    
}

const RenderPassHandle RenderGraph::addRenderPass(
    const std::vector<VkAttachmentDescription>& attachments,
    const std::vector<VkImageLayout>& attachmentLayouts,
    const VkExtent2D& extent, 
    const std::vector<ShaderDescription>& shaders,
    const std::vector<VkDescriptorSetLayout> usedLayouts
) {
    m_renderPasses.push_back(new RenderPass(m_context, attachments, attachmentLayouts, extent, shaders, usedLayouts));
    return m_renderPasses.size() - 1;
}

const NodeHandle RenderGraph::addNode(const RenderGraphNode& node, uint32_t step) {
    m_nodes.push_back(node);
    uint32_t id = m_nodes.size() - 1;

    auto it = m_steps.find(step);
    if(it == m_steps.end()) m_steps[step] = std::vector<uint32_t>();
    m_steps[step].push_back(id);

    return id;
}

void RenderGraph::execute(const VkCommandBuffer commandBuffer, uint32_t image) {
    for (auto it = m_steps.begin(); it != m_steps.end(); it++) {
        for(auto& n : it->second){
            executeNode(n, commandBuffer, m_attachmentResources[image]);
        }
    }
    m_renderQueue.clear();
}

void RenderGraph::addDrawCall(FrameRenderDataHandle frameRenderDataHandle, DrawCall drawCall) {
    auto it = m_renderQueue.find(frameRenderDataHandle);
    if(it == m_renderQueue.end()) m_renderQueue[frameRenderDataHandle] = std::vector<DrawCall>();
    m_renderQueue[frameRenderDataHandle].push_back(drawCall);
}

const ImageAttachmentHandle RenderGraph::addImageAttachment(ImageAttachmentDescription& description) {
    m_imageAttachmentDescriptions.push_back(description);
    return m_imageAttachmentDescriptions.size() - 1;
}

const WriteAttachmentHandle RenderGraph::addWriteAttachment(WriteAttachmentDescription& description) {
    m_writeAttachmentDescriptions.push_back(description);
    return m_writeAttachmentDescriptions.size() - 1;
}

const ReadAttachmentHandle RenderGraph::addReadAttachment(ReadAttachmentDescription& description) {
    m_readAttachmentDescriptions.push_back(description);
    return m_readAttachmentDescriptions.size() - 1;
}

const DescriptorAttachmentHandle RenderGraph::addDescriptorAttachment(DescriptorAttachmentDescription& description) {
    m_descriptorAttachmentDescriptions.push_back(description);
    return m_descriptorAttachmentDescriptions.size() - 1;
}

void RenderGraph::createAttachmentResources(uint32_t imageCount) {
    for(uint32_t i = 0; i < imageCount; i++) {
        AttachmentResources* resources = new AttachmentResources(m_context);
        for(uint32_t j = 0; j < m_imageAttachmentDescriptions.size(); j++) {
            auto& imageDescription = m_imageAttachmentDescriptions[j];
            ImageAttachmentHandle imageAttachmentHandle;
            if(imageDescription.externalImages.size() > 0) {
                imageAttachmentHandle = resources->addImageAttachment(
                    imageDescription.externalImages[i]
                );
            }
            else {
                imageAttachmentHandle = resources->addImageAttachment(
                    imageDescription.format,
                    imageDescription.usage,
                    imageDescription.extent,
                    imageDescription.aspect
                );
            }
            if(imageAttachmentHandle != j) throw std::runtime_error("Image attachment desync!");
        }
        for(uint32_t j = 0; j < m_writeAttachmentDescriptions.size(); j++) {
            auto& writeDescription = m_writeAttachmentDescriptions[j];
            auto handle = resources->addWriteAttachment(
                m_renderPasses[writeDescription.renderPass]->renderPass(),
                writeDescription.extent,
                writeDescription.imageAttachments
            );
            if(handle != j) throw std::runtime_error("Write attachment desync!");
        }
        for(uint32_t j = 0; j < m_readAttachmentDescriptions.size(); j++) {
            auto& readDescription = m_readAttachmentDescriptions[j];
            auto handle = resources->addReadAttachment(
                readDescription.perFrameDescriptors[i],
                readDescription.imageAttachments
            );
            if(handle != j) throw std::runtime_error("Read attachment desync!");
        }
        for(uint32_t j = 0; j < m_descriptorAttachmentDescriptions.size(); j++) {
            auto& descriptorDescription = m_descriptorAttachmentDescriptions[j];
            auto handle = resources->addDescriptorAttachment(
                descriptorDescription.perFrameDescriptors[i],
                descriptorDescription.type,
                descriptorDescription.usage,
                descriptorDescription.bufferSize
            );
            if(handle != j) throw std::runtime_error("Descriptor attachment desync!");
        }
        m_attachmentResources.push_back(resources);
    }
}

void RenderGraph::executeNode(NodeHandle nodeHandle, const VkCommandBuffer commandBuffer, const AttachmentResources* attachments) {
    auto& node = m_nodes[nodeHandle];
    auto renderPass = m_renderPasses[node.renderPass];

    for(auto inputAttachmentHandle : node.inputAttachments) {
        auto inputAttachment = attachments->readAttachment(inputAttachmentHandle);
        for (uint32_t i = 0; i < inputAttachment.images.size(); i++) {
            auto inputAttachmentImage = attachments->imageAttachment(inputAttachment.images[i]);
            inputAttachmentImage.image->transitionLayout(
                VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                commandBuffer
            );
        }
    }

    for(auto d : node.descriptorAttachments) {
        m_descriptorAttachmentDescriptions[d].update(commandBuffer, attachments->descriptorAttachment(d).buffer);
    }

    VkRenderPassBeginInfo renderPassInfo {
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
        .renderPass = renderPass->renderPass(),
        .framebuffer = attachments->writeAttachment(node.outputAttachment).framebuffer,
        .renderArea = {.offset = {0, 0}, .extent = renderPass->extent()},
        .clearValueCount = static_cast<uint32_t>(node.clearValues.size()),
        .pClearValues = node.clearValues.data(),
    };
    vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, renderPass->pipeline().pipeline());

    std::vector<VkDescriptorSet> descriptors;
    for(auto o : node.descriptorOrder) {
        switch (o) {
            case DescriptorStage::Const: {
                descriptors.insert(descriptors.end(), node.constDescriptors.begin(), node.constDescriptors.end());
                break;
            }
            case DescriptorStage::Frame: {
                for(uint32_t i = 0; i < node.descriptorAttachments.size(); i++) {
                    descriptors.push_back(attachments->descriptorAttachment(node.descriptorAttachments[i]).descriptor);
                }
                break;
            }
            case DescriptorStage::Input: {
                for(auto inputAttachmentHandle : node.inputAttachments) {
                    descriptors.push_back(attachments->readAttachment(inputAttachmentHandle).descriptor);
                }
                break;
            }
        }
    }
    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, renderPass->pipeline().layout(), 0, descriptors.size(), descriptors.data(), 0, nullptr);

    for(auto drawCall : m_renderQueue[node.frameRenderData]) {
        drawCall(commandBuffer, renderPass, attachments);
    }

    vkCmdEndRenderPass(commandBuffer);
    
    for(uint32_t i = 0; i < attachments->writeAttachment(node.outputAttachment).images.size(); i++) {
        auto imageHandle = attachments->writeAttachment(node.outputAttachment).images[i];
        auto image = attachments->imageAttachment(imageHandle).image;
        image->forceLayout(m_renderPasses[node.renderPass]->outputLayouts()[i]);
    }
}
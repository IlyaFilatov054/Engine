#include "render/RenderGraph.h"
#include "render/AttachmentResources.h"
#include "render/DescriptorManager.h"
#include "render/RenderPass.h"
#include <cstdint>
#include <stdexcept>
#include <vector>
#include <vulkan/vulkan_core.h>

RenderGraph::RenderGraph(const VkContext* context) : m_context(context) {

}

RenderGraph::~RenderGraph() {
    for(const auto& p : m_renderPasses){
        delete p.second;
    }
    for(const auto& a : m_attachmentResources){
        delete a;
    }    
}

void RenderGraph::addRenderPass(
    RenderPassHandle handle,
    const std::vector<VkAttachmentDescription>& attachments,
    const std::vector<VkImageLayout>& attachmentLayouts,
    const VkExtent2D& extent, 
    const std::vector<ShaderDescription>& shaders,
    const std::vector<DescriptorSetLayout> usedLayouts
) {
    if(m_renderPasses.contains(handle)) throw std::runtime_error("Render pass handle already present!");
    m_renderPasses[handle] = new RenderPass(m_context, attachments, attachmentLayouts, extent, shaders, usedLayouts);
}

void RenderGraph::addNode(NodeHandle handle, const RenderGraphNode& node, uint32_t step) {
    if(m_nodes.contains(handle)) throw std::runtime_error("Node handle already present!");
    m_nodes[handle] = node;

    auto it = m_steps.find(step);
    if(it == m_steps.end()) m_steps[step] = std::vector<uint32_t>();
    m_steps[step].push_back(handle);
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

void RenderGraph::addImageAttachment(ImageAttachmentDescription& description) {
    m_imageAttachmentDescriptions.push_back(description);
}

void RenderGraph::addWriteAttachment(WriteAttachmentDescription& description) {
    m_writeAttachmentDescriptions.push_back(description);
}

void RenderGraph::addReadAttachment(ReadAttachmentDescription& description) {
    m_readAttachmentDescriptions.push_back(description);
}

void RenderGraph::addDescriptorAttachment(DescriptorAttachmentDescription& description) {
    m_descriptorAttachmentDescriptions.push_back(description);
}

void RenderGraph::bindDescriptorAttachmentDataSource(DescriptorAttachmentHandle handle, DescriptorUpdateSource source) {
    m_descriptorAttachmentsUpdateSources[handle] = source;
}

void RenderGraph::createAttachmentResources(uint32_t imageCount) {
    for(uint32_t i = 0; i < imageCount; i++) {
        AttachmentResources* resources = new AttachmentResources(m_context);
        for(uint32_t j = 0; j < m_imageAttachmentDescriptions.size(); j++) {
            auto& imageDescription = m_imageAttachmentDescriptions[j];
            if(imageDescription.externalImages.size() > 0) {
                resources->addImageAttachment(
                    imageDescription.handle,
                    imageDescription.externalImages[i]
                );
            }
            else {
                resources->addImageAttachment(
                    imageDescription.handle,
                    imageDescription.format,
                    imageDescription.usage,
                    imageDescription.extent,
                    imageDescription.aspect
                );
            }
        }
        for(uint32_t j = 0; j < m_writeAttachmentDescriptions.size(); j++) {
            auto& writeDescription = m_writeAttachmentDescriptions[j];
            resources->addWriteAttachment(
                writeDescription.handle,
                m_renderPasses[writeDescription.renderPass]->renderPass(),
                writeDescription.extent,
                writeDescription.imageAttachments
            );
        }
        for(uint32_t j = 0; j < m_readAttachmentDescriptions.size(); j++) {
            auto& readDescription = m_readAttachmentDescriptions[j];
            resources->addReadAttachment(
                readDescription.handle,
                readDescription.perFrameDescriptors[i],
                readDescription.imageAttachments
            );
        }
        for(uint32_t j = 0; j < m_descriptorAttachmentDescriptions.size(); j++) {
            auto& descriptorDescription = m_descriptorAttachmentDescriptions[j];
            resources->addDescriptorAttachment(
                descriptorDescription.handle,
                descriptorDescription.perFrameDescriptors[i],
                descriptorDescription.type,
                descriptorDescription.usage,
                descriptorDescription.bufferSize
            );
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
        auto descriptorBuffer = attachments->descriptorAttachment(d).buffer;
        descriptorBuffer->stagingBuffer().setData(m_descriptorAttachmentsUpdateSources[d]());
        descriptorBuffer->flush(commandBuffer);
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

    std::map<DescriptorSetLayoutHandle, VkDescriptorSet> unorderedDescriptors;
    for(auto d : node.externalDescriptors) {
        unorderedDescriptors[d.layout] = d.descriptor;
    }
    for(auto d : node.descriptorAttachments) {
        unorderedDescriptors[attachments->descriptorAttachment(d).descriptor.layout] = attachments->descriptorAttachment(d).descriptor.descriptor;
    }
    for(auto d : node.inputAttachments) {
        unorderedDescriptors[attachments->readAttachment(d).descriptor.layout] = attachments->readAttachment(d).descriptor.descriptor;
    }
    std::vector<VkDescriptorSet> descriptors;
    for(auto d : renderPass->descriptorOrder()) {
        descriptors.push_back(unorderedDescriptors[d]);
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
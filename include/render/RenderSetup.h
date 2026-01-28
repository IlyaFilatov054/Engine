#pragma once

#include "render/AttachmentResources.h"
#include "render/DescriptorManager.h"
#include "render/RenderGraph.h"
#include <cstdint>
#include <string>
#include <vector>
#include <vulkan/vulkan_core.h>

struct DescriptorSetLayoutSetup {
    DescriptorSetLayoutHandle handle;
    std::vector<VkDescriptorSetLayoutBinding> bindings;
};

struct DescriptorSetSetup {
    DescriptorSetHandle handle;
    DescriptorSetLayoutHandle layout;
    bool perFrame = false;
};

struct ShaderDescriptionSetup {
    std::string shaderName;
    VkShaderStageFlagBits stage;
};

struct RenderPassAttachmentSetup {
    VkAttachmentDescription description;
    bool swapchainFormat;
    VkImageLayout referenceLayout;
};

struct RenderPassSetup {
    RenderPassHandle hadle;
    std::vector<DescriptorSetLayoutHandle> descriptorSetLayouts;
    std::vector<ShaderDescriptionSetup> shaders;
    std::vector<RenderPassAttachmentSetup> attachments;
    VkExtent2D extent;
    bool swapchainExtent = false;
};

struct ImageAttachmentSetup {
    ImageAttachmentHandle handle;
    bool swapchainImage = false;
    bool swapchainFormat = false;
    VkFormat format;
    VkImageUsageFlags usage;
    VkExtent3D extent;
    bool swapchainExtent = false;
    VkImageAspectFlags aspect;
};

struct WriteAttachmentSetup {
    WriteAttachmentHandle handle;
    RenderPassHandle renderPass;
    VkExtent2D extent;
    bool swapchainExtent = false;
    std::vector<ImageAttachmentHandle> imageAttachments;
};

struct ReadAttachmentSetup {
    ReadAttachmentHandle handle;
    DescriptorSetHandle descriptors;
    std::vector<ImageAttachmentHandle> imageAttachments;
};

struct DescriptorAttachmentSetup {
    DescriptorAttachmentHandle handle;
    DescriptorSetHandle descriptors;
    VkDescriptorType type;
    VkBufferUsageFlagBits usage;
    uint32_t bufferSize;
};

struct RenderGraphNodeSetup {
    NodeHandle handle;
    RenderPassHandle renderPass;
    std::vector<ReadAttachmentHandle> inputAttachments;
    WriteAttachmentHandle outputAttachment;
    std::vector<DescriptorSetHandle> externalDescriptors;
    std::vector<DescriptorAttachmentHandle> descriptorAttachments;
    std::vector<VkClearValue> clearValues;
    FrameRenderDataHandle frameRenderData;
    uint32_t step;
};

struct RenderSetup {
    std::vector<DescriptorSetLayoutSetup> descriptorSetLayouts;
    std::vector<DescriptorSetSetup> descriptorSets;
    std::vector<RenderPassSetup> renderPasses;
    std::vector<ImageAttachmentSetup> imageAttachments;
    std::vector<WriteAttachmentSetup> writeAttachments;
    std::vector<ReadAttachmentSetup> readAttachments;
    std::vector<DescriptorAttachmentSetup> descriptorAttachments;
    std::vector<RenderGraphNodeSetup> renderGraphNodes;
};
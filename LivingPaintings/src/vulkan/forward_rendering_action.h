#pragma once
#include "buffer.h"
#include "command_buffer.h"
#include "device.h"
#include "fence.h"
#include "image.h"
#include "pipeline.h"
#include "swapchain.h"
#include "vulkan/vulkan.h"
#include <array>
#include <stdexcept>
#include <vector>

class ForwardRenderingAction {

    VkPipelineLayout graphicsPipelineLayout = VK_NULL_HANDLE;
    VkPipeline graphicsPipeline = VK_NULL_HANDLE;
    VkExtent2D extent {};

public:
    void setContext(Pipeline& pipeline, VkExtent2D extent, size_t selectedPipelineIndex);
    void beginRenderPass(VkCommandBuffer& cmdGraphics,
        VkRenderPass& renderPass,
        std::vector<VkFramebuffer>& framebuffers,
        uint32_t currentFrame);
    void recordCommandBuffer(VkCommandBuffer& commandBuffer,
        VkDescriptorSet& descriptorSet,
        VertexBuffer& vertexBuffer,
        IndexBuffer& indexBuffer,
        Data::GraphicsObject& graphicsObject);
    void endRenderPass(VkCommandBuffer& commandBuffer);
};

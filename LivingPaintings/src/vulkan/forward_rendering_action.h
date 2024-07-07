// This is a personal academic project. Dear PVS-Studio, please check it.

// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com

#pragma once
#include "buffer.h"
#include "command_buffer.h"
#include "device.h"
#include "fence.h"
#include "graphics_pipeline.h"
#include "swapchain.h"
#include "vulkan/vulkan.h"
#include <stdexcept>
#include <vector>

class ForwardRenderingAction {

    VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;
    VkPipeline graphicsPipeline = VK_NULL_HANDLE;
    VkExtent2D extent {};

public:
    void setContext(GraphicsPipeline& graphicsPipeline, const VkExtent2D extent);
    void beginRenderPass(VkCommandBuffer& commandBuffer, VkRenderPass& renderPass,
        const std::vector<VkFramebuffer>& framebuffers,
        const uint32_t currentFrame);
    void recordCommandBuffer(VkCommandBuffer& commandBuffer,
        VkDescriptorSet& descriptorSet,
        VertexBuffer& vertexBuffer,
        IndexBuffer& indexBuffer,
        Data::GraphicsObject& graphicsObject);
    void endRenderPass(VkCommandBuffer& commandBuffer);
};

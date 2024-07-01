// This is a personal academic project. Dear PVS-Studio, please check it.

// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com

#pragma once
#include "buffer.h"
#include "command_buffer.h"
#include "device.h"
#include "fence.h"
#include "swapchain.h"
#include "vulkan/vulkan.h"
#include <vector>

struct ForwardRenderingAction {
    void drawFrame(Device device, CommandBuffer commandBuffer, VkPipeline graphicsPipeline, VkRenderPass renderPass, SwapChain swapChain, std::vector<VkFramebuffer> framebuffers, Fence fence, Semaphore imageAvailable, Semaphore renderFinished);
    void beginRenderPass(CommandBuffer commandBuffer, VkPipeline graphicsPipeline, std::vector<VkFramebuffer> framebuffers, VkRenderPass renderPass, SwapChain swapChain);
    void recordCommandBuffer(CommandBuffer commandBuffer, SwapChain swapChain, VkPipelineLayout pipelineLayout, VkDescriptorSet descriptorSet, VertexBuffer vertexBuffer, IndexBuffer indexBuffer, VertexData vertexData);
    void endRenderPass(CommandBuffer commandBuffer, SwapChain swapChain);
};

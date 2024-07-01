// This is a personal academic project. Dear PVS-Studio, please check it.

// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com

#include "forward_rendering_action.h"
#include <stdexcept>

using namespace std;

void ForwardRenderingAction::beginRenderPass(CommandBuffer commandBuffer, VkPipeline graphicsPipeline, vector<VkFramebuffer> framebuffers, VkRenderPass renderPass, SwapChain swapChain)
{
    const VkClearValue clearColor = { { { 1.0f, 1.0f, 1.0f, 1.0f } } };

    const auto currentFrame = swapChain.getCurrentFrame();
    const auto cmd = commandBuffer.get(currentFrame);

    VkRenderPassBeginInfo renderPassBegin {};
    renderPassBegin.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassBegin.renderPass = renderPass;
    renderPassBegin.framebuffer = framebuffers[swapChain.getCurrentFrame()];
    renderPassBegin.renderArea.offset = { 0, 0 };
    renderPassBegin.renderArea.extent = swapChain.getExtent();
    renderPassBegin.clearValueCount = 1;
    renderPassBegin.pClearValues = &clearColor;

    vkCmdBeginRenderPass(cmd, &renderPassBegin, VK_SUBPASS_CONTENTS_INLINE);

    vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline);
}

void ForwardRenderingAction::recordCommandBuffer(CommandBuffer commandBuffer, SwapChain swapChain, VkPipelineLayout pipelineLayout, VkDescriptorSet descriptorSet, VertexBuffer vertexBuffer, IndexBuffer indexBuffer, VertexData vertexData)
{
    const auto extent = swapChain.getExtent();
    const auto cmd = commandBuffer.get(swapChain.getCurrentFrame());

    VkBuffer vertexBuffers[] = { vertexBuffer.get() };
    VkDeviceSize offsets[] = { 0 };
    vkCmdBindVertexBuffers(cmd, 0, 1, vertexBuffers, offsets);

    vkCmdBindIndexBuffer(cmd, indexBuffer.get(), 0, VK_INDEX_TYPE_UINT16);

    VkViewport viewport {};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = (float)extent.width;
    viewport.height = (float)extent.height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(cmd, 0, 1, &viewport);

    VkRect2D scissor {};
    scissor.offset = { 0, 0 };
    scissor.extent = extent;
    vkCmdSetScissor(cmd, 0, 1, &scissor);

    vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &descriptorSet, 0, nullptr);

    vkCmdDrawIndexed(cmd, static_cast<uint32_t>(vertexData.indicies.size()), 1, 0, 0, 0);
}

void ForwardRenderingAction::endRenderPass(CommandBuffer commandBuffer, SwapChain swapChain)
{
    const auto currentFrame = swapChain.getCurrentFrame();
    vkCmdEndRenderPass(commandBuffer.get(currentFrame));
}

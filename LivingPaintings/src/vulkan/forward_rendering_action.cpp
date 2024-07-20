// This is a personal academic project. Dear PVS-Studio, please check it.

// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com

#include "forward_rendering_action.h"

using namespace std;

const VkClearValue clearColor = { { { 0.0f, 0.0f, 0.0f, 0.0f } } };

void ForwardRenderingAction::setContext(Pipeline& pipeline,
    const VkExtent2D extent,
    const size_t selectedPipelineIndex)
{
    this->graphicsPipelineLayout = pipeline.getLayout(selectedPipelineIndex);
    this->graphicsPipeline = pipeline.get(selectedPipelineIndex);
    this->extent = extent;
}

void ForwardRenderingAction::beginRenderPass(
    VkCommandBuffer& cmdGraphics, VkRenderPass& renderPass,
    const std::vector<VkFramebuffer>& framebuffers,
    const uint32_t currentFrame)
{

    VkRenderPassBeginInfo renderPassBegin {};
    renderPassBegin.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassBegin.renderPass = renderPass;
    renderPassBegin.framebuffer = framebuffers[currentFrame];
    renderPassBegin.renderArea.offset = { 0, 0 };
    renderPassBegin.renderArea.extent = extent;
    renderPassBegin.clearValueCount = 1;
    renderPassBegin.pClearValues = &clearColor;

    vkCmdBeginRenderPass(cmdGraphics, &renderPassBegin,
        VK_SUBPASS_CONTENTS_INLINE);

    vkCmdBindPipeline(cmdGraphics, VK_PIPELINE_BIND_POINT_GRAPHICS,
        graphicsPipeline);
}

void ForwardRenderingAction::recordCommandBuffer(VkCommandBuffer& commandBuffer,
    VkDescriptorSet& descriptorSet,
    VertexBuffer& vertexBuffer,
    IndexBuffer& indexBuffer,
    Data::GraphicsObject& graphicsObject)
{
    const VkBuffer vertexBuffers[] = { vertexBuffer.get() };
    const VkDeviceSize offsets[] = { 0 };
    vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);
    vkCmdBindIndexBuffer(commandBuffer, indexBuffer.get(), 0, VK_INDEX_TYPE_UINT16);

    VkViewport viewport {};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = (float)extent.width;
    viewport.height = (float)extent.height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

    VkRect2D scissor {};
    scissor.offset = { 0, 0 };
    scissor.extent = extent;
    vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
        graphicsPipelineLayout, 0, 1, &descriptorSet, 0,
        nullptr);

    vkCmdDrawIndexed(commandBuffer,
        static_cast<uint32_t>(graphicsObject.indicies.size()),
        1, 0, 0, 0);
}

void ForwardRenderingAction::endRenderPass(VkCommandBuffer& commandBuffer)
{
    vkCmdEndRenderPass(commandBuffer);
}

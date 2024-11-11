#include "forward_rendering_action.h"

const std::array<VkClearValue, 2> clearColors {
    VkClearValue { { { 0.0f, 0.0f, 0.0f, 1.0f } } },
    VkClearValue { { { 1.0f, 0.0f } } }
};

void ForwardRenderingAction::setContext(Pipeline& pipeline,
    VkExtent2D extent,
    size_t selectedPipelineIndex)
{
    this->graphicsPipelineLayout = pipeline.getLayout(selectedPipelineIndex);
    this->graphicsPipeline = pipeline.get(selectedPipelineIndex);
    this->extent = extent;
}

void ForwardRenderingAction::beginRenderPass(
    VkCommandBuffer& cmdGraphics, VkRenderPass& renderPass,
    std::vector<VkFramebuffer>& framebuffers,
    uint32_t currentFrame)
{

    VkRenderPassBeginInfo renderPassBegin {};
    renderPassBegin.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassBegin.renderPass = renderPass;
    renderPassBegin.framebuffer = framebuffers[currentFrame];
    renderPassBegin.renderArea.offset = { 0, 0 };
    renderPassBegin.renderArea.extent = extent;
    renderPassBegin.clearValueCount = static_cast<uint32_t>(clearColors.size());
    renderPassBegin.pClearValues = clearColors.data();

    vkCmdBeginRenderPass(cmdGraphics, &renderPassBegin,
        VK_SUBPASS_CONTENTS_INLINE);

    vkCmdBindPipeline(cmdGraphics, VK_PIPELINE_BIND_POINT_GRAPHICS,
        graphicsPipeline);
}

void ForwardRenderingAction::recordCommandBuffer(VkCommandBuffer& commandBuffer,
    VkDescriptorSet& descriptorSet,
    VkDescriptorSet& bindlessDescriptorSet,
    VertexBuffer& vertexBuffer,
    IndexBuffer& indexBuffer,
    Data::GraphicsObject& graphicsObject)
{
    const uint32_t dynamicOffset = graphicsObject.instanceId * static_cast<uint32_t>(Data::AlignmentProperties::dynamicUniformAlignment_mat4);
    const VkBuffer vertexBuffers[] = { vertexBuffer.get() };
    const VkDeviceSize offsets[] = { 0 };
    vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);
    vkCmdBindIndexBuffer(commandBuffer, indexBuffer.get(), 0, VK_INDEX_TYPE_UINT16);

    VkViewport viewport {};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = static_cast<float>(extent.width);
    viewport.height = static_cast<float>(extent.height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

    VkRect2D scissor {};
    scissor.offset = { 0, 0 };
    scissor.extent = extent;
    vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
        graphicsPipelineLayout, 0, 1, &descriptorSet,
        1, &dynamicOffset);

    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
        graphicsPipelineLayout, 1, 1, &bindlessDescriptorSet,
        0, nullptr);

    vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(graphicsObject.indices.size()),
        1, 0, 0, graphicsObject.instanceId);
}

void ForwardRenderingAction::endRenderPass(VkCommandBuffer& commandBuffer)
{
    vkCmdEndRenderPass(commandBuffer);
}

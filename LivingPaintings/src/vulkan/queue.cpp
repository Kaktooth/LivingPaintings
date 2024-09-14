#include "queue.h"

void Queue::create(VkDevice& device, uint8_t queueFamilyIndex)
{
    this->queueFamilyIndex = queueFamilyIndex;
    vkGetDeviceQueue(device, queueFamilyIndex, 0, &queue);
}

void Queue::submit(VkCommandBuffer& commandBuffer, Fence& fence,
    std::vector<VkSemaphore> waitSemafores,
    std::vector<VkSemaphore> signalSemafores,
    std::vector<VkPipelineStageFlags> waitStages,
    size_t currentFrame)
{
    VkSubmitInfo submitInfo {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.waitSemaphoreCount = waitSemafores.size();
    submitInfo.pWaitSemaphores = waitSemafores.data();
    submitInfo.pWaitDstStageMask = waitStages.data();
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;
    submitInfo.signalSemaphoreCount = signalSemafores.size();
    submitInfo.pSignalSemaphores = signalSemafores.data();

    fence.reset(currentFrame);

    if (vkQueueSubmit(queue, 1, &submitInfo, fence.get(currentFrame)) != VK_SUCCESS) {
        throw std::runtime_error("Failed to submit draw command buffer.");
    }
}

void Queue::submit(VkCommandBuffer& commandBuffer)
{
    VkSubmitInfo submitInfo {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    vkQueueSubmit(queue, 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(queue);
}

void Queue::signal(VkSemaphore& semaphore)
{
    VkSubmitInfo submitInfo {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = &semaphore;

    vkQueueSubmit(queue, 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(queue);
}

VkQueue& Queue::get()
{
    return queue;
}

uint8_t Queue::getQueueFamilyIndex() const
{
    return queueFamilyIndex;
}

// This is a personal academic project. Dear PVS-Studio, please check it.

// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com

#include "queue.h"

using namespace std;

void Queue::submit(VkCommandBuffer& commandBuffer, Fence& fence,
    const std::vector<VkSemaphore> waitSemafores,
    const std::vector<VkSemaphore> signalSemafores,
    const std::vector<VkPipelineStageFlags> waitStages,
    const size_t currentFrame)
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
        throw runtime_error("Failed to submit draw command buffer.");
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

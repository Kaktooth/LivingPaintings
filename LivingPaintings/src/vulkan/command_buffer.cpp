// This is a personal academic project. Dear PVS-Studio, please check it.

// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com

#include "command_buffer.h"

using namespace std;

VkCommandBuffer
CommandBuffer::beginSingleTimeCommands(VkDevice& device, VkCommandPool& commandPool)
{
    VkCommandBufferAllocateInfo cmdAllocinfo {};
    cmdAllocinfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    cmdAllocinfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    cmdAllocinfo.commandPool = commandPool;
    cmdAllocinfo.commandBufferCount = 1;

    VkCommandBuffer commandBuffer {};
    vkAllocateCommandBuffers(device, &cmdAllocinfo, &commandBuffer);

    VkCommandBufferBeginInfo commandBufferBeginInfo {};
    commandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    commandBufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkBeginCommandBuffer(commandBuffer, &commandBufferBeginInfo);

    return commandBuffer;
}

void CommandBuffer::endSingleTimeCommands(VkDevice& device, VkCommandPool& commandPool,
    VkCommandBuffer& commandBuffer, Queue& queue)
{
    vkEndCommandBuffer(commandBuffer);

    queue.submit(commandBuffer);

    vkFreeCommandBuffers(device, commandPool, 1, &commandBuffer);
}

void CommandBuffer::create(VkDevice& device, VkCommandPool& commandPool)
{
    commandBuffers.resize(Constants::MAX_FRAMES_IN_FLIGHT);

    VkCommandBufferAllocateInfo commandBufferInfo {};
    commandBufferInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    commandBufferInfo.commandPool = commandPool;
    commandBufferInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    commandBufferInfo.commandBufferCount = Constants::MAX_FRAMES_IN_FLIGHT;

    if (vkAllocateCommandBuffers(device, &commandBufferInfo, commandBuffers.data()) != VK_SUCCESS) {
        throw runtime_error("Failed to allocate command buffers.");
    }
}

void CommandBuffer::begin(const size_t frame)
{
    VkCommandBufferBeginInfo beginInfo {};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    if (vkBeginCommandBuffer(commandBuffers[frame], &beginInfo) != VK_SUCCESS) {
        throw runtime_error("Failed to begin recording command buffer.");
    }
}

void CommandBuffer::end(const size_t frame)
{
    if (vkEndCommandBuffer(commandBuffers[frame]) != VK_SUCCESS) {
        throw runtime_error("Failed to record command buffer.");
    }
}

void CommandBuffer::reset(const size_t frame)
{
    vkResetCommandBuffer(commandBuffers[frame], 0);
}

vector<VkCommandBuffer>& CommandBuffer::get()
{
    return commandBuffers;
}

VkCommandBuffer& CommandBuffer::get(const size_t frame)
{
    return commandBuffers[frame];
}

// This is a personal academic project. Dear PVS-Studio, please check it.

// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com

#include "command_pool.h"
#include <stdexcept>

using namespace std;

void CommandPool::create(VkDevice device, QueueFamily::Indices indicies)
{
    VkCommandPoolCreateInfo commandPoolInfo {};
    commandPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    commandPoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    commandPoolInfo.queueFamilyIndex = indicies.graphicsFamily.value();

    if (vkCreateCommandPool(device, &commandPoolInfo, nullptr, &commandPool) != VK_SUCCESS) {
        throw runtime_error("Failed to create command pool.");
    }
}

void CommandPool::destroy(VkDevice device)
{
    vkDestroyCommandPool(device, commandPool, nullptr);
}

VkCommandPool CommandPool::get()
{
    return commandPool;
}

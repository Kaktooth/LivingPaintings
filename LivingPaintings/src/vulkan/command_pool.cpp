// This is a personal academic project. Dear PVS-Studio, please check it.

// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com

#include "command_pool.h"

using namespace std;

VkCommandPool& CommandPool::create(Device& _device)
{
    this->device = _device.get();

    const QueueFamily::Indices familyQueueIndicies = _device.getQueueFamily().indicies;

    VkCommandPoolCreateInfo commandPoolInfo {};
    commandPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    commandPoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    commandPoolInfo.queueFamilyIndex = familyQueueIndicies.graphicsFamily.value();

    if (vkCreateCommandPool(device, &commandPoolInfo, nullptr, &commandPool) != VK_SUCCESS) {
        throw runtime_error("Failed to create command pool.");
    }

    return commandPool;
}

void CommandPool::destroy()
{
    vkDestroyCommandPool(device, commandPool, nullptr);
}

VkCommandPool& CommandPool::get()
{
    return commandPool;
}

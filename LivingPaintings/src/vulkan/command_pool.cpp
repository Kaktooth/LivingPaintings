#include "command_pool.h"

VkCommandPool& CommandPool::create(Device& _device, uint8_t queueFamilyIndex)
{
    this->device = _device.get();

    VkCommandPoolCreateInfo commandPoolInfo {};
    commandPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    commandPoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    commandPoolInfo.queueFamilyIndex = queueFamilyIndex;

    if (vkCreateCommandPool(device, &commandPoolInfo, nullptr, &commandPool) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create command pool.");
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

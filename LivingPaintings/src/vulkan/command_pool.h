#pragma once
#include "device.h"
#include "vulkan/vulkan.h"
#include <stdexcept>

class CommandPool {

    VkCommandPool commandPool = VK_NULL_HANDLE;
    VkDevice device = VK_NULL_HANDLE;

public:
    VkCommandPool& create(VkDevice& device);
    VkCommandPool& create(VkDevice& device, uint8_t queueFamilyIndex);
    void destroy();
    VkCommandPool& get();
};

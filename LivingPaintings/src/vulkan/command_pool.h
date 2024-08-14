// This is a personal academic project. Dear PVS-Studio, please check it.

// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com

#pragma once
#include "device.h"
#include "vulkan/vulkan.h"
#include <stdexcept>

class CommandPool {

    VkCommandPool commandPool = VK_NULL_HANDLE;
    VkDevice device = VK_NULL_HANDLE;

public:
    VkCommandPool& create(Device& device, uint8_t queueFamilyIndex);
    void destroy();
    VkCommandPool& get();
};

// This is a personal academic project. Dear PVS-Studio, please check it.

// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com

#pragma once
#include "vulkan/vulkan.h"

class Sampler {

    VkSampler sampler = VK_NULL_HANDLE;
    VkDevice device = VK_NULL_HANDLE;

public:
    void create(VkDevice& device, VkPhysicalDevice& physicalDevice);
    void destroy();
    VkSampler& get();
};

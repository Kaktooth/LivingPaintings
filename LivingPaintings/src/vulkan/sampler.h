#pragma once
#include "vulkan/vulkan.h"
#include <stdexcept>

class Sampler {

    VkSampler sampler = VK_NULL_HANDLE;
    VkDevice device = VK_NULL_HANDLE;

public:
    void create(VkDevice& device, VkPhysicalDevice& physicalDevice);
    void destroy();
    VkSampler& get();
};

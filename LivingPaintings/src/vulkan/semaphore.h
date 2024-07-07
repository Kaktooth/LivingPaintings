// This is a personal academic project. Dear PVS-Studio, please check it.

// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com

#pragma once
#include "consts.h"
#include "vulkan/vulkan.h"
#include <vector>

class Semaphore {

    VkDevice device;
    std::vector<VkSemaphore> semaphores;

public:
    void create(VkDevice& device);
    void destroy();
    std::vector<VkSemaphore>& get();
    VkSemaphore& get(const size_t frame);
};

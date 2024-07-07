// This is a personal academic project. Dear PVS-Studio, please check it.

// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com

#pragma once
#include "consts.h"
#include "vulkan/vulkan.h"
#include <vector>
class Fence {

    std::vector<VkFence> fences;
    VkDevice device;

public:
    void create(VkDevice& device, const bool signaled);
    void wait(const size_t frame);
    void reset(const size_t frame);
    void destroy();
    VkFence& get(const size_t frame);
};

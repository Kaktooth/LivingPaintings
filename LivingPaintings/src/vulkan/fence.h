#pragma once
#include "consts.h"
#include "vulkan/vulkan.h"
#include <stdexcept>
#include <vector>

class Fence {

    std::vector<VkFence> fences;
    VkDevice device;

public:
    void create(VkDevice& device, bool signaled);
    void wait(size_t frame);
    void reset(size_t frame);
    void destroy();
    VkFence& get(size_t frame);
};

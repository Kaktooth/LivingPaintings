#pragma once
#include "consts.h"
#include "vulkan/vulkan.h"
#include <stdexcept>
#include <vector>

class Semaphore {

    VkDevice device;
    std::vector<VkSemaphore> semaphores;

public:
    void create(VkDevice& device);
    void destroy();
    std::vector<VkSemaphore>& get();
    VkSemaphore& get(size_t frame);
};

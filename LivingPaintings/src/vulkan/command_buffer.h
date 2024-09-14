#pragma once
#include "command_pool.h"
#include "consts.h"
#include "queue.h"
#include <stdexcept>
#include <vector>

class CommandBuffer {

    std::vector<VkCommandBuffer> commandBuffers;

public:
    static VkCommandBuffer beginSingleTimeCommands(VkDevice& device, VkCommandPool& commandPool);
    static void endSingleTimeCommands(VkDevice& device, VkCommandPool& commandPool,
        VkCommandBuffer& commandBuffer,
        Queue& queue);
    void create(VkDevice& device, VkCommandPool& commandPool);
    void begin(size_t frame);
    void end(size_t frame);
    void reset(size_t frame);
    std::vector<VkCommandBuffer>& get();
    VkCommandBuffer& get(size_t frame);
};

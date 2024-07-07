// This is a personal academic project. Dear PVS-Studio, please check it.

// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com

#pragma once
#include "command_pool.h"
#include "consts.h"
#include "queue.h"

class CommandBuffer {

    std::vector<VkCommandBuffer> commandBuffers;

public:
    static VkCommandBuffer beginSingleTimeCommands(VkDevice& device, VkCommandPool& commandPool);
    static void endSingleTimeCommands(VkDevice& device, VkCommandPool& commandPool,
        VkCommandBuffer& commandBuffer,
        Queue& queue);
    void create(VkDevice& device, VkCommandPool& commandPool);
    void begin(const size_t frame);
    void end(const size_t frame);
    void reset(const size_t frame);
    std::vector<VkCommandBuffer>& get();
    VkCommandBuffer& get(const size_t frame);
};

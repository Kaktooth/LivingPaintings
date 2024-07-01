// This is a personal academic project. Dear PVS-Studio, please check it.

// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com

#pragma once
#include "fence.h"
#include "vulkan/vulkan.h"
#include <vector>

class Queue {

    VkQueue queue;

public:
    void submit(VkCommandBuffer commandBuffer, Fence fence, std::vector<VkSemaphore> waitSemafores, std::vector<VkSemaphore> signalSemafores, std::vector<VkPipelineStageFlags> waitStages, size_t currentFrame);
    void submit(VkCommandBuffer commandBuffer);
    VkQueue& get();
};

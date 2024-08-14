// This is a personal academic project. Dear PVS-Studio, please check it.

// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com

#pragma once
#include "fence.h"
#include "vulkan/vulkan.h"
#include <stdexcept>
#include <vector>

class Queue {

    VkQueue queue;
    uint8_t queueFamilyIndex;

public:
    void create(VkDevice& device, uint8_t queueFamilyIndex);
    void submit(VkCommandBuffer& commandBuffer, Fence& fence,
        const std::vector<VkSemaphore> waitSemafores,
        const std::vector<VkSemaphore> signalSemafores,
        const std::vector<VkPipelineStageFlags> waitStages,
        const size_t currentFrame);
    void submit(VkCommandBuffer& commandBuffer);
    void signal(VkSemaphore& semaphore);
    VkQueue& get();
    uint8_t getQueueFamilyIndex();
};

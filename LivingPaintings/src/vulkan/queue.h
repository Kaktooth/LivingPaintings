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
        std::vector<VkSemaphore> waitSemafores,
        std::vector<VkSemaphore> signalSemafores,
        std::vector<VkPipelineStageFlags> waitStages,
        size_t currentFrame);
    void submit(VkCommandBuffer& commandBuffer);
    void signal(VkSemaphore& semaphore);
    VkQueue& get();
    uint8_t getQueueFamilyIndex() const;
};

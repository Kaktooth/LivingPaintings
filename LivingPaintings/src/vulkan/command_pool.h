// This is a personal academic project. Dear PVS-Studio, please check it.

// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com

#pragma once
#include "queue_family.h"
#include "vulkan/vulkan.h"

class CommandPool {

    VkCommandPool commandPool;

public:
    void create(VkDevice device, QueueFamily::Indices indicies);
    void destroy(VkDevice device);
    VkCommandPool get();
};

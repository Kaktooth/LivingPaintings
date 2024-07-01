// This is a personal academic project. Dear PVS-Studio, please check it.

// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com

#pragma once
#include "swapchain.h"
#include "vulkan/vulkan.h"
#include <vector>

class RenderPass {

    VkRenderPass renderPass;

public:
    void create(VkDevice device, VkFormat imageFormat);
    void destroy(VkDevice device, VkPipelineLayout pipelineLayout);
    VkRenderPass& get();
};

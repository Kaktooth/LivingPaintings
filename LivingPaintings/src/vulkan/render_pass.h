// This is a personal academic project. Dear PVS-Studio, please check it.

// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com

#pragma once
#include "swapchain.h"
#include "vulkan/vulkan.h"
#include <array>
#include <stdexcept>
#include <vector>

class RenderPass {

    VkRenderPass renderPass = VK_NULL_HANDLE;
    VkDevice device = VK_NULL_HANDLE;
    VkSampleCountFlagBits samples;

public:
    VkRenderPass& create(VkDevice& device, VkFormat colorFormat,
        VkFormat depthFormat, VkSampleCountFlagBits samples);
    void destroy();
    VkRenderPass& get();
    VkSampleCountFlagBits& getSampleCount();
};

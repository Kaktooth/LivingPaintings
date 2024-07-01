// This is a personal academic project. Dear PVS-Studio, please check it.

// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com

#pragma once
#include "vulkan/vulkan.h"
#include <vector>

struct Framebuffer {
    void create(VkDevice device, std::vector<VkImageView> imageViews, VkExtent2D extent, VkRenderPass renderPass);
    void destroy(VkDevice device);
    std::vector<VkFramebuffer> get();
};

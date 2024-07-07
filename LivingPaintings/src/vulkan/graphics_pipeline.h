// This is a personal academic project. Dear PVS-Studio, please check it.

// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com

#pragma once
#include "shader_manager.h"
#include "vertex_data.h"
#include <vulkan/vulkan_core.h>

class GraphicsPipeline {

    ShaderManager shaderManager;
    VkPipelineLayout layout = VK_NULL_HANDLE;
    VkPipeline graphicsPipeline = VK_NULL_HANDLE;

public:
    void create(VkDevice& device, const VkExtent2D swapChainExtent,
        VkRenderPass& renderPass,
        VkDescriptorSetLayout& descriptorSetLayout);
    void destroy(VkDevice& device);
    VkPipelineLayout& getLayout();
    VkPipeline& get();
};

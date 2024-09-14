#pragma once
#include "shader_manager.h"
#include "vertex_data.h"
#include <vector>
#include <vulkan/vulkan.h>

class Pipeline {

    ShaderManager shaderManager;
    std::vector<VkPipelineLayout> layouts;
    std::vector<VkPipeline> graphicsPipelines;
    std::vector<VkPipeline> computePipelines;
    std::vector<VkPipelineLayout> computePipelineLayouts;
    VkDevice device = VK_NULL_HANDLE;
    VkRenderPass renderPass = VK_NULL_HANDLE;
    VkDescriptorSetLayout descriptorSetLayout = VK_NULL_HANDLE;
    VkExtent2D extent;
    VkSampleCountFlagBits samples;

public:
    void create(VkDevice& device, VkRenderPass& renderPass,
        VkDescriptorSetLayout& descriptorSetLayout,
        const VkExtent2D extent, VkSampleCountFlagBits samples);
    void destroy();
    bool recreateifShadersChanged();
    void bind(VkCommandBuffer& cmdCompute, VkDescriptorSet& computeDescriptorSet);
    void updateExtent(VkExtent2D& extent);
    VkPipelineLayout& getLastLayout();
    VkPipeline& getLast();
    VkPipelineLayout& getLayout(const size_t index);
    VkPipeline& get(const size_t index);
    size_t getPipelineHistorySize();
};

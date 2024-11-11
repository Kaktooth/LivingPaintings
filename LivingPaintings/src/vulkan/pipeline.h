#pragma once
#include "shader_manager.h"
#include "vertex_data.h"
#include <vector>
#include <vulkan/vulkan.h>

class Pipeline {

    ShaderManager shaderManager;
    std::vector<VkPipelineLayout> graphicsPipelineLayouts;
    std::vector<VkPipeline> graphicsPipelines;
    std::vector<VkPipelineLayout> computePipelineLayouts;
    std::vector<VkPipeline> computePipelines;
    VkDevice device = VK_NULL_HANDLE;
    VkRenderPass renderPass = VK_NULL_HANDLE;
    std::vector<VkDescriptorSetLayout> descriptorSetLayouts;
    VkExtent2D extent;
    VkSampleCountFlagBits samples;

public:
    void create(VkDevice& device, VkRenderPass& renderPass,
        std::vector<VkDescriptorSetLayout>& descriptorSetLayouts,
        const VkExtent2D extent, VkSampleCountFlagBits samples);
    void destroy();
    bool recreateifShadersChanged();
    void bind(VkCommandBuffer& cmdCompute, VkDescriptorSet& descriptorSet, VkDescriptorSet& bindlessDescriptorSet);
    void updateExtent(VkExtent2D& extent);
    VkPipelineLayout& getLastLayout();
    VkPipeline& getLast();
    VkPipelineLayout& getLayout(const size_t index);
    VkPipeline& get(const size_t index);
    size_t getPipelineHistorySize();
};

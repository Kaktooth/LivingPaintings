// This is a personal academic project. Dear PVS-Studio, please check it.

// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com

#pragma once
#include "buffer.h"
#include "consts.h"
#include "image.h"
#include "sampler.h"
#include "vulkan/vulkan.h"
#include <stdexcept>

class Descriptor {

    VkDescriptorSetLayout setLayout = VK_NULL_HANDLE;
    VkDescriptorPool pool = VK_NULL_HANDLE;
    std::vector<VkDescriptorSet> sets;

public:
    void create(VkDevice& device, std::vector<UniformBuffer>& uniformBuffers,
        Image& paintingTexture, Image& heightMapTexture,
        Sampler& textureSampler);
    void destroy(VkDevice& device);
    VkDescriptorSetLayout& getSetLayout();
    VkDescriptorPool& getPool();
    VkDescriptorSet& getSet(const uint32_t frame);
};

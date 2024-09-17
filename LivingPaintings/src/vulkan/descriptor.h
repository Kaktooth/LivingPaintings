#pragma once
#include "buffer.h"
#include "consts.h"
#include "image.h"
#include "sampler.h"
#include "controls.h"
#include "gui_params.h"
#include "vulkan/vulkan.h"
#include <stdexcept>

using Constants::MASKS_COUNT;

class Descriptor {

    VkDescriptorSetLayout setLayout = VK_NULL_HANDLE;
    VkDescriptorPool pool = VK_NULL_HANDLE;
    std::vector<VkDescriptorSet> sets;

public:
    void create(VkDevice& device,
                std::vector<UniformBuffer> uniformInstanceBuffers,
                std::vector<UniformBuffer> uniformViewBuffers,
                Image& paintingTexture, Image& heightMapTexture, Sampler& textureSampler,
                UniformBuffer& mouseUniform, std::array<Image, MASKS_COUNT>& selectedPosMasks,
                UniformBuffer& timeUniform, UniformBuffer& effectParamsUniform);
    void destroy(VkDevice& device);
    VkDescriptorSetLayout& getSetLayout();
    VkDescriptorPool& getPool();
    VkDescriptorSet& getSet(uint32_t frame);
};

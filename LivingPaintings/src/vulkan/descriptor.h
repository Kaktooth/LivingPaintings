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

    VkDevice device = VK_NULL_HANDLE;
    VkDescriptorSetLayout setLayout = VK_NULL_HANDLE;
    VkDescriptorSetLayout bindlessSetLayout = VK_NULL_HANDLE;
    VkDescriptorPool pool = VK_NULL_HANDLE;
    VkDescriptorPool bindlessPool = VK_NULL_HANDLE;
    std::vector<VkDescriptorSet> sets;
    std::vector<VkDescriptorSet> bindlessSets;
    VkSampler sampler = VK_NULL_HANDLE;

public:
    void create(VkDevice& device,
                std::vector<UniformBuffer> uniformInstanceBuffers,
                std::vector<UniformBuffer> uniformViewBuffers,
                Image& paintingTexture, Image& heightMapTexture, Sampler& textureSampler,
                UniformBuffer& mouseUniform, std::array<Image, MASKS_COUNT>& selectedPosMasks,
                UniformBuffer& timeUniform, UniformBuffer& effectParamsUniform, UniformBuffer& lightParamsUniform);
    void updateBindlessTexture(Image& textureWrite, uint32_t arrayElementId);
    void updateHeightTexture(Image& heightTexture);
    void updateMaskTextures(std::array<Image, MASKS_COUNT>& maskTextures);
    void destroy();
    VkDescriptorSetLayout& getSetLayout();
    VkDescriptorSetLayout& getBindlessSetLayout();
    VkDescriptorPool& getPool();
    VkDescriptorPool& getBindlessPool();
    VkDescriptorSet& getSet(uint32_t frame);
    VkDescriptorSet& getBindlessSet(uint32_t frame);
};

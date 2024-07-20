// This is a personal academic project. Dear PVS-Studio, please check it.

// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com

#include "descriptor.h"

using namespace std;

void Descriptor::create(VkDevice& device,
    std::vector<UniformBuffer>& uniformBuffers,
    Image& paintingTexture, Image& heightMapTexture,
    Sampler& textureSampler)
{
    VkDescriptorSetLayoutBinding uniformLayoutBinding {};
    uniformLayoutBinding.binding = 0;
    uniformLayoutBinding.descriptorCount = 1;
    uniformLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    uniformLayoutBinding.pImmutableSamplers = nullptr;
    uniformLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

    VkDescriptorSetLayoutBinding samplerLayoutBinding {};
    samplerLayoutBinding.binding = 1;
    samplerLayoutBinding.descriptorCount = 1;
    samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    samplerLayoutBinding.pImmutableSamplers = nullptr;
    samplerLayoutBinding.stageFlags = paintingTexture.getDetails().stageUsage;

    VkDescriptorSetLayoutBinding bumpTextureBinding {};
    bumpTextureBinding.binding = 2;
    bumpTextureBinding.descriptorCount = 1;
    bumpTextureBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
    bumpTextureBinding.pImmutableSamplers = nullptr;
    bumpTextureBinding.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;

    VkDescriptorSetLayoutBinding bumpTextureSamplerBinding {};
    bumpTextureSamplerBinding.binding = 3;
    bumpTextureSamplerBinding.descriptorCount = 1;
    bumpTextureSamplerBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    bumpTextureSamplerBinding.pImmutableSamplers = nullptr;
    bumpTextureSamplerBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

    const vector<VkDescriptorSetLayoutBinding> bindings = {
        uniformLayoutBinding, samplerLayoutBinding, bumpTextureBinding,
        bumpTextureSamplerBinding
    };

    VkDescriptorSetLayoutCreateInfo descriptorSetLayoutInfo {};
    descriptorSetLayoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    descriptorSetLayoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
    descriptorSetLayoutInfo.pBindings = bindings.data();

    if (vkCreateDescriptorSetLayout(device, &descriptorSetLayoutInfo, nullptr, &setLayout) != VK_SUCCESS) {
        throw runtime_error("Failed to create descriptor set layout.");
    }

    vector<VkDescriptorPoolSize> poolSizes(bindings.size());
    poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    poolSizes[0].descriptorCount = static_cast<uint32_t>(Constants::MAX_FRAMES_IN_FLIGHT);
    poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    poolSizes[1].descriptorCount = static_cast<uint32_t>(Constants::MAX_FRAMES_IN_FLIGHT) * 2;
    poolSizes[2].type = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
    poolSizes[2].descriptorCount = static_cast<uint32_t>(Constants::MAX_FRAMES_IN_FLIGHT) * 2;
    poolSizes[3].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    poolSizes[3].descriptorCount = static_cast<uint32_t>(Constants::MAX_FRAMES_IN_FLIGHT) * 2;

    VkDescriptorPoolCreateInfo poolInfo {};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
    poolInfo.pPoolSizes = poolSizes.data();
    poolInfo.maxSets = static_cast<uint32_t>(Constants::MAX_FRAMES_IN_FLIGHT) * 2;
    poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;

    if (vkCreateDescriptorPool(device, &poolInfo, nullptr, &pool) != VK_SUCCESS) {
        throw runtime_error("Failed to create descriptor pool.");
    }

    vector<VkDescriptorSetLayout> layouts(Constants::MAX_FRAMES_IN_FLIGHT, setLayout);
    VkDescriptorSetAllocateInfo descriptorSetAllocInfo {};
    descriptorSetAllocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    descriptorSetAllocInfo.descriptorPool = pool;
    descriptorSetAllocInfo.descriptorSetCount = static_cast<uint32_t>(Constants::MAX_FRAMES_IN_FLIGHT);
    descriptorSetAllocInfo.pSetLayouts = layouts.data();

    sets.resize(Constants::MAX_FRAMES_IN_FLIGHT);
    if (vkAllocateDescriptorSets(device, &descriptorSetAllocInfo, sets.data()) != VK_SUCCESS) {
        throw runtime_error("Failed to allocate descriptor sets.");
    }

    for (size_t i = 0; i < Constants::MAX_FRAMES_IN_FLIGHT; i++) {
        VkDescriptorBufferInfo bufferInfo {};
        bufferInfo.buffer = uniformBuffers[i].get();
        bufferInfo.offset = 0;
        bufferInfo.range = sizeof(Data::GraphicsObject::UniformBufferObject);

        VkDescriptorImageInfo paintingTextureinfo {};
        paintingTextureinfo.imageLayout = paintingTexture.getDetails().layout;
        paintingTextureinfo.imageView = paintingTexture.getView();
        paintingTextureinfo.sampler = textureSampler.get();

        VkDescriptorImageInfo bumpTextureInfo {};
        bumpTextureInfo.imageLayout = heightMapTexture.getDetails().layout;
        bumpTextureInfo.imageView = heightMapTexture.getView();

        VkDescriptorImageInfo bumpTextureSamplerInfo {};
        bumpTextureSamplerInfo.imageLayout = heightMapTexture.getDetails().layout;
        bumpTextureSamplerInfo.imageView = heightMapTexture.getView();
        bumpTextureSamplerInfo.sampler = textureSampler.get();

        vector<VkWriteDescriptorSet> writeDescriptorSets(bindings.size());
        writeDescriptorSets[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        writeDescriptorSets[0].dstSet = sets[i];
        writeDescriptorSets[0].dstBinding = 0;
        writeDescriptorSets[0].dstArrayElement = 0;
        writeDescriptorSets[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        writeDescriptorSets[0].descriptorCount = 1;
        writeDescriptorSets[0].pBufferInfo = &bufferInfo;

        writeDescriptorSets[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        writeDescriptorSets[1].dstSet = sets[i];
        writeDescriptorSets[1].dstBinding = 1;
        writeDescriptorSets[1].dstArrayElement = 0;
        writeDescriptorSets[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        writeDescriptorSets[1].descriptorCount = 1;
        writeDescriptorSets[1].pImageInfo = &paintingTextureinfo;

        writeDescriptorSets[2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        writeDescriptorSets[2].dstSet = sets[i];
        writeDescriptorSets[2].dstBinding = 2;
        writeDescriptorSets[2].dstArrayElement = 0;
        writeDescriptorSets[2].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
        writeDescriptorSets[2].descriptorCount = 1;
        writeDescriptorSets[2].pImageInfo = &bumpTextureInfo;

        writeDescriptorSets[3].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        writeDescriptorSets[3].dstSet = sets[i];
        writeDescriptorSets[3].dstBinding = 3;
        writeDescriptorSets[3].dstArrayElement = 0;
        writeDescriptorSets[3].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        writeDescriptorSets[3].descriptorCount = 1;
        writeDescriptorSets[3].pImageInfo = &bumpTextureSamplerInfo;

        vkUpdateDescriptorSets(device, static_cast<uint32_t>(writeDescriptorSets.size()), writeDescriptorSets.data(), 0, nullptr);
    }
}

void Descriptor::destroy(VkDevice& device)
{
    vkDestroyDescriptorPool(device, pool, nullptr);
    vkDestroyDescriptorSetLayout(device, setLayout, nullptr);
    sets.clear();
}

VkDescriptorSetLayout& Descriptor::getSetLayout()
{
    return setLayout;
}

VkDescriptorPool& Descriptor::getPool()
{
    return pool;
}

VkDescriptorSet& Descriptor::getSet(const uint32_t frame)
{
    return sets[frame];
}

#include "descriptor.h"

using Constants::MAX_FRAMES_IN_FLIGHT;

void Descriptor::create(VkDevice& device,
    std::vector<UniformBuffer> uniformInstanceBuffers,
    std::vector<UniformBuffer> uniformViewBuffers,
    Image& paintingTexture, Image& heightMapTexture, Sampler& textureSampler,
    UniformBuffer& mouseUniform, std::array<Image, MASKS_COUNT>& selectedPosMasks, 
    UniformBuffer& timeUniform, UniformBuffer& effectParamsUniform)
{
    std::vector<VkDescriptorSetLayoutBinding> bindings;

    VkDescriptorSetLayoutBinding instanceLayoutBinding {};
    instanceLayoutBinding.binding = 0;
    instanceLayoutBinding.descriptorCount = 1;
    instanceLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
    instanceLayoutBinding.pImmutableSamplers = nullptr;
    instanceLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    bindings.push_back(instanceLayoutBinding);

    VkDescriptorSetLayoutBinding viewLayoutBinding {};
    viewLayoutBinding.binding = 1;
    viewLayoutBinding.descriptorCount = 1;
    viewLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    viewLayoutBinding.pImmutableSamplers = nullptr;
    viewLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    bindings.push_back(viewLayoutBinding);

    VkDescriptorSetLayoutBinding samplerLayoutBinding {};
    samplerLayoutBinding.binding = 2;
    samplerLayoutBinding.descriptorCount = 1;
    samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    samplerLayoutBinding.pImmutableSamplers = nullptr;
    samplerLayoutBinding.stageFlags = paintingTexture.getDetails().stageUsage;
    bindings.push_back(samplerLayoutBinding);

    VkDescriptorSetLayoutBinding bumpTextureBinding {};
    bumpTextureBinding.binding = 3;
    bumpTextureBinding.descriptorCount = 1;
    bumpTextureBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
    bumpTextureBinding.pImmutableSamplers = nullptr;
    bumpTextureBinding.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
    bindings.push_back(bumpTextureBinding);

    VkDescriptorSetLayoutBinding bumpTextureSamplerBinding {};
    bumpTextureSamplerBinding.binding = 4;
    bumpTextureSamplerBinding.descriptorCount = 1;
    bumpTextureSamplerBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    bumpTextureSamplerBinding.pImmutableSamplers = nullptr;
    bumpTextureSamplerBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    bindings.push_back(bumpTextureSamplerBinding);

    VkDescriptorSetLayoutBinding mousePosLayoutBinding {};
    mousePosLayoutBinding.binding = 5;
    mousePosLayoutBinding.descriptorCount = 1;
    mousePosLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    mousePosLayoutBinding.pImmutableSamplers = nullptr;
    mousePosLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    bindings.push_back(mousePosLayoutBinding);

    VkDescriptorSetLayoutBinding selectedPosMaskLayoutBinding {};
    selectedPosMaskLayoutBinding.binding = 6;
    selectedPosMaskLayoutBinding.descriptorCount = MASKS_COUNT;
    selectedPosMaskLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    selectedPosMaskLayoutBinding.pImmutableSamplers = nullptr;
    selectedPosMaskLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    bindings.push_back(selectedPosMaskLayoutBinding);

    VkDescriptorSetLayoutBinding timeLayoutBinding{};
    timeLayoutBinding.binding = 7;
    timeLayoutBinding.descriptorCount = 1;
    timeLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    timeLayoutBinding.pImmutableSamplers = nullptr;
    timeLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    bindings.push_back(timeLayoutBinding);

    VkDescriptorSetLayoutBinding effectsParamsLayoutBinding{};
    effectsParamsLayoutBinding.binding = 8;
    effectsParamsLayoutBinding.descriptorCount = 1;
    effectsParamsLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    effectsParamsLayoutBinding.pImmutableSamplers = nullptr;
    effectsParamsLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    bindings.push_back(effectsParamsLayoutBinding);

    VkDescriptorSetLayoutCreateInfo descriptorSetLayoutInfo {};
    descriptorSetLayoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    descriptorSetLayoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
    descriptorSetLayoutInfo.pBindings = bindings.data();

    if (vkCreateDescriptorSetLayout(device, &descriptorSetLayoutInfo, nullptr, &setLayout) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create descriptor set layout.");
    }

    std::vector<VkDescriptorPoolSize> poolSizes(bindings.size());
    poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
    poolSizes[0].descriptorCount = static_cast<uint32_t>(Constants::MAX_FRAMES_IN_FLIGHT);
    poolSizes[1].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    poolSizes[1].descriptorCount = static_cast<uint32_t>(Constants::MAX_FRAMES_IN_FLIGHT);
    poolSizes[2].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    poolSizes[2].descriptorCount = static_cast<uint32_t>(Constants::MAX_FRAMES_IN_FLIGHT) * 2;
    poolSizes[3].type = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
    poolSizes[3].descriptorCount = static_cast<uint32_t>(Constants::MAX_FRAMES_IN_FLIGHT);
    poolSizes[4].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    poolSizes[4].descriptorCount = static_cast<uint32_t>(Constants::MAX_FRAMES_IN_FLIGHT);
    poolSizes[5].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    poolSizes[5].descriptorCount = 1;
    poolSizes[6].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    poolSizes[6].descriptorCount = static_cast<uint32_t>(Constants::MAX_FRAMES_IN_FLIGHT) * MASKS_COUNT;
    poolSizes[7].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    poolSizes[7].descriptorCount = 1;
    poolSizes[8].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    poolSizes[8].descriptorCount = 1;

    VkDescriptorPoolCreateInfo poolInfo {};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
    poolInfo.pPoolSizes = poolSizes.data();
    poolInfo.maxSets = bindings.size();
    poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;

    if (vkCreateDescriptorPool(device, &poolInfo, nullptr, &pool) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create descriptor pool.");
    }

    std::vector<VkDescriptorSetLayout> layouts(MAX_FRAMES_IN_FLIGHT, setLayout);
    VkDescriptorSetAllocateInfo descriptorSetAllocInfo {};
    descriptorSetAllocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    descriptorSetAllocInfo.descriptorPool = pool;
    descriptorSetAllocInfo.descriptorSetCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
    descriptorSetAllocInfo.pSetLayouts = layouts.data();

    sets.resize(MAX_FRAMES_IN_FLIGHT);
    if (vkAllocateDescriptorSets(device, &descriptorSetAllocInfo, sets.data()) != VK_SUCCESS) {
        throw std::runtime_error("Failed to allocate descriptor sets.");
    }

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        VkDescriptorBufferInfo instanceBufferInfo {};
        instanceBufferInfo.buffer = uniformInstanceBuffers[i].get();
        instanceBufferInfo.offset = 0;
        instanceBufferInfo.range = Data::AlignmentProperties::dynamicUniformAlignment;

        VkDescriptorBufferInfo viewBufferInfo {};
        viewBufferInfo.buffer = uniformViewBuffers[i].get();
        viewBufferInfo.offset = 0;
        viewBufferInfo.range = sizeof(Data::GraphicsObject::ViewUbo);

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

        VkDescriptorBufferInfo mousePosBufferInfo{};
        mousePosBufferInfo.buffer = mouseUniform.get();
        mousePosBufferInfo.offset = 0;
        mousePosBufferInfo.range = sizeof(Controls::MouseControl);

        std::array<VkDescriptorImageInfo, MASKS_COUNT> selectedPosMaskInfo {};
        for (uint16_t maskIndex = 0; maskIndex < MASKS_COUNT; maskIndex++) {
            selectedPosMaskInfo[maskIndex].imageLayout = selectedPosMasks[maskIndex].getDetails().layout;
            selectedPosMaskInfo[maskIndex].imageView = selectedPosMasks[maskIndex].getView();
            selectedPosMaskInfo[maskIndex].sampler = textureSampler.get();
        }

        VkDescriptorBufferInfo timeBufferInfo{};
        timeBufferInfo.buffer = timeUniform.get();
        timeBufferInfo.offset = 0;
        timeBufferInfo.range = sizeof(float);

        VkDescriptorBufferInfo effectsParamsBufferInfo{};
        effectsParamsBufferInfo.buffer = effectParamsUniform.get();
        effectsParamsBufferInfo.offset = 0;
        effectsParamsBufferInfo.range = sizeof(EffectParams);

        std::vector<VkWriteDescriptorSet> writeDescriptorSets(bindings.size());
        writeDescriptorSets[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        writeDescriptorSets[0].dstSet = sets[i];
        writeDescriptorSets[0].dstBinding = 0;
        writeDescriptorSets[0].dstArrayElement = 0;
        writeDescriptorSets[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
        writeDescriptorSets[0].descriptorCount = 1;
        writeDescriptorSets[0].pBufferInfo = &instanceBufferInfo;

        writeDescriptorSets[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        writeDescriptorSets[1].dstSet = sets[i];
        writeDescriptorSets[1].dstBinding = 1;
        writeDescriptorSets[1].dstArrayElement = 0;
        writeDescriptorSets[1].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        writeDescriptorSets[1].descriptorCount = 1;
        writeDescriptorSets[1].pBufferInfo = &viewBufferInfo;

        writeDescriptorSets[2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        writeDescriptorSets[2].dstSet = sets[i];
        writeDescriptorSets[2].dstBinding = 2;
        writeDescriptorSets[2].dstArrayElement = 0;
        writeDescriptorSets[2].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        writeDescriptorSets[2].descriptorCount = 1;
        writeDescriptorSets[2].pImageInfo = &paintingTextureinfo;

        writeDescriptorSets[3].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        writeDescriptorSets[3].dstSet = sets[i];
        writeDescriptorSets[3].dstBinding = 3;
        writeDescriptorSets[3].dstArrayElement = 0;
        writeDescriptorSets[3].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
        writeDescriptorSets[3].descriptorCount = 1;
        writeDescriptorSets[3].pImageInfo = &bumpTextureInfo;

        writeDescriptorSets[4].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        writeDescriptorSets[4].dstSet = sets[i];
        writeDescriptorSets[4].dstBinding = 4;
        writeDescriptorSets[4].dstArrayElement = 0;
        writeDescriptorSets[4].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        writeDescriptorSets[4].descriptorCount = 1;
        writeDescriptorSets[4].pImageInfo = &bumpTextureSamplerInfo;

        writeDescriptorSets[5].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        writeDescriptorSets[5].dstSet = sets[i];
        writeDescriptorSets[5].dstBinding = 5;
        writeDescriptorSets[5].dstArrayElement = 0;
        writeDescriptorSets[5].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        writeDescriptorSets[5].descriptorCount = 1;
        writeDescriptorSets[5].pBufferInfo = &mousePosBufferInfo;

        writeDescriptorSets[6].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        writeDescriptorSets[6].dstSet = sets[i];
        writeDescriptorSets[6].dstBinding = 6;
        writeDescriptorSets[6].dstArrayElement = 0;
        writeDescriptorSets[6].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        writeDescriptorSets[6].descriptorCount = MASKS_COUNT;
        writeDescriptorSets[6].pImageInfo = selectedPosMaskInfo.data();

        writeDescriptorSets[7].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        writeDescriptorSets[7].dstSet = sets[i];
        writeDescriptorSets[7].dstBinding = 7;
        writeDescriptorSets[7].dstArrayElement = 0;
        writeDescriptorSets[7].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        writeDescriptorSets[7].descriptorCount = 1;
        writeDescriptorSets[7].pBufferInfo = &timeBufferInfo;

        writeDescriptorSets[8].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        writeDescriptorSets[8].dstSet = sets[i];
        writeDescriptorSets[8].dstBinding = 8;
        writeDescriptorSets[8].dstArrayElement = 0;
        writeDescriptorSets[8].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        writeDescriptorSets[8].descriptorCount = 1;
        writeDescriptorSets[8].pBufferInfo = &effectsParamsBufferInfo;

        vkUpdateDescriptorSets(device, static_cast<uint32_t>(writeDescriptorSets.size()),
                               writeDescriptorSets.data(), 0, nullptr);
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

VkDescriptorSet& Descriptor::getSet(uint32_t frame)
{
    return sets[frame];
}

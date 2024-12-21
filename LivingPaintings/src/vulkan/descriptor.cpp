#include "descriptor.h"

using Constants::MAX_FRAMES_IN_FLIGHT;
using Constants::MAX_BINDLESS_RESOURCES;

void Descriptor::create(VkDevice& device,
	std::vector<UniformBuffer> uniformInstanceBuffers,
	std::vector<UniformBuffer> uniformViewBuffers,
	Image& paintingTexture, Image& heightMapTexture, Sampler& textureSampler,
	UniformBuffer& mouseUniform, std::array<Image, MASKS_COUNT>& selectedPosMasks,
	UniformBuffer& timeUniform, UniformBuffer& effectParamsUniform, UniformBuffer& lightParamsUniform)
{
	this->device = device;
	this->sampler = textureSampler.get();

	VkDescriptorBindingFlags bindlessFlags = VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT_EXT
		| VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT_EXT
		| VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT_EXT;

	std::vector<VkDescriptorSetLayoutBinding> bindings;
	std::vector<VkDescriptorSetLayoutBinding> bindlessTexturesBindings;

	VkDescriptorSetLayoutBinding instanceLayoutBinding{};
	instanceLayoutBinding.binding = 0;
	instanceLayoutBinding.descriptorCount = 1;
	instanceLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
	instanceLayoutBinding.pImmutableSamplers = nullptr;
	instanceLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
	bindings.push_back(instanceLayoutBinding);

	VkDescriptorSetLayoutBinding viewLayoutBinding{};
	viewLayoutBinding.binding = 1;
	viewLayoutBinding.descriptorCount = 1;
	viewLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	viewLayoutBinding.pImmutableSamplers = nullptr;
	viewLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
	bindings.push_back(viewLayoutBinding);

	VkDescriptorSetLayoutBinding bumpTextureBinding{};
	bumpTextureBinding.binding = 2;
	bumpTextureBinding.descriptorCount = 1;
	bumpTextureBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
	bumpTextureBinding.pImmutableSamplers = nullptr;
	bumpTextureBinding.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
	bindings.push_back(bumpTextureBinding);

	VkDescriptorSetLayoutBinding bumpTextureSamplerBinding{};
	bumpTextureSamplerBinding.binding = 3;
	bumpTextureSamplerBinding.descriptorCount = 1;
	bumpTextureSamplerBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	bumpTextureSamplerBinding.pImmutableSamplers = nullptr;
	bumpTextureSamplerBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
	bindings.push_back(bumpTextureSamplerBinding);

	VkDescriptorSetLayoutBinding mousePosLayoutBinding{};
	mousePosLayoutBinding.binding = 4;
	mousePosLayoutBinding.descriptorCount = 1;
	mousePosLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	mousePosLayoutBinding.pImmutableSamplers = nullptr;
	mousePosLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
	bindings.push_back(mousePosLayoutBinding);

	VkDescriptorSetLayoutBinding selectedPosMaskLayoutBinding{};
	selectedPosMaskLayoutBinding.binding = 5;
	selectedPosMaskLayoutBinding.descriptorCount = MASKS_COUNT;
	selectedPosMaskLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	selectedPosMaskLayoutBinding.pImmutableSamplers = nullptr;
	selectedPosMaskLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
	bindings.push_back(selectedPosMaskLayoutBinding);

	VkDescriptorSetLayoutBinding timeLayoutBinding{};
	timeLayoutBinding.binding = 6;
	timeLayoutBinding.descriptorCount = 1;
	timeLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	timeLayoutBinding.pImmutableSamplers = nullptr;
	timeLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
	bindings.push_back(timeLayoutBinding);

	VkDescriptorSetLayoutBinding effectsParamsLayoutBinding{};
	effectsParamsLayoutBinding.binding = 7;
	effectsParamsLayoutBinding.descriptorCount = 1;
	effectsParamsLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	effectsParamsLayoutBinding.pImmutableSamplers = nullptr;
	effectsParamsLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
	bindings.push_back(effectsParamsLayoutBinding);

	VkDescriptorSetLayoutBinding lightParamsLayoutBinding{};
	lightParamsLayoutBinding.binding = 8;
	lightParamsLayoutBinding.descriptorCount = 1;
	lightParamsLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	lightParamsLayoutBinding.pImmutableSamplers = nullptr;
	lightParamsLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
	bindings.push_back(lightParamsLayoutBinding);

	VkDescriptorSetLayoutCreateInfo descriptorSetLayoutInfo{};
	descriptorSetLayoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	descriptorSetLayoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
	descriptorSetLayoutInfo.pBindings = bindings.data();

	VkDescriptorSetLayoutBinding paintingTextureLayoutBinding{};
	paintingTextureLayoutBinding.binding = 0;
	paintingTextureLayoutBinding.descriptorCount = MAX_BINDLESS_RESOURCES;
	paintingTextureLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	paintingTextureLayoutBinding.pImmutableSamplers = nullptr;
	paintingTextureLayoutBinding.stageFlags = paintingTexture.imageDetails.stageUsage;
	bindlessTexturesBindings.push_back(paintingTextureLayoutBinding);

	VkDescriptorSetLayoutBindingFlagsCreateInfo bindingExtendedInfo = {
	   VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO,
	   nullptr
	};
	bindingExtendedInfo.bindingCount = static_cast<uint32_t>(bindlessTexturesBindings.size());
	bindingExtendedInfo.pBindingFlags = &bindlessFlags;

	VkDescriptorSetLayoutCreateInfo bindlessDescriptorSetLayoutInfo{};
	bindlessDescriptorSetLayoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	bindlessDescriptorSetLayoutInfo.bindingCount = static_cast<uint32_t>(bindlessTexturesBindings.size());
	bindlessDescriptorSetLayoutInfo.pBindings = bindlessTexturesBindings.data();
	bindlessDescriptorSetLayoutInfo.flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT;
	bindlessDescriptorSetLayoutInfo.pNext = &bindingExtendedInfo;

	if (vkCreateDescriptorSetLayout(device, &descriptorSetLayoutInfo, nullptr, &setLayout) != VK_SUCCESS
		|| vkCreateDescriptorSetLayout(device, &bindlessDescriptorSetLayoutInfo, nullptr, &bindlessSetLayout) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create descriptor set layouts.");
	}

	std::vector<VkDescriptorPoolSize> poolSizes(bindings.size());
	poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
	poolSizes[0].descriptorCount = static_cast<uint32_t>(Constants::MAX_FRAMES_IN_FLIGHT) * 2;
	poolSizes[1].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	poolSizes[1].descriptorCount = static_cast<uint32_t>(Constants::MAX_FRAMES_IN_FLIGHT) * 2;
	poolSizes[2].type = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
	poolSizes[2].descriptorCount = static_cast<uint32_t>(Constants::MAX_FRAMES_IN_FLIGHT) * 2;
	poolSizes[3].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	poolSizes[3].descriptorCount = static_cast<uint32_t>(Constants::MAX_FRAMES_IN_FLIGHT) * 2;
	poolSizes[4].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	poolSizes[4].descriptorCount = 1;
	poolSizes[5].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	poolSizes[5].descriptorCount = static_cast<uint32_t>(Constants::MAX_FRAMES_IN_FLIGHT) * MASKS_COUNT;
	poolSizes[6].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	poolSizes[6].descriptorCount = 1;
	poolSizes[7].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	poolSizes[7].descriptorCount = 1;
	poolSizes[8].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	poolSizes[8].descriptorCount = 1;

	VkDescriptorPoolCreateInfo poolInfo{};
	poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
	poolInfo.pPoolSizes = poolSizes.data();
	poolInfo.maxSets = bindings.size() + MASKS_COUNT;

	std::vector<VkDescriptorPoolSize> bindlessPoolSizes(bindlessTexturesBindings.size());
	bindlessPoolSizes[0].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	bindlessPoolSizes[0].descriptorCount = MAX_BINDLESS_RESOURCES;

	VkDescriptorPoolCreateInfo bindlessPoolInfo{};
	bindlessPoolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	bindlessPoolInfo.poolSizeCount = static_cast<uint32_t>(bindlessPoolSizes.size());
	bindlessPoolInfo.pPoolSizes = bindlessPoolSizes.data();
	bindlessPoolInfo.maxSets = bindlessTexturesBindings.size() * MAX_BINDLESS_RESOURCES;
	bindlessPoolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT;

	if (vkCreateDescriptorPool(device, &poolInfo, nullptr, &pool) != VK_SUCCESS
		|| vkCreateDescriptorPool(device, &bindlessPoolInfo, nullptr, &bindlessPool) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create descriptor pools.");
	}

	std::vector<VkDescriptorSetLayout> layouts(MAX_FRAMES_IN_FLIGHT, setLayout);
	VkDescriptorSetAllocateInfo descriptorSetAllocInfo{};
	descriptorSetAllocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	descriptorSetAllocInfo.descriptorPool = pool;
	descriptorSetAllocInfo.descriptorSetCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
	descriptorSetAllocInfo.pSetLayouts = layouts.data();

	std::vector<VkDescriptorSetLayout> bindlessLayouts(1, bindlessSetLayout);
	uint32_t maxBindings = MAX_BINDLESS_RESOURCES - 1;
	VkDescriptorSetVariableDescriptorCountAllocateInfo countInfo{ VK_STRUCTURE_TYPE_DESCRIPTOR_SET_VARIABLE_DESCRIPTOR_COUNT_ALLOCATE_INFO };
	countInfo.descriptorSetCount = 1;
	countInfo.pDescriptorCounts = &maxBindings;

	VkDescriptorSetAllocateInfo bindlessDescriptorSetAllocInfo{};
	bindlessDescriptorSetAllocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	bindlessDescriptorSetAllocInfo.descriptorPool = bindlessPool;
	bindlessDescriptorSetAllocInfo.descriptorSetCount = 1;
	bindlessDescriptorSetAllocInfo.pSetLayouts = bindlessLayouts.data();
	bindlessDescriptorSetAllocInfo.pNext = &countInfo;

	sets.resize(MAX_FRAMES_IN_FLIGHT);
	bindlessSets.resize(1);
	if (vkAllocateDescriptorSets(device, &descriptorSetAllocInfo, sets.data()) != VK_SUCCESS) {
		throw std::runtime_error("Failed to allocate descriptor sets.");
	}

	if (vkAllocateDescriptorSets(device, &bindlessDescriptorSetAllocInfo, bindlessSets.data()) != VK_SUCCESS) {
		throw std::runtime_error("Failed to allocate descriptor sets.");
	}

	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
		VkDescriptorBufferInfo instanceBufferInfo{};
		instanceBufferInfo.buffer = uniformInstanceBuffers[i].get();
		instanceBufferInfo.offset = 0;
		instanceBufferInfo.range = Data::AlignmentProperties::dynamicUniformAlignment_mat4;

		VkDescriptorBufferInfo viewBufferInfo{};
		viewBufferInfo.buffer = uniformViewBuffers[i].get();
		viewBufferInfo.offset = 0;
		viewBufferInfo.range = sizeof(Data::GraphicsObject::View);

		VkDescriptorImageInfo bumpTextureInfo{};
		bumpTextureInfo.imageLayout = heightMapTexture.getDetails().layout;
		bumpTextureInfo.imageView = heightMapTexture.getView();

		VkDescriptorImageInfo bumpTextureSamplerInfo{};
		bumpTextureSamplerInfo.imageLayout = heightMapTexture.getDetails().layout;
		bumpTextureSamplerInfo.imageView = heightMapTexture.getView();
		bumpTextureSamplerInfo.sampler = textureSampler.get();

		VkDescriptorBufferInfo mousePosBufferInfo{};
		mousePosBufferInfo.buffer = mouseUniform.get();
		mousePosBufferInfo.offset = 0;
		mousePosBufferInfo.range = sizeof(Controls::MouseControl);

		std::array<VkDescriptorImageInfo, MASKS_COUNT> selectedPosMaskInfo{};
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

		VkDescriptorBufferInfo lightParamsBufferInfo{};
		lightParamsBufferInfo.buffer = lightParamsUniform.get();
		lightParamsBufferInfo.offset = 0;
		lightParamsBufferInfo.range = sizeof(LightParams);

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

		writeDescriptorSets[4].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		writeDescriptorSets[4].dstSet = sets[i];
		writeDescriptorSets[4].dstBinding = 4;
		writeDescriptorSets[4].dstArrayElement = 0;
		writeDescriptorSets[4].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		writeDescriptorSets[4].descriptorCount = 1;
		writeDescriptorSets[4].pBufferInfo = &mousePosBufferInfo;

		writeDescriptorSets[5].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		writeDescriptorSets[5].dstSet = sets[i];
		writeDescriptorSets[5].dstBinding = 5;
		writeDescriptorSets[5].dstArrayElement = 0;
		writeDescriptorSets[5].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		writeDescriptorSets[5].descriptorCount = MASKS_COUNT;
		writeDescriptorSets[5].pImageInfo = selectedPosMaskInfo.data();

		writeDescriptorSets[6].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		writeDescriptorSets[6].dstSet = sets[i];
		writeDescriptorSets[6].dstBinding = 6;
		writeDescriptorSets[6].dstArrayElement = 0;
		writeDescriptorSets[6].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		writeDescriptorSets[6].descriptorCount = 1;
		writeDescriptorSets[6].pBufferInfo = &timeBufferInfo;

		writeDescriptorSets[7].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		writeDescriptorSets[7].dstSet = sets[i];
		writeDescriptorSets[7].dstBinding = 7;
		writeDescriptorSets[7].dstArrayElement = 0;
		writeDescriptorSets[7].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		writeDescriptorSets[7].descriptorCount = 1;
		writeDescriptorSets[7].pBufferInfo = &effectsParamsBufferInfo;

		writeDescriptorSets[7].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		writeDescriptorSets[7].dstSet = sets[i];
		writeDescriptorSets[7].dstBinding = 7;
		writeDescriptorSets[7].dstArrayElement = 0;
		writeDescriptorSets[7].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		writeDescriptorSets[7].descriptorCount = 1;
		writeDescriptorSets[7].pBufferInfo = &effectsParamsBufferInfo;

		writeDescriptorSets[8].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		writeDescriptorSets[8].dstSet = sets[i];
		writeDescriptorSets[8].dstBinding = 8;
		writeDescriptorSets[8].dstArrayElement = 0;
		writeDescriptorSets[8].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		writeDescriptorSets[8].descriptorCount = 1;
		writeDescriptorSets[8].pBufferInfo = &lightParamsBufferInfo;

		vkUpdateDescriptorSets(device, static_cast<uint32_t>(writeDescriptorSets.size()),
			writeDescriptorSets.data(), 0, nullptr);
	}

	VkDescriptorImageInfo paintingTextureInfo{};
	paintingTextureInfo.imageLayout = paintingTexture.getDetails().layout;
	paintingTextureInfo.imageView = paintingTexture.getView();
	paintingTextureInfo.sampler = textureSampler.get();

	VkWriteDescriptorSet bindlessTextureDescriptorSetWrite = { VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET };
	bindlessTextureDescriptorSetWrite.dstSet = bindlessSets[0];
	bindlessTextureDescriptorSetWrite.dstBinding = 0;
	bindlessTextureDescriptorSetWrite.dstArrayElement = 0;
	bindlessTextureDescriptorSetWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	bindlessTextureDescriptorSetWrite.descriptorCount = 1;
	bindlessTextureDescriptorSetWrite.pImageInfo = &paintingTextureInfo;

	vkUpdateDescriptorSets(device, 1,
		&bindlessTextureDescriptorSetWrite, 0, nullptr);
}

//TODO update resources for many textures and use dstBinding as texture id
void Descriptor::updateBindlessTexture(Image& textureWrite, uint32_t arrayElementId)
{
		uint32_t bindingId = textureWrite.imageDetails.bindingId;
		VkDescriptorImageInfo textureWriteInfo{};
		textureWriteInfo.imageLayout = textureWrite.getDetails().layout;
		textureWriteInfo.imageView = textureWrite.getView();
		if (sampler != VK_NULL_HANDLE) {
			textureWriteInfo.sampler = sampler;
		}

		VkWriteDescriptorSet textureDescriptorSetWrite = { VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET };
		textureDescriptorSetWrite.dstSet = bindlessSets[0];
		textureDescriptorSetWrite.dstBinding = bindingId;
		textureDescriptorSetWrite.dstArrayElement = arrayElementId;
		textureDescriptorSetWrite.descriptorCount = 1;
		textureDescriptorSetWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		textureDescriptorSetWrite.pImageInfo = &textureWriteInfo;
		vkUpdateDescriptorSets(device, 1, &textureDescriptorSetWrite,
			0, nullptr);
}

void Descriptor::updateHeightTexture(Image& heightTexture)
{

	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
		VkDescriptorImageInfo bumpTextureInfo{};
		bumpTextureInfo.imageLayout = heightTexture.getDetails().layout;
		bumpTextureInfo.imageView = heightTexture.getView();

		VkDescriptorImageInfo bumpTextureSamplerInfo{};
		bumpTextureSamplerInfo.imageLayout = heightTexture.getDetails().layout;
		bumpTextureSamplerInfo.imageView = heightTexture.getView();

		if (sampler != VK_NULL_HANDLE) {
			bumpTextureSamplerInfo.sampler = sampler;
		}

		VkWriteDescriptorSet bumpTextureDescriptorSetWrite = { VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET };
		bumpTextureDescriptorSetWrite.dstSet = sets[i];
		bumpTextureDescriptorSetWrite.dstBinding = 2;
		bumpTextureDescriptorSetWrite.dstArrayElement = 0;
		bumpTextureDescriptorSetWrite.descriptorCount = 1;
		bumpTextureDescriptorSetWrite.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
		bumpTextureDescriptorSetWrite.pImageInfo = &bumpTextureInfo;
		vkUpdateDescriptorSets(device, 1, &bumpTextureDescriptorSetWrite,
			0, nullptr);

		VkWriteDescriptorSet bumpSampledTextureDescriptorSetWrite = { VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET };
		bumpSampledTextureDescriptorSetWrite.dstSet = sets[i];
		bumpSampledTextureDescriptorSetWrite.dstBinding = 3;
		bumpSampledTextureDescriptorSetWrite.dstArrayElement = 0;
		bumpSampledTextureDescriptorSetWrite.descriptorCount = 1;
		bumpSampledTextureDescriptorSetWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		bumpSampledTextureDescriptorSetWrite.pImageInfo = &bumpTextureSamplerInfo;
		vkUpdateDescriptorSets(device, 1, &bumpSampledTextureDescriptorSetWrite,
			0, nullptr);
	}
}

void Descriptor::updateMaskTextures(std::array<Image, MASKS_COUNT>& maskTextures)
{
	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
		std::array<VkDescriptorImageInfo, MASKS_COUNT> selectedPosMaskInfo{};
		for (uint16_t maskIndex = 0; maskIndex < MASKS_COUNT; maskIndex++) {
			selectedPosMaskInfo[maskIndex].imageLayout = maskTextures[maskIndex].getDetails().layout;
			selectedPosMaskInfo[maskIndex].imageView = maskTextures[maskIndex].getView();

			if (sampler != VK_NULL_HANDLE) {
				selectedPosMaskInfo[maskIndex].sampler = sampler;
			}
		}

		VkWriteDescriptorSet maskTexturesDescriptorSetWrite = { VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET };
		maskTexturesDescriptorSetWrite.dstSet = sets[i];
		maskTexturesDescriptorSetWrite.dstBinding = 5;
		maskTexturesDescriptorSetWrite.dstArrayElement = 0;
		maskTexturesDescriptorSetWrite.descriptorCount = MASKS_COUNT;
		maskTexturesDescriptorSetWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		maskTexturesDescriptorSetWrite.pImageInfo = selectedPosMaskInfo.data();
		vkUpdateDescriptorSets(device, 1, &maskTexturesDescriptorSetWrite,
			0, nullptr);
	}
}

void Descriptor::destroy()
{
	vkDestroyDescriptorPool(device, pool, nullptr);
	vkDestroyDescriptorPool(device, bindlessPool, nullptr);
	vkDestroyDescriptorSetLayout(device, setLayout, nullptr);
	vkDestroyDescriptorSetLayout(device, bindlessSetLayout, nullptr);
	sets.clear();
	bindlessSets.clear();
}

VkDescriptorSetLayout& Descriptor::getSetLayout()
{
	return setLayout;
}

VkDescriptorSetLayout& Descriptor::getBindlessSetLayout()
{
	return bindlessSetLayout;
}

VkDescriptorPool& Descriptor::getPool()
{
	return pool;
}

VkDescriptorPool& Descriptor::getBindlessPool()
{
	return bindlessPool;
}

VkDescriptorSet& Descriptor::getSet(uint32_t frame)
{
	return sets[frame];
}

VkDescriptorSet& Descriptor::getBindlessSet(uint32_t frame)
{
	return bindlessSets[frame];
}

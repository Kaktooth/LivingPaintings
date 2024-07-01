// This is a personal academic project. Dear PVS-Studio, please check it.

// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com

#pragma once
#include "command_buffer.h"
#include "consts.h"
#include "queue.h"
#include "vertex_data.h"
#include "vulkan/vulkan.h"

#define GLM_FORCE_RADIANS
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include <chrono>
#include <stb_image.h>
#include <vector>

class Buffer {

    VkBuffer buffer;
    VkDeviceMemory deviceMemory;

public:
    void create(VkDevice device, VkPhysicalDevice physicalDevice, unsigned long long size, VkBufferUsageFlags usage, VkSharingMode sharingMode, VkMemoryPropertyFlags memoryPropertyFlags);
    void copyBuffer(VkDevice device, VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size, VkCommandPool commandPool, Queue transferQueue);
    void destroy(VkDevice device);
    VkBuffer get();
    VkDeviceMemory getDeviceMemory();
};

class StagingBuffer : public Buffer {

public:
    void create(VkDevice device, VkPhysicalDevice physicalDevice, stbi_uc* pixels, VkDeviceSize size);
};

class VertexBuffer : public Buffer {

    Buffer stagingBuffer;

public:
    void create(VkDevice device, VkPhysicalDevice physicalDevice, std::vector<VertexData::Vertex> vertecies, VkCommandPool commandPool, CommandBuffer commandBuffer, Queue transferQueue);
};

class IndexBuffer : public Buffer {

    Buffer stagingBuffer;

public:
    void create(VkDevice device, VkPhysicalDevice physicalDevice, std::vector<uint16_t> indicies, VkCommandPool commandPool, CommandBuffer commandBuffer, Queue transferQueue);
};

class UniformBuffer : public Buffer {

    void* mapped;

public:
    void create(VkDevice device, VkPhysicalDevice physicalDevice, VkDeviceSize size);
    void update(VertexData::UniformBufferObject uniformObject);
};

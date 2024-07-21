// This is a personal academic project. Dear PVS-Studio, please check it.

// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com

#pragma once
#include "command_buffer.h"
#include "consts.h"
#include "queue.h"
#include "vertex_data.h"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include <chrono>
#include <stb_image.h>
#include <stdexcept>
#include <vector>

class Buffer {

    VkBuffer buffer = VK_NULL_HANDLE;
    VkDeviceMemory deviceMemory = VK_NULL_HANDLE;
    VkDeviceSize memorySize = 0;

protected:
    VkDevice device = VK_NULL_HANDLE;

public:
    void create(VkDevice& device, VkPhysicalDevice& physicalDevice,
        const unsigned long long size, const VkBufferUsageFlags usage,
        const VkSharingMode sharingMode,
        const VkMemoryPropertyFlags memoryPropertyFlags);
    void copyBuffer(VkCommandPool& commandPool, const VkBuffer srcBuffer,
        const VkBuffer dstBuffer, const VkDeviceSize size,
        Queue& transferQueue);
    void destroy();
    VkBuffer& get();
    VkDeviceMemory& getDeviceMemory();
    VkDeviceSize& getMemorySize();
};

class StagingBuffer : public Buffer {

public:
    void create(VkDevice& device, VkPhysicalDevice& physicalDevice,
        const stbi_uc* pixels, VkDeviceSize& size);
};

class VertexBuffer : public Buffer {

    Buffer stagingBuffer;

public:
    void create(VkDevice& device, VkPhysicalDevice& physicalDevice,
        VkCommandPool& commandPool,
        const std::vector<Data::GraphicsObject::Vertex>& vertecies,
        Queue& transferQueue);
};

class IndexBuffer : public Buffer {

    Buffer stagingBuffer;

public:
    void create(VkDevice& device, VkPhysicalDevice& physicalDevice,
        VkCommandPool& commandPool, const std::vector<uint16_t>& indicies,
        Queue& transferQueue);
};

class UniformBuffer : public Buffer {

    void* mapped;

public:
    void create(VkDevice& device, VkPhysicalDevice& physicalDevice,
        const VkDeviceSize size);
    void update(const Data::GraphicsObject::UniformBufferObject& uniformObject);
};

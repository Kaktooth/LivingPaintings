#pragma once
#include "command_buffer.h"
#include "consts.h"
#include "queue.h"
#include "vertex_data.h"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include <chrono>
#include <stb_image.h>
#include <stdexcept>
#include <vector>

class Buffer {

    VkBuffer buffer = VK_NULL_HANDLE;
    VkDeviceMemory deviceMemory = VK_NULL_HANDLE;

protected:
    VkDeviceSize memorySize = 0;
    VkDevice device = VK_NULL_HANDLE;

public:
    void create(VkDevice& device, VkPhysicalDevice& physicalDevice,
        uint64_t size, VkBufferUsageFlags usage,
        VkSharingMode sharingMode,
        VkMemoryPropertyFlags memoryPropertyFlags);
    void copyBuffer(VkCommandPool& commandPool, VkBuffer srcBuffer,
        VkBuffer dstBuffer, VkDeviceSize size,
        Queue& transferQueue);
    void destroy();
    VkBuffer& get();
    VkDeviceMemory& getDeviceMemory();
    VkDeviceSize& getMemorySize();
};

class StagingBuffer : public Buffer {

public:
    void create(VkDevice& device, VkPhysicalDevice& physicalDevice,
        stbi_uc* pixels, VkDeviceSize size);
};

class VertexBuffer : public Buffer {

    Buffer stagingBuffer;

public:
    void create(VkDevice& device, VkPhysicalDevice& physicalDevice,
        VkCommandPool& commandPool,
        std::vector<Data::GraphicsObject::Vertex>& vertecies,
        Queue& transferQueue);
};

class IndexBuffer : public Buffer {

    Buffer stagingBuffer;

public:
    void create(VkDevice& device, VkPhysicalDevice& physicalDevice,
        VkCommandPool& commandPool, std::vector<uint16_t>& indicies,
        Queue& transferQueue);
};

class UniformBuffer : public Buffer {

    void* mapped;

public:
    void create(VkDevice& device, VkPhysicalDevice& physicalDevice,
        const VkDeviceSize size,
        VkMemoryPropertyFlags memoryProperyFlags);
    void update(const auto& uniformObject);
};

// This is a personal academic project. Dear PVS-Studio, please check it.

// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com

#include "buffer.h"
#include <stdexcept>

using namespace std;

void Buffer::create(VkDevice device, VkPhysicalDevice physicalDevice, unsigned long long size, VkBufferUsageFlags usage, VkSharingMode sharingMode, VkMemoryPropertyFlags memoryPropertyFlags)
{
    VkBufferCreateInfo bufferInfo {};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = size;
    bufferInfo.usage = usage;
    bufferInfo.sharingMode = sharingMode;

    if (vkCreateBuffer(device, &bufferInfo, nullptr, &buffer) != VK_SUCCESS) {
        throw runtime_error("Failed to create buffer.");
    }

    VkMemoryRequirements memoryRequirements;
    vkGetBufferMemoryRequirements(device, buffer, &memoryRequirements);

    VkPhysicalDeviceMemoryProperties memoryProperties;
    vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memoryProperties);

    auto memoryType = [&]() {
        for (uint32_t i = 0; i < memoryProperties.memoryTypeCount; i++) {
            if ((memoryRequirements.memoryTypeBits & (1 << i)) && (memoryProperties.memoryTypes[i].propertyFlags & memoryPropertyFlags) == memoryPropertyFlags) {
                return i;
            }
        }
        throw runtime_error("Failed to find suitable memory type.");
    }();

    VkMemoryAllocateInfo memoryAllocInfo {};
    memoryAllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    memoryAllocInfo.allocationSize = memoryRequirements.size;
    memoryAllocInfo.memoryTypeIndex = memoryType;

    if (vkAllocateMemory(device, &memoryAllocInfo, nullptr, &deviceMemory) != VK_SUCCESS) {
        throw runtime_error("Failed to allocate buffer memory.");
    }

    vkBindBufferMemory(device, buffer, deviceMemory, 0);
}

void Buffer::copyBuffer(VkDevice device, VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size, VkCommandPool commandPool, Queue transferQueue)
{
    auto cmd = CommandBuffer::beginSingleTimeCommands(device, commandPool);
    VkBufferCopy bufferCopy {};
    bufferCopy.size = size;
    vkCmdCopyBuffer(cmd, srcBuffer, dstBuffer, 1, &bufferCopy);
    CommandBuffer::endSingleTimeCommands(device, commandPool, cmd, transferQueue);
}

void Buffer::destroy(VkDevice device)
{
    vkDestroyBuffer(device, buffer, nullptr);
    vkFreeMemory(device, deviceMemory, nullptr);
}

VkBuffer Buffer::get()
{
    return buffer;
}

VkDeviceMemory Buffer::getDeviceMemory()
{
    return deviceMemory;
}

// TODO create stagebuffer and use templates to use different param for memcpy
void StagingBuffer::create(VkDevice device, VkPhysicalDevice physicalDevice, stbi_uc* pixels, VkDeviceSize size)
{
    Buffer::create(device, physicalDevice, size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_SHARING_MODE_EXCLUSIVE, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    void* data;
    vkMapMemory(device, getDeviceMemory(), 0, size, 0, &data);
    memcpy(data, pixels, (size_t)size);
    vkUnmapMemory(device, getDeviceMemory());
}

void VertexBuffer::create(VkDevice device, VkPhysicalDevice physicalDevice, vector<VertexData::Vertex> vertecies, VkCommandPool commandPool, CommandBuffer commandBuffer, Queue transferQueue)
{
    auto size = sizeof(vertecies[0]) * vertecies.size();

    stagingBuffer.create(device, physicalDevice, size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_SHARING_MODE_EXCLUSIVE, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    auto stagingBufferDeviceMemory = stagingBuffer.getDeviceMemory();

    void* data;
    vkMapMemory(device, stagingBufferDeviceMemory, 0, size, 0, &data);
    memcpy(data, vertecies.data(), (size_t)size);
    vkUnmapMemory(device, stagingBufferDeviceMemory);

    Buffer::create(device, physicalDevice, size, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_SHARING_MODE_EXCLUSIVE, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    copyBuffer(device, stagingBuffer.get(), get(), size, commandPool, transferQueue);

    stagingBuffer.destroy(device);
}

void IndexBuffer::create(VkDevice device, VkPhysicalDevice physicalDevice, vector<uint16_t> indicies, VkCommandPool commandPool, CommandBuffer commandBuffer, Queue transferQueue)
{
    auto size = sizeof(indicies[0]) * indicies.size();

    stagingBuffer.create(device, physicalDevice, size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_SHARING_MODE_EXCLUSIVE, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    auto stagingBufferDeviceMemory = stagingBuffer.getDeviceMemory();

    void* data;
    vkMapMemory(device, stagingBufferDeviceMemory, 0, size, 0, &data);
    memcpy(data, indicies.data(), (size_t)size);
    vkUnmapMemory(device, stagingBufferDeviceMemory);

    Buffer::create(device, physicalDevice, size, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_SHARING_MODE_EXCLUSIVE, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    copyBuffer(device, stagingBuffer.get(), get(), size, commandPool, transferQueue);

    stagingBuffer.destroy(device);
}

void UniformBuffer::create(VkDevice device, VkPhysicalDevice physicalDevice, VkDeviceSize size)
{
    Buffer::create(device, physicalDevice, size, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_SHARING_MODE_EXCLUSIVE, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    vkMapMemory(device, getDeviceMemory(), 0, size, 0, &mapped);
}

void UniformBuffer::update(VertexData::UniformBufferObject uniformObject)
{
    memcpy(mapped, &uniformObject, sizeof(uniformObject));
}

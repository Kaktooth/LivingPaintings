// This is a personal academic project. Dear PVS-Studio, please check it.

// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com

#include "buffer.h"

using namespace std;

void Buffer::create(VkDevice& device, VkPhysicalDevice& physicalDevice,
    const unsigned long long size, VkBufferUsageFlags usage,
    VkSharingMode sharingMode,
    VkMemoryPropertyFlags memoryPropertyFlags)
{
    this->device = device;
    this->memorySize = size;

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

    uint32_t memoryType = [&]() {
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

void Buffer::copyBuffer(VkCommandPool& commandPool, VkBuffer srcBuffer,
    VkBuffer dstBuffer, VkDeviceSize size,
    Queue& transferQueue)
{
    VkCommandBuffer cmd = CommandBuffer::beginSingleTimeCommands(device, commandPool);

    VkBufferCopy bufferCopy {};
    bufferCopy.size = size;

    vkCmdCopyBuffer(cmd, srcBuffer, dstBuffer, 1, &bufferCopy);
    CommandBuffer::endSingleTimeCommands(device, commandPool, cmd, transferQueue);
}

void Buffer::destroy()
{
    vkDestroyBuffer(device, buffer, nullptr);
    vkFreeMemory(device, deviceMemory, nullptr);
}

VkBuffer& Buffer::get()
{
    return buffer;
}

VkDeviceMemory& Buffer::getDeviceMemory()
{
    return deviceMemory;
}

VkDeviceSize& Buffer::getMemorySize()
{
    return memorySize;
}

void StagingBuffer::create(VkDevice& device, VkPhysicalDevice& physicalDevice,
    const stbi_uc* pixels, VkDeviceSize& size)
{
    this->device = device;

    Buffer::create(device, physicalDevice, size,
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_SHARING_MODE_EXCLUSIVE,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    void* data;
    const VkDeviceMemory deviceMemory = getDeviceMemory();
    vkMapMemory(device, deviceMemory, 0, size, 0, &data);
    memcpy(data, pixels, (size_t)size);
    vkUnmapMemory(device, deviceMemory);
}

void VertexBuffer::create(VkDevice& device, VkPhysicalDevice& physicalDevice,
    VkCommandPool& commandPool,
    const std::vector<Data::GraphicsObject::Vertex>& vertecies,
    Queue& transferQueue)
{
    this->device = device;

    const unsigned long long size = sizeof(vertecies[0]) * vertecies.size();
    stagingBuffer.create(device, physicalDevice, size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_SHARING_MODE_EXCLUSIVE, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    void* data;
    const VkDeviceMemory stagingBufferDeviceMemory = stagingBuffer.getDeviceMemory();
    vkMapMemory(device, stagingBufferDeviceMemory, 0, size, 0, &data);
    memcpy(data, vertecies.data(), (size_t)size);
    vkUnmapMemory(device, stagingBufferDeviceMemory);

    Buffer::create(
        device, physicalDevice, size,
        VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
        VK_SHARING_MODE_EXCLUSIVE, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    copyBuffer(commandPool, stagingBuffer.get(), get(), size, transferQueue);

    stagingBuffer.destroy();
}

void IndexBuffer::create(VkDevice& device, VkPhysicalDevice& physicalDevice,
    VkCommandPool& commandPool,
    const std::vector<uint16_t>& indicies,
    Queue& transferQueue)
{
    this->device = device;

    const unsigned long long size = sizeof(indicies[0]) * indicies.size();
    stagingBuffer.create(device, physicalDevice, size,
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_SHARING_MODE_EXCLUSIVE,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    void* data;
    const VkDeviceMemory stagingBufferDeviceMemory = stagingBuffer.getDeviceMemory();
    vkMapMemory(device, stagingBufferDeviceMemory, 0, size, 0, &data);
    memcpy(data, indicies.data(), (size_t)size);
    vkUnmapMemory(device, stagingBufferDeviceMemory);

    Buffer::create(
        device, physicalDevice, size,
        VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
        VK_SHARING_MODE_EXCLUSIVE, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    copyBuffer(commandPool, stagingBuffer.get(), get(), size, transferQueue);

    stagingBuffer.destroy();
}

void UniformBuffer::create(VkDevice& device, VkPhysicalDevice& physicalDevice,
    VkDeviceSize size)
{
    Buffer::create(device, physicalDevice, size,
        VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
        VK_SHARING_MODE_EXCLUSIVE,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    vkMapMemory(device, getDeviceMemory(), 0, size, 0, &mapped);
}

void UniformBuffer::update(const Data::GraphicsObject::UniformBufferObject& uniformObject)
{
    memcpy(mapped, &uniformObject, sizeof(uniformObject));
}

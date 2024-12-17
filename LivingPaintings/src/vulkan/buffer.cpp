#include "buffer.h"
#include "controls.h"

void Buffer::create(VkDevice& device, VkPhysicalDevice& physicalDevice,
    const uint64_t size, VkBufferUsageFlags usage,
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
        throw std::runtime_error("Failed to create buffer.");
    }

    VkMemoryRequirements memoryRequirements;
    vkGetBufferMemoryRequirements(device, buffer, &memoryRequirements);

    VkPhysicalDeviceMemoryProperties memoryProperties;
    vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memoryProperties);

    const uint32_t memoryType = [&]() {
        for (uint32_t i = 0; i < memoryProperties.memoryTypeCount; i++) {
            if ((memoryRequirements.memoryTypeBits & (1 << i)) && (memoryProperties.memoryTypes[i].propertyFlags & memoryPropertyFlags) == memoryPropertyFlags) {
                return i;
            }
        }
        throw std::runtime_error("Failed to find suitable memory type.");
    }();

    VkMemoryAllocateInfo memoryAllocInfo {};
    memoryAllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    memoryAllocInfo.allocationSize = memoryRequirements.size;
    memoryAllocInfo.memoryTypeIndex = memoryType;

    if (vkAllocateMemory(device, &memoryAllocInfo, nullptr, &deviceMemory) != VK_SUCCESS) {
        throw std::runtime_error("Failed to allocate buffer memory.");
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
    stbi_uc* pixels, VkDeviceSize size)
{
    this->device = device;

    Buffer::create(device, physicalDevice, size,
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_SHARING_MODE_EXCLUSIVE,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    
    void* data;
    VkDeviceMemory deviceMemory = getDeviceMemory();
    vkMapMemory(device, deviceMemory, 0, size, 0, &data);
    memcpy(data, pixels, static_cast<size_t>(size));
    vkUnmapMemory(device, deviceMemory);
}

void VertexBuffer::create(VkDevice& device, VkPhysicalDevice& physicalDevice,
    VkCommandPool& commandPool,
    std::vector<Data::GraphicsObject::Vertex>& vertecies,
    Queue& transferQueue)
{
    this->device = device;

    const uint64_t size = sizeof(vertecies[0]) * vertecies.size();
    stagingBuffer.create(device, physicalDevice, size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_SHARING_MODE_EXCLUSIVE, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    void* data;
    VkDeviceMemory stagingBufferDeviceMemory = stagingBuffer.getDeviceMemory();
    vkMapMemory(device, stagingBufferDeviceMemory, 0, size, 0, &data);
    memcpy(data, vertecies.data(), static_cast<size_t>(size));
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
    std::vector<uint16_t>& indicies,
    Queue& transferQueue)
{
    this->device = device;

    const uint64_t size = sizeof(indicies[0]) * indicies.size();
    stagingBuffer.create(device, physicalDevice, size,
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_SHARING_MODE_EXCLUSIVE,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    void* data;
    VkDeviceMemory stagingBufferDeviceMemory = stagingBuffer.getDeviceMemory();
    vkMapMemory(device, stagingBufferDeviceMemory, 0, size, 0, &data);
    memcpy(data, indicies.data(), static_cast<size_t>(size));
    vkUnmapMemory(device, stagingBufferDeviceMemory);

    Buffer::create(
        device, physicalDevice, size,
        VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
        VK_SHARING_MODE_EXCLUSIVE, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    copyBuffer(commandPool, stagingBuffer.get(), get(), size, transferQueue);

    stagingBuffer.destroy();
}

void UniformBuffer::create(VkDevice& device, VkPhysicalDevice& physicalDevice,
    VkDeviceSize size,
    VkMemoryPropertyFlags memoryProperyFlags)
{
    Buffer::create(device, physicalDevice, size, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
        VK_SHARING_MODE_EXCLUSIVE, memoryProperyFlags);
    vkMapMemory(device, getDeviceMemory(), 0, size, 0, &mapped);
}

template <>
void UniformBuffer::update(const Data::GraphicsObject::Instance& uniform)
{
    memcpy(mapped, uniform.model, memorySize);
}

template <>
void UniformBuffer::update(const Data::GraphicsObject::View& uniform)
{
    memcpy(mapped, &uniform, sizeof(uniform));
}

template <>
void UniformBuffer::update(const Controls::MouseControl& uniform)
{
    memcpy(mapped, &uniform, sizeof(uniform));
}

template <>
void UniformBuffer::update(const float& uniform)
{
    memcpy(mapped, &uniform, sizeof(uniform));
}

template <>
void UniformBuffer::update(const EffectParams& uniform)
{
    memcpy(mapped, &uniform, sizeof(uniform));
}

template <>
void UniformBuffer::update(const LightParams& uniform)
{
    memcpy(mapped, &uniform, sizeof(uniform));
}

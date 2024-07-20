// This is a personal academic project. Dear PVS-Studio, please check it.

// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com

#include "image.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

using namespace std;

void Image::Details::createImageInfo(const char* filePath, uint16_t width,
    uint16_t height, uint8_t channels, VkImageLayout imageLayout,
    VkImageViewType viewType, VkFormat format,
    int stageUsage,
    VkImageTiling tiling)
{
    this->filePath = filePath;
    this->width = width;
    this->height = height;
    this->layout = imageLayout;
    this->channels = channels;
    this->viewType = viewType;
    this->format = format;
    this->stageUsage = stageUsage;
    this->tiling = tiling;
}

void Image::create(Device& _device, VkCommandPool& commandPool, VkBufferUsageFlags usage, VkMemoryPropertyFlags memoryPropertyFlags)
{
    this->device = _device.get();
    this->physicalDevice = _device.getPhysicalDevice();
    this->commandPool = commandPool;

    bool imageFound = imageDetails.filePath != "";
    if (imageFound) {
        load(imageDetails.filePath);
    } else {
        VkDeviceSize bufferSize = imageDetails.width * imageDetails.height * imageDetails.channels;
        stbi_uc* emptyBuffer = (stbi_uc*)calloc(1, bufferSize);
        stagingBuffer.create(device, physicalDevice, emptyBuffer, bufferSize);
    }

    VkImageCreateInfo imageInfo {};
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.extent.width = static_cast<uint32_t>(imageDetails.width);
    imageInfo.extent.height = static_cast<uint32_t>(imageDetails.height);
    imageInfo.extent.depth = 1;
    imageInfo.mipLevels = 1;
    imageInfo.arrayLayers = 1;
    imageInfo.format = imageDetails.format;
    imageInfo.tiling = imageDetails.tiling;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageInfo.usage = usage;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;

    if (vkCreateImage(device, &imageInfo, nullptr, &textureImage) != VK_SUCCESS) {
        throw runtime_error("Failed to create image.");
    }

    VkMemoryRequirements memoryRequirements {};
    vkGetImageMemoryRequirements(device, textureImage, &memoryRequirements);

    VkPhysicalDeviceMemoryProperties memoryProperties;
    vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memoryProperties);

    const uint32_t memoryType = [&]() {
        for (uint32_t i = 0; i < memoryProperties.memoryTypeCount; i++) {
            if ((memoryRequirements.memoryTypeBits & (1 << i)) && (memoryProperties.memoryTypes[i].propertyFlags & memoryPropertyFlags) == memoryPropertyFlags) {
                return i;
            }
        }
        throw runtime_error("Failed to find suitable memory type for image.");
    }();

    VkMemoryAllocateInfo allocInfo {};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memoryRequirements.size;
    allocInfo.memoryTypeIndex = memoryType;

    if (vkAllocateMemory(device, &allocInfo, nullptr, &imageMemory) != VK_SUCCESS) {
        throw runtime_error("Failed to allocate image memory.");
    }

    vkBindImageMemory(device, textureImage, imageMemory, 0);

    Queue& graphicsQueue = _device.getGraphicsQueue();
    transitionLayout(graphicsQueue, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        VK_PIPELINE_STAGE_TRANSFER_BIT);

    copyBufferToImage(graphicsQueue);

    transitionLayout(graphicsQueue, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        imageDetails.layout, imageDetails.stageUsage);

    stagingBuffer.destroy();

    createImageView(imageDetails.viewType, imageDetails.format);
}

void Image::load(const char* filePath)
{
    int width, height, channels;
    stbi_uc* pixels = stbi_load(filePath, &width, &height, &channels, STBI_rgb_alpha);
    VkDeviceSize imageSize = imageDetails.width * imageDetails.height * imageDetails.channels;

    if (!pixels) {
        throw runtime_error("Failed to load texture image.");
    }

    stagingBuffer.create(device, physicalDevice, pixels, imageSize);
    stbi_image_free(pixels);
}

void Image::transitionLayout(Queue& queue, VkImageLayout oldLayout,
    VkImageLayout newLayout,
    VkPipelineStageFlags destinationStage)
{
    VkPipelineStageFlags sourceStage;
    VkCommandBuffer cmd = CommandBuffer::beginSingleTimeCommands(device, commandPool);

    VkImageMemoryBarrier barrier {};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = oldLayout;
    barrier.newLayout = newLayout;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = textureImage;
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = 1;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;

    if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
    } else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    } else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_GENERAL) {
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT;

        sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    } else {
        throw invalid_argument("Unsupported layout transition.");
    }

    vkCmdPipelineBarrier(cmd, sourceStage, destinationStage, 0, 0, nullptr, 0, nullptr, 1, &barrier);

    CommandBuffer::endSingleTimeCommands(device, commandPool, cmd, queue);
}

void Image::copyBufferToImage(Queue& queue)
{
    VkCommandBuffer cmd = CommandBuffer::beginSingleTimeCommands(device, commandPool);

    VkBufferImageCopy region {};
    region.bufferOffset = 0;
    region.bufferRowLength = 0;
    region.bufferImageHeight = 0;

    region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.mipLevel = 0;
    region.imageSubresource.baseArrayLayer = 0;
    region.imageSubresource.layerCount = 1;

    region.imageOffset = { 0, 0, 0 };
    region.imageExtent = { imageDetails.width, imageDetails.height, 1 };

    const VkBuffer buffer = stagingBuffer.get();
    vkCmdCopyBufferToImage(cmd, buffer, textureImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

    CommandBuffer::endSingleTimeCommands(device, commandPool, cmd, queue);
}

void Image::createImageView(VkImageViewType viewType, VkFormat format)
{
    VkImageViewCreateInfo imageViewInfo {};
    imageViewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    imageViewInfo.image = textureImage;
    imageViewInfo.viewType = viewType;
    imageViewInfo.format = format;
    imageViewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    imageViewInfo.subresourceRange.baseMipLevel = 0;
    imageViewInfo.subresourceRange.levelCount = 1;
    imageViewInfo.subresourceRange.baseArrayLayer = 0;
    imageViewInfo.subresourceRange.layerCount = 1;

    if (vkCreateImageView(device, &imageViewInfo, nullptr, &imageView) != VK_SUCCESS) {
        throw runtime_error("Failed to create texture image view.");
    }
}

void Image::destroy()
{
    vkDestroyImageView(device, imageView, nullptr);
    vkDestroyImage(device, textureImage, nullptr);
    vkFreeMemory(device, imageMemory, nullptr);
}

VkImage& Image::get()
{
    return textureImage;
}

VkImageView& Image::getView()
{
    return imageView;
}

StagingBuffer& Image::getBuffer()
{
    return stagingBuffer;
}

Image::Details& Image::getDetails()
{
    return imageDetails;
}

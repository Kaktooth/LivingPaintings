#include "image.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

std::map<uint16_t, uint16_t> Image::bindingIdToImageArrayElementId = {};
uint32_t Image::Details::imageId = 1000;

void Image::Details::createImageInfo(

    const char* filePath,
    uint16_t width, uint16_t height, uint8_t channels,
    VkImageLayout imageLayout, VkImageViewType viewType, VkFormat format,
    int stageUsage, VkImageTiling tiling, int aspectFlags,
    VkSampleCountFlagBits samples, stbi_uc* pixels, uint32_t bindingId)
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
    this->aspectFlags = aspectFlags;
    this->samples = samples;
    this->pixels = pixels;
    this->bufferSize = width * height * channels;
    this->bindingId = bindingId;
    if (bindingIdToImageArrayElementId.contains(bindingId)) {
        bindingIdToImageArrayElementId.insert({ bindingId, bindingIdToImageArrayElementId[bindingId]++});
    }
    else {
        bindingIdToImageArrayElementId.insert({ bindingId, 0 });
    }
}

void Image::create(VkDevice& device, VkPhysicalDevice& physicalDevice, 
                   VkCommandPool& commandPool, VkBufferUsageFlags usage,
                   VkMemoryPropertyFlags memoryPropertyFlags, Queue& queue)
{
    this->device = device;
    this->physicalDevice = physicalDevice;
    this->commandPool = commandPool;
    this->usageFlags = usage;

    bool imageFound = strcmp(imageDetails.filePath, "") != 0;
    if (imageFound) {
        load();
    } else {
        bool needAlloc = imageDetails.pixels == nullptr;
        if (needAlloc) {
            imageDetails.pixels = (stbi_uc*)calloc(1, imageDetails.bufferSize);
        }

        stagingBuffer.create(device, physicalDevice, imageDetails.pixels, imageDetails.bufferSize);

        if (needAlloc) {
            free(imageDetails.pixels);
        }
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
    imageInfo.samples = imageDetails.samples;

    if (vkCreateImage(device, &imageInfo, nullptr, &textureImage) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create image.");
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
        throw std::runtime_error("Failed to find suitable memory type for image.");
    }();

    VkMemoryAllocateInfo allocInfo {};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memoryRequirements.size;
    allocInfo.memoryTypeIndex = memoryType;

    if (vkAllocateMemory(device, &allocInfo, nullptr, &imageMemory) != VK_SUCCESS) {
        throw std::runtime_error("Failed to allocate image memory.");
    }

    vkBindImageMemory(device, textureImage, imageMemory, 0);
    if ((usage & VK_IMAGE_USAGE_TRANSFER_DST_BIT) == VK_IMAGE_USAGE_TRANSFER_DST_BIT) {
        transitionLayout(queue, VK_IMAGE_LAYOUT_UNDEFINED,
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            VK_PIPELINE_STAGE_TRANSFER_BIT);

        copyBufferToImage(queue, stagingBuffer.get(),
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

        transitionLayout(queue, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            imageDetails.layout, imageDetails.stageUsage);
    } else if (usage == VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT) {
        transitionLayout(queue, VK_IMAGE_LAYOUT_UNDEFINED,
            VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
            VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT);
    }

    stagingBuffer.destroy();

    createImageView();
}

void Image::load()
{
    int width, height, channels;
    stbi_uc* pixels = stbi_load(imageDetails.filePath, &width, &height, &channels, STBI_rgb_alpha);
    imageDetails.width = width;
    imageDetails.height = height;
    imageDetails.bufferSize = width * height * imageDetails.channels;

    if (!pixels) {
        throw std::runtime_error("Failed to load texture image.");
    }

    stagingBuffer.create(device, physicalDevice, pixels, imageDetails.bufferSize);
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
    barrier.subresourceRange.aspectMask = imageDetails.aspectFlags;
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
    } else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

        sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
    } else {
        throw std::invalid_argument("Unsupported layout transition.");
    }

    vkCmdPipelineBarrier(cmd, sourceStage, destinationStage, 0, 0, nullptr, 0, nullptr, 1, &barrier);

    CommandBuffer::endSingleTimeCommands(device, commandPool, cmd, queue);
}

void Image::copyBufferToImage(Queue& queue, VkBuffer& buffer, VkImageLayout dstLayout)
{

    VkCommandBuffer cmd = CommandBuffer::beginSingleTimeCommands(device, commandPool);

    VkBufferImageCopy region {};
    region.bufferOffset = 0;
    region.bufferRowLength = 0;
    region.bufferImageHeight = 0;

    region.imageSubresource.aspectMask = imageDetails.aspectFlags;
    region.imageSubresource.mipLevel = 0;
    region.imageSubresource.baseArrayLayer = 0;
    region.imageSubresource.layerCount = 1;

    region.imageOffset = { 0, 0, 0 };
    region.imageExtent = { imageDetails.width, imageDetails.height, 1 };

    vkCmdCopyBufferToImage(cmd, buffer, textureImage, dstLayout, 1, &region);

    CommandBuffer::endSingleTimeCommands(device, commandPool, cmd, queue);
}

void Image::copyBufferToImage(Queue& queue, unsigned char* buffer)
{
    Image tempImage;
    tempImage.imageDetails.createImageInfo("", imageDetails.width, imageDetails.height,
                                                    imageDetails.channels, VK_IMAGE_LAYOUT_GENERAL,
                                                    VK_IMAGE_VIEW_TYPE_2D, imageDetails.format,
                                                    VK_SHADER_STAGE_FRAGMENT_BIT, VK_IMAGE_TILING_OPTIMAL,
                                                    VK_IMAGE_ASPECT_COLOR_BIT, VK_SAMPLE_COUNT_1_BIT,
                                                    buffer);
    tempImage.create(device, physicalDevice, commandPool,
                               VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
                               VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, queue);

    VkCommandBuffer cmd = CommandBuffer::beginSingleTimeCommands(device, commandPool);

    VkImageSubresourceLayers imageSubresource{};
    imageSubresource.aspectMask = imageDetails.aspectFlags;
    imageSubresource.mipLevel = 0;
    imageSubresource.baseArrayLayer = 0;
    imageSubresource.layerCount = 1;

    VkImageCopy imageCopy{};
    imageCopy.srcSubresource = imageSubresource;
    imageCopy.srcOffset = { 0, 0, 0 };
    imageCopy.dstSubresource = imageSubresource;
    imageCopy.dstOffset = { 0, 0, 0 };
    imageCopy.extent = { imageDetails.width, imageDetails.height, 1 };

    vkCmdCopyImage(cmd, tempImage.get(),
                   VK_IMAGE_LAYOUT_GENERAL,
                   textureImage,
                   imageDetails.layout, 1, &imageCopy);

    CommandBuffer::endSingleTimeCommands(device, commandPool, cmd, queue);

    tempImage.destroy();
}

void Image::copyBufferToImage(Queue& queue, unsigned char* buffer, uint32_t bufImageWidth, uint32_t bufImageHeight)
{
    Image tempImage;
    tempImage.imageDetails.createImageInfo("", bufImageWidth, bufImageHeight,
        imageDetails.channels, VK_IMAGE_LAYOUT_GENERAL,
        VK_IMAGE_VIEW_TYPE_2D, imageDetails.format,
        VK_SHADER_STAGE_FRAGMENT_BIT, VK_IMAGE_TILING_OPTIMAL,
        VK_IMAGE_ASPECT_COLOR_BIT, VK_SAMPLE_COUNT_1_BIT,
        buffer);
    tempImage.create(device, physicalDevice, commandPool,
        VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, queue);

    VkCommandBuffer cmd = CommandBuffer::beginSingleTimeCommands(device, commandPool);

    VkImageSubresourceLayers imageSubresource{};
    imageSubresource.aspectMask = imageDetails.aspectFlags;
    imageSubresource.mipLevel = 0;
    imageSubresource.baseArrayLayer = 0;
    imageSubresource.layerCount = 1;

    VkImageCopy imageCopy{};
    imageCopy.srcSubresource = imageSubresource;
    imageCopy.srcOffset = { 0, 0, 0 };
    imageCopy.dstSubresource = imageSubresource;
    imageCopy.dstOffset = { 0, 0, 0 };
    imageCopy.extent = { bufImageWidth, bufImageHeight, 1 };

    vkCmdCopyImage(cmd, tempImage.get(),
        VK_IMAGE_LAYOUT_GENERAL,
        textureImage,
        imageDetails.layout, 1, &imageCopy);

    CommandBuffer::endSingleTimeCommands(device, commandPool, cmd, queue);

    tempImage.destroy();

    imageDetails.width = bufImageWidth;
    imageDetails.height = bufImageHeight;
}

void Image::createImageView()
{
    VkImageViewCreateInfo imageViewInfo {};
    imageViewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    imageViewInfo.image = textureImage;
    imageViewInfo.viewType = imageDetails.viewType;
    imageViewInfo.format = imageDetails.format;
    imageViewInfo.subresourceRange.aspectMask = imageDetails.aspectFlags;
    imageViewInfo.subresourceRange.baseMipLevel = 0;
    imageViewInfo.subresourceRange.levelCount = 1;
    imageViewInfo.subresourceRange.baseArrayLayer = 0;
    imageViewInfo.subresourceRange.layerCount = 1;

    if (vkCreateImageView(device, &imageViewInfo, nullptr, &imageView) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create texture image view.");
    }
}

void Image::destroy()
{
    vkDestroyImageView(device, imageView, nullptr);
    vkDestroyImage(device, textureImage, nullptr);
    vkFreeMemory(device, imageMemory, nullptr);
}

const VkImage& Image::get() const
{
    return textureImage;
}

const VkImageView& Image::getView() const
{
    return imageView;
}

const StagingBuffer Image::getBuffer() const
{
    return stagingBuffer;
}

const Image::Details& Image::getDetails() const
{
    return imageDetails;
}

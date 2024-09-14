#pragma once
#include "buffer.h"
#include <stdexcept>
#include <string>

class Image {

    StagingBuffer stagingBuffer;
    VkDeviceMemory imageMemory = VK_NULL_HANDLE;
    VkImage textureImage = VK_NULL_HANDLE;
    VkImageView imageView = VK_NULL_HANDLE;
    VkDevice device = VK_NULL_HANDLE;
    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
    VkCommandPool commandPool = VK_NULL_HANDLE;

    void load(const char* filePath);
    void transitionLayout(Queue& queue, VkImageLayout oldLayout,
                          VkImageLayout newLayout,
                          VkPipelineStageFlags destinationStage);
    void copyBufferToImage(Queue& queue, VkBuffer& buffer,
                           VkImageLayout dstLayout);

public:
    struct Details {
        const char* filePath;
        uint16_t width;
        uint16_t height;
        int8_t channels;
        VkImageLayout layout;
        VkFormat format;
        int stageUsage; // see VkShaderStageFlagBits, using int to combine bits
        VkImageTiling tiling;
        VkImageViewType viewType;
        int aspectFlags; // see VkImageAspectFlagBits
        VkSampleCountFlagBits samples;
        stbi_uc* pixels;
        VkDeviceSize bufferSize;

        void createImageInfo(
            const char* filePath,
            uint16_t width,
            uint16_t height, uint8_t channels,
            VkImageLayout imageLayout,
            VkImageViewType viewType, VkFormat format,
            int stageUsage, VkImageTiling tiling,
            int aspectFlags,
            VkSampleCountFlagBits samples,
            stbi_uc* pixels = (stbi_uc*)"");
    } imageDetails;

    void create(Device& _device, VkCommandPool& commandPool,
                VkBufferUsageFlags usage,
                VkMemoryPropertyFlags memoryPropertyFlags, Queue& queue);
    void createImageView();
    void destroy();
    VkImage& get();
    VkImageView& getView();
    StagingBuffer& getBuffer();
    Details& getDetails();
};

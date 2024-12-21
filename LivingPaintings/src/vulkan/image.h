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
    VkBufferUsageFlags usageFlags;

    void load(const char* filePath);
    void transitionLayout(Queue& queue, VkImageLayout oldLayout,
                          VkImageLayout newLayout,
                          VkPipelineStageFlags destinationStage);

public:
    static std::map<uint16_t, uint16_t> bindingIdToImageArrayElementId;

    struct Details {
        static uint32_t imageId;
        uint32_t bindingId;
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
            stbi_uc* pixels = (stbi_uc*)"",
            uint32_t bindingId = imageId++);
    } imageDetails;

    void create(VkDevice& device, VkPhysicalDevice& physicalDevice,
                VkCommandPool& commandPool, VkBufferUsageFlags usage,
                VkMemoryPropertyFlags memoryPropertyFlags, Queue& queue);
    void copyBufferToImage(Queue& queue, VkBuffer& buffer,
                           VkImageLayout dstLayout);
    void copyBufferToImage(Queue& queue, unsigned char* buffer);
    void copyBufferToImage(Queue& queue, unsigned char* buffer, uint32_t bufImageWidth, uint32_t bufImageHeight);
    void createImageView();
    void destroy();
    VkImage& get();
    VkImageView& getView();
    StagingBuffer getBuffer();
    Details& getDetails();
};

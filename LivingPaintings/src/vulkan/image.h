// This is a personal academic project. Dear PVS-Studio, please check it.

// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com

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
    void copyBufferToImage(Queue& queue);
    void createImageView(VkImageViewType viewType, VkFormat format);

public:
    const VkClearColorValue clearColor = { { 0.0f, 0.0f, 0.0f, 0.0f } };

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

        void createImageInfo(const char* filePath, uint16_t width,
            uint16_t height, uint8_t channels, VkImageLayout imageLayout,
            VkImageViewType viewType, VkFormat format,
            int stageUsage,
            VkImageTiling tiling);
    } imageDetails;

    void create(Device& _device, VkCommandPool& commandPool,
        VkBufferUsageFlags usage,
        VkMemoryPropertyFlags memoryPropertyFlags);
    void destroy();
    VkImage& get();
    VkImageView& getView();
    StagingBuffer& getBuffer();
    Details& getDetails();
};

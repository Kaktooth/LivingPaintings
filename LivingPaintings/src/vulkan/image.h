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
    void transitionLayout(Queue& queue, VkImageLayout oldLayout, VkImageLayout newLayout);
    void copyBufferToImage(Queue& queue, uint32_t width, uint32_t height);
    void createImageView(VkImageViewType viewType, VkFormat format);

public:
    struct Details {
        const char* filePath;
        int width;
        int height;
        VkFormat format;
        VkImageTiling tiling;
        VkImageViewType viewType;

        void createImageInfo(const char* filePath,
            uint32_t width, uint32_t height,
            VkImageViewType viewType, VkFormat format,
            VkImageTiling tiling);
    } imageDetails;

    void create(Device& _device, VkCommandPool& commandPool,
        VkBufferUsageFlags usage,
        VkMemoryPropertyFlags memoryPropertyFlags);
    void destroy();
    VkImage& get();
    VkImageView& getView();
};

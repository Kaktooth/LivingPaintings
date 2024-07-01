// This is a personal academic project. Dear PVS-Studio, please check it.

// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com

#pragma once
#include "buffer.h"
#include "vulkan/vulkan.h"

class Image {

    StagingBuffer stagingBuffer;
    VkDeviceMemory imageMemory;
    VkImage textureImage;
    VkImageView imageView;

    void load(VkDevice device, VkPhysicalDevice physicalDevice, const char* filePath);
    void transitionLayout(VkDevice device, VkCommandPool commandPool, Queue queue, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);
    void copyBufferToImage(VkDevice device, VkCommandPool commandPool, Queue queue, uint32_t width, uint32_t height);
    void createImageView(VkDevice device);

public:
    void create(VkDevice device, VkPhysicalDevice physicalDevice, VkCommandPool commandPool, Queue graphicsQueue, const char* filePath, int width, int height, VkFormat format, VkImageTiling tiling, VkBufferUsageFlags usage, VkMemoryPropertyFlags memoryPropertyFlags);
    void destroy(VkDevice device);
    VkImage get();
    VkImageView getView();
};

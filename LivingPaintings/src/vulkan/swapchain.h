// This is a personal academic project. Dear PVS-Studio, please check it.

// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com

#pragma once
#include "device.h"
#include "image.h"
#include "semaphore.h"
#include "surface.h"
#include "vulkan/vulkan.h"
#include <array>
#include <stdexcept>
#include <vector>

class Swapchain {

    VkSwapchainKHR swapchain = VK_NULL_HANDLE;
    VkDevice device = VK_NULL_HANDLE;
    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
    VkCommandPool commandPool = VK_NULL_HANDLE;
    Surface surface;
    QueueFamily::Indices queueFamilyIndicies;
    uint32_t minImageCount;
    Image depthImage;
    Image colorImage;
    std::vector<VkImage> images;
    std::vector<VkImageView> imageViews;
    std::vector<VkFramebuffer> framebuffers;
    VkColorSpaceKHR colorSpace;
    VkFormat imageFormat;
    VkFormat depthFormat;
    VkSampleCountFlagBits samples;
    int aspectFlags;
    VkExtent2D extent;
    uint32_t currentFrame;
    bool framebufferResized;

    void createSpecializedImages(Device& device);

public:
    void setContext(Device& device, Surface& surface,
        VkCommandPool commandPool, VkSampleCountFlagBits samples,
        std::vector<VkFormat> depthFormatCandidates);
    void create();
    VkImageView& createImageView(VkImage& image, VkFormat& format,
        VkImageAspectFlags aspectFlags);
    void createSpecializedImageViews();
    void createImageViews();
    void createFramebuffers(VkRenderPass& renderPass);
    void presentImage(VkRenderPass& renderPass, VkQueue& presentationQueue, const std::vector<VkSemaphore> signalSemafores, GLFWwindow* window);
    uint32_t asquireNextImage(VkRenderPass& renderPass,
        VkSemaphore& imageAvailable, GLFWwindow* window);
    void nextFrame();
    void recreate(VkRenderPass& renderPass, GLFWwindow* window);
    void destroy(bool destroyImages = false);
    uint32_t getMinImageCount();
    VkFormat& getImageFormat();
    VkFormat& getDepthFormat();
    VkExtent2D& getExtent();
    std::vector<VkImageView>& getImageViews();
    std::vector<VkFramebuffer>& getFramebuffers();
    uint32_t& getCurrentFrame();
    void resizeFramebuffer();
};

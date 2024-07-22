// This is a personal academic project. Dear PVS-Studio, please check it.

// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com

#pragma once
#include "device.h"
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
    Surface surface;
    QueueFamily::Indices queueFamilyIndicies;
    uint32_t minImageCount;
    std::vector<VkImage> images;
    std::vector<VkImageView> imageViews;
    std::vector<VkImageView> specializedImageViews; // currently 1. resolve 2. depth
    std::vector<VkFramebuffer> framebuffers;
    VkFormat imageFormat;
    VkExtent2D extent;
    uint32_t currentFrame;
    bool framebufferResized;

public:
    void setDeviceContext(Device& device, Surface& surface);
    void create();
    VkImageView& createImageView(VkImage& image, VkFormat& format,
        VkImageAspectFlags aspectFlags);
    void createSpecializedImageView(VkImage image, VkFormat format,
        VkImageAspectFlags aspectFlags);
    void createImageViews();
    void createFramebuffers(VkRenderPass& renderPass);
    void presentImage(VkRenderPass& renderPass, VkQueue& presentationQueue, const std::vector<VkSemaphore> signalSemafores, GLFWwindow* window);
    uint32_t asquireNextImage(VkRenderPass& renderPass,
        VkSemaphore& imageAvailable, GLFWwindow* window);
    void nextFrame();
    void recreate(VkRenderPass& renderPass, GLFWwindow* window);
    void destroy();
    uint32_t getMinImageCount();
    VkFormat& getImageFormat();
    VkExtent2D& getExtent();
    std::vector<VkImageView>& getImageViews();
    std::vector<VkFramebuffer>& getFramebuffers();
    uint32_t& getCurrentFrame();
    void resizeFramebuffer();
};

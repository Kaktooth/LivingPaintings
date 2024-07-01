// This is a personal academic project. Dear PVS-Studio, please check it.

// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com

#pragma once
#include "queue_family.h"
#include "semaphore.h"
#include "surface.h"
#include "vulkan/vulkan.h"
#include <vector>

class SwapChain {

    VkSwapchainKHR swapChain;
    uint32_t minImageCount;
    std::vector<VkImage> images;
    std::vector<VkImageView> imageViews;
    std::vector<VkFramebuffer> framebuffers;
    VkSurfaceFormatKHR surfaceFormat;
    VkPresentModeKHR presentationMode;
    VkFormat imageFormat;
    VkExtent2D extent;
    uint32_t currentFrame;
    bool framebufferResized;

    struct Details {
        VkSurfaceCapabilitiesKHR capabilities;
        std::vector<VkSurfaceFormatKHR> formats;
        std::vector<VkPresentModeKHR> presentationModes;
    };

    VkSurfaceFormatKHR chooseSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
    VkPresentModeKHR choosePresentationMode(const std::vector<VkPresentModeKHR>& availablePresentationModes);

public:
    void create(VkDevice device, VkPhysicalDevice physicalDevice, Surface usedSurface, QueueFamily::Indices indicies);
    void createImageViews(VkDevice device);
    void createFramebuffers(VkDevice device, VkRenderPass renderPass);
    void presentImage(VkDevice device, VkPhysicalDevice physicalDevice, VkRenderPass renderPass, Surface surface, QueueFamily::Indices indicies, VkQueue& presentationQueue, std::vector<VkSemaphore> signalSemafores, GLFWwindow* window);
    uint32_t asquireNextImage(VkDevice device, VkPhysicalDevice physicalDevice, VkRenderPass renderPass, Surface surface, QueueFamily::Indices indicies, Semaphore imageAvailable, GLFWwindow* window);
    static SwapChain::Details getDetails(VkPhysicalDevice& device, VkSurfaceKHR& surface);
    void nextFrame();
    void recreate(VkDevice device, VkPhysicalDevice physicalDevice, VkRenderPass renderPass, Surface surface, QueueFamily::Indices indicies, GLFWwindow* window);
    void destroy(VkDevice device);
    uint32_t getMinImageCount();
    VkFormat& getImageFormat();
    VkExtent2D& getExtent();
    std::vector<VkImageView>& getImageViews();
    std::vector<VkFramebuffer>& getFramebuffers();
    uint32_t& getCurrentFrame();
    void resizeFramebuffer();
};

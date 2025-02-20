#pragma once
#include "device.h"
#include "image.h"
#include "semaphore.h"
#include "surface.h"
#include "vulkan/vulkan.h"
#include <algorithm>
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

    void createSpecializedImages(Queue& graphicsQueue);

public:
    void setContext(Device& device, Surface& surface,
        VkCommandPool& commandPool, VkSampleCountFlagBits& samples,
        std::vector<VkFormat> depthFormatCandidates);
    void create();
    VkImageView& createImageView(VkImage& image, VkFormat& format,
        VkImageAspectFlags aspectFlags);
    void createImageViews();
    void createFramebuffers(VkRenderPass& renderPass);
    void presentImage(Queue& graphicsQueue, VkRenderPass& renderPass,
        VkQueue& presentationQueue,
        std::vector<VkSemaphore> signalSemafores,
        GLFWwindow* window);
    uint32_t asquireNextImage(Queue& graphicsQueue, VkRenderPass& renderPass,
        VkSemaphore& imageAvailable, GLFWwindow* window);
    void nextFrame();
    void recreate(Queue& graphicsQueue, VkRenderPass& renderPass, GLFWwindow* window);
    void destroy();
    
    std::shared_ptr<unsigned char> writeFrameToBuffer(VkCommandBuffer cmd, Queue transferQueue, uint8_t currentFrame);
    
    uint32_t getMinImageCount();
    VkFormat& getImageFormat();
    VkFormat& getDepthFormat();
    VkExtent2D& getExtent();
    std::vector<VkImageView>& getImageViews();
    std::vector<VkFramebuffer>& getFramebuffers();
    uint32_t& getCurrentFrame();
    void resizeFramebuffer();
    VkDevice& getDevice();
    VkPhysicalDevice& getPhysicalDevice();
    VkCommandPool& getCommandPool();
    Image& getColorImage();
    Image& getDepthImage();
};

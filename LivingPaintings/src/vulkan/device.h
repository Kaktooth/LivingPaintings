#pragma once
#include "consts.h"
#include "queue.h"
#include "queue_family.h"
#include "vulkan/vulkan.h"
#include <algorithm>
#include <array>
#include <cstdint>
#include <limits>
#include <map>
#include <set>
#include <stdexcept>
#include <vector>

class Device {

    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
    VkDevice device = VK_NULL_HANDLE;
    VkSurfaceKHR surface = VK_NULL_HANDLE;
    Queue graphicsQueue;
    Queue presentationQueue;
    Queue transferQueue;
    Queue computeQueue;
    QueueFamily queueFamily;
    VkPhysicalDeviceProperties deviceProperties;
    VkPhysicalDeviceFeatures deviceFeatures;

    static VkDeviceQueueCreateInfo createQueueCreateInfo(uint32_t queueFamily, float queuePriority);
    int getDeviceScore(VkPhysicalDevice& physicalDevice, Surface& surface);
    static bool checkDeviceExtensionSupport(VkPhysicalDevice& physicalDevice);

public:
    VkDevice& create(VkInstance& instance, Surface& surface);
    void selectPhysicalDevice(VkInstance& instance, Surface& surface);
    VkFormat findSupportedFormat(std::vector<VkFormat>& formats,
        VkImageTiling tiling,
        VkFormatFeatureFlags features);
    static bool hasStencilComponent(VkFormat format);
    VkSampleCountFlagBits getMaxSampleCount() const;
    void destroy();
    VkDevice& get();
    VkPhysicalDevice& getPhysicalDevice();
    VkSurfaceKHR getSurface();
    Queue& getGraphicsQueue();
    Queue& getPresentationQueue();
    Queue& getTransferQueue();
    Queue& getComputeQueue();
    QueueFamily& getQueueFamily();
    VkPhysicalDeviceProperties getProperties();
    VkPhysicalDeviceFeatures getFeatures();
};

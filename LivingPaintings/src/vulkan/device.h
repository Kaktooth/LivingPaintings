// This is a personal academic project. Dear PVS-Studio, please check it.

// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com

#pragma once
#include "consts.h"
#include "queue.h"
#include "queue_family.h"
#include "vulkan/vulkan.h"
#include <algorithm>
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
    QueueFamily queueFamily;

    VkDeviceQueueCreateInfo createQueueCreateInfo(const uint32_t queueFamily, const float queuePriority);
    int getDeviceScore(VkPhysicalDevice& physicalDevice, Surface& surface);
    bool checkDeviceExtensionSupport(VkPhysicalDevice& physicalDevice);
    VkPhysicalDeviceFeatures selectedDeviceFeatures();

public:
    VkDevice& create(VkInstance& instance, Surface& surface);
    void selectPhysicalDevice(VkInstance& instance, Surface& surface);
    void destroy();
    VkDevice& get();
    VkPhysicalDevice& getPhysicalDevice();
    VkSurfaceKHR getSurface();
    Queue& getGraphicsQueue();
    Queue& getPresentationQueue();
    Queue& getTransferQueue();
    QueueFamily& getQueueFamily();
};

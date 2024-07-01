// This is a personal academic project. Dear PVS-Studio, please check it.

// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com

#pragma once
#include "consts.h"
#include "queue_family.h"
#include "surface.h"
#include "swapchain.h"
#include "vulkan/vulkan.h"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <set>

class PhysicalDevice {

    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
    Surface surface;
    QueueFamily queueFamily;

    int getDeviceScore(VkPhysicalDevice device);
    bool checkDeviceExtensionSupport(VkPhysicalDevice device);

public:
    VkPhysicalDeviceFeatures selectedDeviceFeatures();
    void select(VkInstance& instance, Surface& surface, QueueFamily& queueFamily);
    VkPhysicalDevice get();
    Surface usedSurface();
};

// This is a personal academic project. Dear PVS-Studio, please check it.

// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com

#pragma once
#include "consts.h"
#include "physical_device.h"
#include "queue.h"
#include "vulkan/vulkan.h"

class Device {

    VkPhysicalDevice physicalDevice;
    VkDevice device;
    Queue graphicsQueue;
    Queue presentationQueue;
    Queue transferQueue;

    VkDeviceQueueCreateInfo createQueueCreateInfo(uint32_t queueFamily, float queuePriority);

public:
    void create(VkPhysicalDevice& physicalDevice, QueueFamily::Indices& queueFamilyIndicies,
        VkPhysicalDeviceFeatures const& deviceFeatures);
    void destroy();
    VkDevice& get();
    VkPhysicalDevice& getPhysicalDevice();
    Queue getGraphicsQueue();
    Queue getPresentationQueue();
    Queue getTransferQueue();
};

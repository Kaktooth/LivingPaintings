// This is a personal academic project. Dear PVS-Studio, please check it.

// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com

#include "device.h"
#include <set>
#include <stdexcept>
#include <vector>

using namespace std;

void Device::create(VkPhysicalDevice& physicalDevice, QueueFamily::Indices& queueFamilyIndicies,
    VkPhysicalDeviceFeatures const& deviceFeatures)
{
    Device::physicalDevice = physicalDevice;

    vector<VkDeviceQueueCreateInfo> queueCreateInfos;
    float queuePriority = 1.0f;

    set<uint32_t> queueFamilies = { queueFamilyIndicies.graphicsFamily.value(),
        queueFamilyIndicies.presentationFamily.value(), queueFamilyIndicies.transferFamily.value() };

    for (auto queueFamily : queueFamilies) {
        auto queueCreateInfo = createQueueCreateInfo(queueFamily, queuePriority);
        queueCreateInfos.push_back(queueCreateInfo);
    }

    VkDeviceCreateInfo deviceInfo {};
    deviceInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    deviceInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
    deviceInfo.pQueueCreateInfos = queueCreateInfos.data();
    deviceInfo.pEnabledFeatures = &deviceFeatures;
    deviceInfo.enabledExtensionCount = static_cast<uint32_t>(Constants::DEVICE_EXTENTIONS.size());
    deviceInfo.ppEnabledExtensionNames = Constants::DEVICE_EXTENTIONS.data();

    if (Constants::ENABLE_VALIDATION_LAYERS) {
        deviceInfo.enabledExtensionCount = static_cast<uint32_t>(Constants::VALIDATION_LAYERS.size());
        deviceInfo.ppEnabledLayerNames = Constants::VALIDATION_LAYERS.data();
    } else {
        deviceInfo.enabledLayerCount = 0;
    }

    if (vkCreateDevice(physicalDevice, &deviceInfo, nullptr, &device) != VK_SUCCESS) {
        throw runtime_error("Failed to create logical device.");
    }

    vkGetDeviceQueue(device, queueFamilyIndicies.graphicsFamily.value(), 0, &graphicsQueue.get());
    vkGetDeviceQueue(device, queueFamilyIndicies.presentationFamily.value(), 0, &presentationQueue.get());
    vkGetDeviceQueue(device, queueFamilyIndicies.transferFamily.value(), 0, &transferQueue.get());
}

VkDeviceQueueCreateInfo Device::createQueueCreateInfo(uint32_t queueFamily, float queuePriority)
{
    VkDeviceQueueCreateInfo queueInfo {};
    queueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queueInfo.queueFamilyIndex = queueFamily;
    queueInfo.queueCount = 1;
    queueInfo.pQueuePriorities = &queuePriority;

    return queueInfo;
}

void Device::destroy()
{
    vkDestroyDevice(device, nullptr);
}

VkDevice& Device::get()
{
    return device;
}

VkPhysicalDevice& Device::getPhysicalDevice()
{
    return physicalDevice;
}

Queue Device::getGraphicsQueue()
{
    return graphicsQueue;
}

Queue Device::getPresentationQueue()
{
    return presentationQueue;
}

Queue Device::getTransferQueue()
{
    return transferQueue;
}

// This is a personal academic project. Dear PVS-Studio, please check it.

// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com

#include "queue_family.h"
#include <stdexcept>
#include <vector>

using namespace std;

bool QueueFamily::Indices::isAvailable()
{
    return QueueFamily::Indices::graphicsFamily.has_value() && QueueFamily::Indices::presentationFamily.has_value()
        && QueueFamily::Indices::transferFamily.has_value();
}

QueueFamily::Indices
QueueFamily::findQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR surface)
{
    uint32_t queueFamilyCount = 0;
    QueueFamily::Indices indicies;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);
    vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(
        device, &queueFamilyCount, queueFamilies.data());

    int queueIndex = 0;
    VkBool32 presentationSupport = false;

    for (const auto& queueFamily : queueFamilies) {
        if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            indicies.graphicsFamily = queueIndex;
        }

        if (queueFamily.queueFlags & VK_QUEUE_TRANSFER_BIT) {
            indicies.transferFamily = queueIndex;
        }

        vkGetPhysicalDeviceSurfaceSupportKHR(
            device, queueIndex, surface, &presentationSupport);
        if (presentationSupport) {
            indicies.presentationFamily = queueIndex;
        }

        if (indicies.isAvailable()) {
            break;
        }
        queueIndex++;
    }

    return indicies;
}

#include "queue_family.h"

bool QueueFamily::Indices::isAvailable()
{
    return QueueFamily::Indices::graphicsFamily.has_value() && QueueFamily::Indices::presentationFamily.has_value()
        && QueueFamily::Indices::transferFamily.has_value() && QueueFamily::Indices::computeFamily.has_value();
}

QueueFamily::Indices QueueFamily::findQueueFamilies(VkPhysicalDevice& device, VkSurfaceKHR& surface)
{
    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);
    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

    VkBool32 presentationSupport;
    for (size_t queueIndex = 0; queueIndex < queueFamilies.size(); queueIndex++) {
        if (!indicies.graphicsFamily.has_value() && queueFamilies[queueIndex].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            indicies.graphicsFamily = queueIndex;
        }

        if (!indicies.transferFamily.has_value()
            && queueFamilies[queueIndex].queueFlags & VK_QUEUE_TRANSFER_BIT) {
            indicies.transferFamily = queueIndex;
        }

        if (!indicies.computeFamily.has_value() && queueFamilies[queueIndex].queueFlags & VK_QUEUE_COMPUTE_BIT) {
            indicies.computeFamily = queueIndex;
        }

        vkGetPhysicalDeviceSurfaceSupportKHR(device, queueIndex, surface, &presentationSupport);
        if (!indicies.presentationFamily.has_value() && presentationSupport) {
            indicies.presentationFamily = queueIndex;
        }

        if (indicies.isAvailable()) {
            break;
        }
    }

    if (!indicies.isAvailable()) {
        throw std::runtime_error("Queue family was not found!");
    }

    return indicies;
}

// This is a personal academic project. Dear PVS-Studio, please check it.

// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com

#include "surface.h"

using namespace std;

VkSurfaceKHR& Surface::create(VkInstance& instance, GLFWwindow* pWindow)
{
    this->instance = instance;
    this->pWindow = pWindow;
    if (glfwCreateWindowSurface(instance, pWindow, nullptr, &surface) != VK_SUCCESS) {
        throw runtime_error("Failed to create window surface.");
    }

    return surface;
}

VkExtent2D Surface::chooseResolution()
{
    constexpr unsigned int maxResolution = numeric_limits<uint32_t>::max();
    if (details.capabilities.currentExtent.width != maxResolution && details.capabilities.currentExtent.height != maxResolution) {
        return details.capabilities.currentExtent;
    } else {
        int width, height;
        glfwGetFramebufferSize(pWindow, &width, &height);

        VkExtent2D extent = {
            static_cast<uint32_t>(width),
            static_cast<uint32_t>(height)
        };

        extent.width = clamp(extent.width, details.capabilities.minImageExtent.width,
            details.capabilities.maxImageExtent.width);
        extent.height = clamp(extent.height, details.capabilities.minImageExtent.height,
            details.capabilities.maxImageExtent.height);
        return extent;
    }
}

Surface::Details Surface::findSurfaceDetails(VkPhysicalDevice& device)
{

    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities);

    uint32_t formatsCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatsCount,
        nullptr);

    if (formatsCount != 0) {
        details.formats.resize(formatsCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatsCount,
            details.formats.data());
    }

    uint32_t presentationModsCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface,
        &presentationModsCount, nullptr);

    if (formatsCount != 0) {
        details.presentationModes.resize(presentationModsCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(
            device, surface, &presentationModsCount,
            details.presentationModes.data());
    }

    return details;
}

VkSurfaceFormatKHR Surface::chooseSurfaceFormat()
{
    for (const VkSurfaceFormatKHR& availableFormat : details.formats) {
        if (availableFormat.format == Constants::IMAGE_FORMAT && availableFormat.colorSpace == Constants::COLOR_SPACE) {
            return availableFormat;
        }
    }

    return details.formats[0];
}

VkPresentModeKHR Surface::choosePresentationMode()
{
    for (VkPresentModeKHR presentationMode : details.presentationModes) {
        if (presentationMode == VK_PRESENT_MODE_MAILBOX_KHR) {
            return VK_PRESENT_MODE_MAILBOX_KHR;
        }
    }
    return VK_PRESENT_MODE_IMMEDIATE_KHR;
}

void Surface::destory()
{
    vkDestroySurfaceKHR(instance, surface, nullptr);
}

VkSurfaceKHR& Surface::get()
{
    return surface;
}

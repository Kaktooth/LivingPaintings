// This is a personal academic project. Dear PVS-Studio, please check it.

// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com

#include "surface.h"
#include <algorithm>
#include <stdexcept>

using namespace std;

void Surface::create(VkInstance& instance, GLFWwindow* window)
{
    glfwWindow = window;
    if (glfwCreateWindowSurface(instance, window, nullptr, &surface) != VK_SUCCESS) {
        throw runtime_error("Failed to create window surface.");
    }
}

VkExtent2D
Surface::chooseResolution(const VkSurfaceCapabilitiesKHR& capabilities)
{
    constexpr auto maxResolution = numeric_limits<uint32_t>::max();
    if (capabilities.currentExtent.width != maxResolution && capabilities.currentExtent.height != maxResolution) {
        return capabilities.currentExtent;
    } else {
        int width, height;
        glfwGetFramebufferSize(glfwWindow, &width, &height);

        VkExtent2D extent = {
            static_cast<uint32_t>(width),
            static_cast<uint32_t>(height)
        };

        extent.width = clamp(extent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
        extent.height = clamp(extent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);
        return extent;
    }
}

void Surface::destory(VkInstance& instance)
{
    vkDestroySurfaceKHR(instance, surface, nullptr);
}

VkSurfaceKHR& Surface::get()
{
    return surface;
}

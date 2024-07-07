// This is a personal academic project. Dear PVS-Studio, please check it.

// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com

#pragma once
#define GLFW_INCLUDE_VULKAN
#include "vulkan/vulkan.h"
#include <GLFW/glfw3.h>
#include <vector>

class Surface {

    VkInstance instance;
    VkSurfaceKHR surface;
    GLFWwindow* pWindow;

public:
    struct Details {
        VkSurfaceCapabilitiesKHR capabilities;
        std::vector<VkSurfaceFormatKHR> formats;
        std::vector<VkPresentModeKHR> presentationModes;
    } details;

    VkSurfaceKHR& create(VkInstance& instance, GLFWwindow* pWindow);
    void destory();
    VkExtent2D chooseResolution();
    Surface::Details findSurfaceDetails(VkPhysicalDevice& device);
    VkSurfaceFormatKHR chooseSurfaceFormat();
    VkPresentModeKHR choosePresentationMode();
    VkSurfaceKHR& get();
};

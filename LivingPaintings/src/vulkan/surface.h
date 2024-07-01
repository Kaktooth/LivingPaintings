// This is a personal academic project. Dear PVS-Studio, please check it.

// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com

#pragma once
#define GLFW_INCLUDE_VULKAN
#include "vulkan/vulkan.h"
#include <GLFW/glfw3.h>

class Surface {

    VkSurfaceKHR surface;
    GLFWwindow* glfwWindow;

public:
    void create(VkInstance& instance, GLFWwindow* window);
    void destory(VkInstance& instance);
    VkExtent2D chooseResolution(const VkSurfaceCapabilitiesKHR& capabilities);
    VkSurfaceKHR& get();
};

#pragma once
#define GLFW_INCLUDE_VULKAN
#include "consts.h"
#include "glm/common.hpp"
#include "vulkan/vulkan.h"
#include <GLFW/glfw3.h>
#include <algorithm>
#include <stdexcept>
#include <vector>

class Surface {

    VkInstance instance;
    VkSurfaceKHR surface;
    std::shared_ptr<GLFWwindow> pWindow;

public:
    struct Details {
        VkSurfaceCapabilitiesKHR capabilities;
        std::vector<VkSurfaceFormatKHR> formats;
        std::vector<VkPresentModeKHR> presentationModes;
    } details;

    VkSurfaceKHR& create(VkInstance& instance, GLFWwindow* pWindow);
    void destory() const;
    VkExtent2D chooseResolution();
    Surface::Details findSurfaceDetails(VkPhysicalDevice& device);
    VkSurfaceFormatKHR chooseSurfaceFormat();
    VkPresentModeKHR choosePresentationMode();
    VkSurfaceKHR& get();
};

#pragma once
#include "consts.h"
#include "debug.h"
#include "vulkan/vulkan.h"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <iostream>
#include <stdexcept>
#include <vector>
#include <cstring>

class VulkanInstance {

    VkInstance instance = VK_NULL_HANDLE;

public:
    VkInstance& create(VkDebugUtilsMessengerCreateInfoEXT& debugCreateInfo);
    static void enumerateExtentions();
    std::vector<const char*> findRequiredExtensions() const;
    [[nodiscard]]
    bool checkValidationLayerSupport();
    void destroy();
    VkInstance& get();
};

// This is a personal academic project. Dear PVS-Studio, please check it.

// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com

#pragma once
#include "consts.h"
#include "debug.h"
#include "vulkan/vulkan.h"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <iostream>
#include <stdexcept>
#include <vector>

class VulkanInstance {

    VkInstance instance = VK_NULL_HANDLE;

public:
    VkInstance& create(VkDebugUtilsMessengerCreateInfoEXT& debugCreateInfo);
    void enumerateExtentions();
    std::vector<const char*> findRequiredExtensions() const;
    bool checkValidationLayerSupport() const;
    void destroy();
    VkInstance& get();
};

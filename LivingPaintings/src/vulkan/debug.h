// This is a personal academic project. Dear PVS-Studio, please check it.

// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com

#pragma once
#include "consts.h"
#include "instance.h"
#include "vulkan/vulkan.h"

#include <iostream>
#include <stdexcept>

class VulkanDebugMessenger {

    VkDebugUtilsMessengerEXT debugMessenger;

public:
    VulkanDebugMessenger() = default;

    void setup(VkInstance& instance, VkDebugUtilsMessengerCreateInfoEXT& debugCreateInfo);
    VkDebugUtilsMessengerCreateInfoEXT makeDebugMessengerCreateInfo();
    VkResult createDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
        const VkAllocationCallbacks* pAllocator,
        VkDebugUtilsMessengerEXT* pDebugMessenger);
    void destroyIfLayersEnabled(VkInstance& instance);
    void destroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger,
        const VkAllocationCallbacks* pAllocator);
    static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
        VkDebugUtilsMessageTypeFlagsEXT messageType,
        const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
        void* pUserData)
    {
        if (messageSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) {
            std::cerr << "validation layer: [" << messageType << "] " << pCallbackData->pMessage << std::endl;
        }

        return VK_FALSE;
    }
};

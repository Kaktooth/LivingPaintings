// This is a personal academic project. Dear PVS-Studio, please check it.

// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com

#pragma once
#include "consts.h"
#include "instance.h"
#include "vulkan/vulkan.h"

#include <iostream>
#include <stdexcept>

class VulkanDebugMessenger {

    VkDebugUtilsMessengerEXT debugMessenger = VK_NULL_HANDLE;
    VkInstance instance = VK_NULL_HANDLE;

public:
    VkDebugUtilsMessengerEXT setup(VkInstance& instance, VkDebugUtilsMessengerCreateInfoEXT& debugCreateInfo);
    VkDebugUtilsMessengerCreateInfoEXT makeDebugMessengerCreateInfo();
    VkResult createDebugUtilsMessengerEXT(VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
        const VkAllocationCallbacks* pAllocator);
    void destroyIfLayersEnabled();
    void destroyDebugUtilsMessengerEXT(const VkAllocationCallbacks* pAllocator);
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

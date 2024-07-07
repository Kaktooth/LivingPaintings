// This is a personal academic project. Dear PVS-Studio, please check it.

// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com

#include "debug.h"

using namespace std;

VkDebugUtilsMessengerEXT VulkanDebugMessenger::setup(VkInstance& instance, VkDebugUtilsMessengerCreateInfoEXT& debugCreateInfo)
{
    if (!Constants::ENABLE_VALIDATION_LAYERS)
        return VK_NULL_HANDLE;

    this->instance = instance;
    if (createDebugUtilsMessengerEXT(&debugCreateInfo, nullptr) != VK_SUCCESS) {
        throw runtime_error("failed to set up debug messenger...");
    }

    return debugMessenger;
}

VkDebugUtilsMessengerCreateInfoEXT VulkanDebugMessenger::makeDebugMessengerCreateInfo()
{
    VkDebugUtilsMessengerCreateInfoEXT debugInfo {};
    debugInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    debugInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    debugInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    debugInfo.pfnUserCallback = VulkanDebugMessenger::debugCallback;
    return debugInfo;
}

VkResult VulkanDebugMessenger::createDebugUtilsMessengerEXT(VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
    const VkAllocationCallbacks* pAllocator)
{
    const PFN_vkCreateDebugUtilsMessengerEXT func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
        instance, "vkCreateDebugUtilsMessengerEXT");
    if (func != nullptr) {
        return func(instance, pCreateInfo, pAllocator, &debugMessenger);
    } else {
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
}

void VulkanDebugMessenger::destroyDebugUtilsMessengerEXT(const VkAllocationCallbacks* pAllocator)
{
    const PFN_vkDestroyDebugUtilsMessengerEXT func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
        instance, "vkDestroyDebugUtilsMessengerEXT");
    if (func != nullptr) {
        func(instance, debugMessenger, pAllocator);
    }
}

void VulkanDebugMessenger::destroyIfLayersEnabled()
{
    if (Constants::ENABLE_VALIDATION_LAYERS) {
        VulkanDebugMessenger::destroyDebugUtilsMessengerEXT(nullptr);
    }
}

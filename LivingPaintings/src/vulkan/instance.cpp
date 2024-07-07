// This is a personal academic project. Dear PVS-Studio, please check it.

// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com

#include "instance.h"

using namespace std;

VkInstance& VulkanInstance::create(VkDebugUtilsMessengerCreateInfoEXT& debugCreateInfo)
{
    if (Constants::ENABLE_VALIDATION_LAYERS && !checkValidationLayerSupport()) {
        throw runtime_error("validation layers requested, but not available!");
    }

    VkApplicationInfo appInfo {};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = Constants::APP_NAME;
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = Constants::ENGINE_NAME;
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_2;

    VkInstanceCreateInfo instanceInfo {};
    instanceInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instanceInfo.pApplicationInfo = &appInfo;

    const std::vector<const char*> requiredExtentions = findRequiredExtensions();
    instanceInfo.enabledExtensionCount = static_cast<uint32_t>(requiredExtentions.size());
    instanceInfo.ppEnabledExtensionNames = requiredExtentions.data();

    enumerateExtentions();

    if (Constants::ENABLE_VALIDATION_LAYERS) {
        instanceInfo.enabledLayerCount = static_cast<uint32_t>(Constants::VALIDATION_LAYERS.size());
        instanceInfo.ppEnabledLayerNames = Constants::VALIDATION_LAYERS.data();

        instanceInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;
    } else {
        instanceInfo.enabledLayerCount = 0;
        instanceInfo.pNext = NULL;
    }

    if (vkCreateInstance(&instanceInfo, nullptr, &instance) != VK_SUCCESS) {
        throw runtime_error("Failed to create instance.");
    }

    return instance;
}

void VulkanInstance::enumerateExtentions()
{
    uint32_t extentionCount = 0;
    vkEnumerateInstanceExtensionProperties(nullptr, &extentionCount, nullptr);

    vector<VkExtensionProperties> extensions(extentionCount);

    vkEnumerateInstanceExtensionProperties(nullptr, &extentionCount, extensions.data());

    cout << "Available extensions:\n";

    for (const VkExtensionProperties& extension : extensions) {
        cout << '\t' << extension.extensionName << '\n';
    }
}

vector<const char*> VulkanInstance::findRequiredExtensions() const
{
    uint32_t glfwExtensionCount = 0;
    const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
    vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

    if (Constants::ENABLE_VALIDATION_LAYERS) {
        extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }

    return extensions;
}

bool VulkanInstance::checkValidationLayerSupport() const
{
    uint32_t layerCount;
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

    vector<VkLayerProperties> availableLayers(layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

    for (const char* layerName : Constants::VALIDATION_LAYERS) {
        bool layerFound = false;

        for (const VkLayerProperties& layerProperties : availableLayers) {
            if (strcmp(layerName, layerProperties.layerName) == 0) {
                layerFound = true;
                break;
            }
        }

        if (!layerFound) {
            return false;
        }
    }
    return true;
}

void VulkanInstance::destroy()
{
    vkDestroyInstance(VulkanInstance::instance, nullptr);
}

VkInstance& VulkanInstance::get()
{
    return VulkanInstance::instance;
}

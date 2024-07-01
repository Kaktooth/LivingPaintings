// This is a personal academic project. Dear PVS-Studio, please check it.

// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com

#include "instance.h"

using namespace std;

void VulkanInstance::create(VkDebugUtilsMessengerCreateInfoEXT& debugCreateInfo)
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

    auto extentions = getRequiredExtensions();
    instanceInfo.enabledExtensionCount = static_cast<uint32_t>(extentions.size());
    instanceInfo.ppEnabledExtensionNames = extentions.data();

    enumerateExtentions();

    if (Constants::ENABLE_VALIDATION_LAYERS) {
        instanceInfo.enabledLayerCount = static_cast<uint32_t>(Constants::VALIDATION_LAYERS.size());
        instanceInfo.ppEnabledLayerNames = Constants::VALIDATION_LAYERS.data();

        instanceInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;
    } else {
        instanceInfo.enabledLayerCount = 0;
        instanceInfo.pNext = NULL;
    }

    VkResult result = vkCreateInstance(&instanceInfo, nullptr, &instance);

    if (result != VK_SUCCESS) {
        throw runtime_error("failed to create instance!");
    }
}

void VulkanInstance::enumerateExtentions()
{
    uint32_t extentionCount = 0;
    vkEnumerateInstanceExtensionProperties(nullptr, &extentionCount, nullptr);

    vector<VkExtensionProperties> extensions(extentionCount);

    vkEnumerateInstanceExtensionProperties(nullptr, &extentionCount, extensions.data());

    cout << "available extensions:\n";

    for (const auto& extension : extensions) {
        cout << '\t' << extension.extensionName << '\n';
    }
}

vector<const char*> VulkanInstance::getRequiredExtensions() const
{
    uint32_t glfwExtensionCount = 0;
    const char** glfwExtensions;
    glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

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

        for (const auto& layerProperties : availableLayers) {
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

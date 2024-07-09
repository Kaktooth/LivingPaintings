// This is a personal academic project. Dear PVS-Studio, please check it.

// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com

#include "device.h"

using namespace std;

VkDevice& Device::create(VkInstance& instance, Surface& surface)
{
    vector<VkDeviceQueueCreateInfo> queueCreateInfos;
    float queuePriority = 1.0f;

    this->surface = surface.get();

    selectPhysicalDevice(instance, surface);

    set<uint32_t> queueFamilies = {
        queueFamily.indicies.graphicsFamily.value(),
        queueFamily.indicies.presentationFamily.value(),
        queueFamily.indicies.transferFamily.value()
    };

    for (const uint32_t queueFamily : queueFamilies) {
        VkDeviceQueueCreateInfo queueCreateInfo = createQueueCreateInfo(queueFamily, queuePriority);
        queueCreateInfos.push_back(queueCreateInfo);
    }

    const VkPhysicalDeviceFeatures deviceFeatures = selectedDeviceFeatures();
    VkDeviceCreateInfo deviceInfo {};
    deviceInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    deviceInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
    deviceInfo.pQueueCreateInfos = queueCreateInfos.data();
    deviceInfo.pEnabledFeatures = &deviceFeatures;
    deviceInfo.enabledExtensionCount = static_cast<uint32_t>(Constants::DEVICE_EXTENTIONS.size());
    deviceInfo.ppEnabledExtensionNames = Constants::DEVICE_EXTENTIONS.data();

    if (Constants::ENABLE_VALIDATION_LAYERS) {
        deviceInfo.enabledExtensionCount = static_cast<uint32_t>(Constants::VALIDATION_LAYERS.size());
        deviceInfo.ppEnabledLayerNames = Constants::VALIDATION_LAYERS.data();
    } else {
        deviceInfo.enabledLayerCount = 0;
    }

    if (vkCreateDevice(physicalDevice, &deviceInfo, nullptr, &device) != VK_SUCCESS) {
        throw runtime_error("Failed to create logical device.");
    }

    vkGetDeviceQueue(device, queueFamily.indicies.graphicsFamily.value(), 0,
        &graphicsQueue.get());
    vkGetDeviceQueue(device, queueFamily.indicies.presentationFamily.value(), 0,
        &presentationQueue.get());
    vkGetDeviceQueue(device, queueFamily.indicies.transferFamily.value(), 0,
        &transferQueue.get());

    return device;
}

void Device::selectPhysicalDevice(VkInstance& instance, Surface& surface)
{
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);

    if (!deviceCount) {
        throw runtime_error("Failed to find any device.");
    }

    vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

    multimap<int, VkPhysicalDevice> deviceCandidates;
    for (VkPhysicalDevice& device : devices) {
        const int deviceScore = Device::getDeviceScore(device, surface);
        deviceCandidates.insert({ deviceScore, device });
    }

    physicalDevice = deviceCandidates.begin()->second;

    if (physicalDevice == VK_NULL_HANDLE) {
        throw runtime_error("Failed to find a suitible device.");
    }
}

int Device::getDeviceScore(VkPhysicalDevice& physicalDevice, Surface& surface)
{
    VkPhysicalDeviceProperties deviceProperties;
    VkPhysicalDeviceFeatures deviceFeatures;
    vkGetPhysicalDeviceProperties(physicalDevice, &deviceProperties);
    vkGetPhysicalDeviceFeatures(physicalDevice, &deviceFeatures);
    queueFamily.findQueueFamilies(physicalDevice, surface.get());

    int score = 0;
    if (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
        score += 500;
    }
    if (deviceProperties.limits.maxSamplerAnisotropy > 14.0f) {
        score += 100;
    }
    if (deviceProperties.limits.maxFramebufferWidth > 8000) {
        score += 200;
    }

    bool extensionsSupported = checkDeviceExtensionSupport(physicalDevice);
    bool supportedSurfaceCapabilities = !surface.details.formats.empty() && !surface.details.presentationModes.empty();
    if (!(deviceFeatures.geometryShader || queueFamily.indicies.isAvailable() || supportedSurfaceCapabilities || extensionsSupported)) {
        return 0;
    }

    return score;
}

bool Device::checkDeviceExtensionSupport(VkPhysicalDevice& physicalDevice)
{
    set<string> requiredExtensions(Constants::DEVICE_EXTENTIONS.begin(),
        Constants::DEVICE_EXTENTIONS.end());

    uint32_t extensionCount;
    vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensionCount, nullptr);

    vector<VkExtensionProperties> availableExtensions(extensionCount);
    vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensionCount, availableExtensions.data());

    for (VkExtensionProperties& extension : availableExtensions) {
        requiredExtensions.erase(extension.extensionName);
    }

    return requiredExtensions.empty();
}

VkPhysicalDeviceFeatures Device::selectedDeviceFeatures()
{
    VkPhysicalDeviceFeatures deviceFeatures;
    vkGetPhysicalDeviceFeatures(physicalDevice, &deviceFeatures);
    return deviceFeatures;
}

VkDeviceQueueCreateInfo Device::createQueueCreateInfo(const uint32_t queueFamily,
    const float queuePriority)
{
    VkDeviceQueueCreateInfo queueInfo {};
    queueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queueInfo.queueFamilyIndex = queueFamily;
    queueInfo.queueCount = 1;
    queueInfo.pQueuePriorities = &queuePriority;

    return queueInfo;
}

void Device::destroy()
{
    vkDestroyDevice(device, nullptr);
}

VkDevice& Device::get()
{
    return device;
}

VkPhysicalDevice& Device::getPhysicalDevice()
{
    return physicalDevice;
}

VkSurfaceKHR Device::getSurface()
{
    return surface;
}

Queue& Device::getGraphicsQueue()
{
    return graphicsQueue;
}

Queue& Device::getPresentationQueue()
{
    return presentationQueue;
}

Queue& Device::getTransferQueue()
{
    return transferQueue;
}

QueueFamily& Device::getQueueFamily()
{
    return queueFamily;
}

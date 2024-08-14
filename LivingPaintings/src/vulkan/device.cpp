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
        queueFamily.indicies.transferFamily.value(),
        queueFamily.indicies.computeFamily.value()
    };

    for (const uint32_t queueFamily : queueFamilies) {
        VkDeviceQueueCreateInfo queueCreateInfo = createQueueCreateInfo(queueFamily, queuePriority);
        queueCreateInfos.push_back(queueCreateInfo);
    }

    vkGetPhysicalDeviceProperties(physicalDevice, &deviceProperties);
    vkGetPhysicalDeviceFeatures(physicalDevice, &deviceFeatures);

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

    graphicsQueue.create(device, queueFamily.indicies.graphicsFamily.value());
    presentationQueue.create(device, queueFamily.indicies.presentationFamily.value());
    transferQueue.create(device, queueFamily.indicies.transferFamily.value());
    computeQueue.create(device, queueFamily.indicies.computeFamily.value());

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

    deviceFeatures.sampleRateShading = VK_TRUE;
    deviceFeatures.samplerAnisotropy = VK_TRUE;

    physicalDevice = deviceCandidates.begin()->second;

    if (physicalDevice == VK_NULL_HANDLE) {
        throw runtime_error("Failed to find a suitible device.");
    }
}

int Device::getDeviceScore(VkPhysicalDevice& physicalDevice, Surface& surface)
{
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
    if (!(deviceFeatures.geometryShader || deviceFeatures.sampleRateShading || deviceFeatures.samplerAnisotropy || queueFamily.indicies.isAvailable()
            || supportedSurfaceCapabilities || extensionsSupported)) {
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

VkFormat Device::findSupportedFormat(const std::vector<VkFormat>& formats,
    VkImageTiling tiling,
    VkFormatFeatureFlags features)
{
    for (const VkFormat format : formats) {
        VkFormatProperties formatProperties;
        vkGetPhysicalDeviceFormatProperties(physicalDevice, format, &formatProperties);
        if (tiling == VK_IMAGE_TILING_LINEAR && (formatProperties.linearTilingFeatures & features) == features
            || tiling == VK_IMAGE_TILING_OPTIMAL && (formatProperties.optimalTilingFeatures & features) == features) {
            return format;
        }
    }
}

bool Device::hasStencilComponent(VkFormat format)
{
    return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT;
}

VkSampleCountFlagBits Device::getMaxSampleCount()
{
    VkSampleCountFlags sampleNumber = deviceProperties.limits.framebufferColorSampleCounts
        & deviceProperties.limits.framebufferDepthSampleCounts;
    std::array<VkSampleCountFlagBits, 6> sampleRates {
        VK_SAMPLE_COUNT_64_BIT, VK_SAMPLE_COUNT_32_BIT, VK_SAMPLE_COUNT_16_BIT,
        VK_SAMPLE_COUNT_8_BIT, VK_SAMPLE_COUNT_4_BIT, VK_SAMPLE_COUNT_2_BIT
    };

    for (auto& sampleRate : sampleRates) {
        if (sampleRate & sampleNumber) {
            return sampleRate;
        }
    }
    return VK_SAMPLE_COUNT_1_BIT;
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

Queue& Device::getComputeQueue()
{
    return computeQueue;
}

QueueFamily& Device::getQueueFamily()
{
    return queueFamily;
}

#include "device.h"

VkDevice& Device::create(VkInstance& instance, Surface& surface)
{
    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
    float queuePriority = 1.0f;

    this->surface = surface.get();

    selectPhysicalDevice(instance, surface);

    std::set<uint32_t> queueFamilies = {
        queueFamily.indicies.graphicsFamily.value(),
        queueFamily.indicies.presentationFamily.value(),
        queueFamily.indicies.transferFamily.value(),
        queueFamily.indicies.computeFamily.value()
    };

    for (const uint32_t queueFamily : queueFamilies) {
        VkDeviceQueueCreateInfo queueCreateInfo = createQueueCreateInfo(queueFamily, queuePriority);
        queueCreateInfos.push_back(queueCreateInfo);
    }

    // TODO to device select
    vkGetPhysicalDeviceProperties(physicalDevice, &deviceProperties);
    vkGetPhysicalDeviceFeatures(physicalDevice, &deviceFeatures);

    VkPhysicalDeviceFeatures2 deviceIndexingFeatures{ VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2, &indexingFeatures };
    vkGetPhysicalDeviceFeatures2(physicalDevice, &deviceIndexingFeatures);
    vkGetPhysicalDeviceFeatures2(physicalDevice, &deviceFeatures2);

    VkDeviceCreateInfo deviceInfo {};
    deviceInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    deviceInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
    deviceInfo.pQueueCreateInfos = queueCreateInfos.data();
    deviceInfo.enabledExtensionCount = static_cast<uint32_t>(Constants::DEVICE_EXTENTIONS.size());
    deviceInfo.ppEnabledExtensionNames = Constants::DEVICE_EXTENTIONS.data();
    deviceInfo.pNext = &deviceFeatures2;

    bool bindlessSupported = indexingFeatures.descriptorBindingPartiallyBound && indexingFeatures.runtimeDescriptorArray;
    if (bindlessSupported) {
        indexingFeatures.descriptorBindingPartiallyBound = VK_TRUE;
        indexingFeatures.runtimeDescriptorArray = VK_TRUE;
        deviceFeatures2.pNext = &indexingFeatures;
    }

    if (Constants::ENABLE_VALIDATION_LAYERS) {
        deviceInfo.enabledExtensionCount = static_cast<uint32_t>(Constants::VALIDATION_LAYERS.size());
        deviceInfo.ppEnabledLayerNames = Constants::VALIDATION_LAYERS.data();
    } else {
        deviceInfo.enabledLayerCount = 0;
    }

    if (vkCreateDevice(physicalDevice, &deviceInfo, nullptr, &device) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create logical device.");
    }

    graphicsQueue.create(device, queueFamily.indicies.graphicsFamily.value());
    presentationQueue.create(device, queueFamily.indicies.presentationFamily.value());
    transferQueue.create(device, queueFamily.indicies.transferFamily.value());
    computeQueue.create(device, queueFamily.indicies.computeFamily.value());

    return device;
}

void Device::selectPhysicalDevice(VkInstance& instance, Surface& surface)
{
    uint32_t deviceCount;
    vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);

    if (!deviceCount) {
        throw std::runtime_error("Failed to find any device.");
    }

    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

    std::multimap<int, VkPhysicalDevice> deviceCandidates;
    for (VkPhysicalDevice& device : devices) {
        const int deviceScore = Device::getDeviceScore(device, surface);
        deviceCandidates.insert({ deviceScore, device });
    }

    physicalDevice = deviceCandidates.begin()->second;

    if (physicalDevice == VK_NULL_HANDLE) {
        throw std::runtime_error("Failed to find a suitible device.");
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
    if (!(deviceFeatures.geometryShader || deviceFeatures.sampleRateShading 
        || deviceFeatures.samplerAnisotropy 
        || supportedSurfaceCapabilities || extensionsSupported
        || queueFamily.indicies.isAvailable())) {
        return 0;
    }

    return score;
}

bool Device::checkDeviceExtensionSupport(VkPhysicalDevice& physicalDevice)
{
    std::set<std::string> requiredExtensions(Constants::DEVICE_EXTENTIONS.begin(),
        Constants::DEVICE_EXTENTIONS.end());

    uint32_t extensionCount;
    vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensionCount, nullptr);

    std::vector<VkExtensionProperties> availableExtensions(extensionCount);
    vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensionCount, availableExtensions.data());

    for (VkExtensionProperties& extension : availableExtensions) {
        requiredExtensions.erase(extension.extensionName);
    }

    return requiredExtensions.empty();
}

VkFormat Device::findSupportedFormat(std::vector<VkFormat>& formats,
    VkImageTiling tiling,
    VkFormatFeatureFlags features)
{
    for (const VkFormat format : formats) {
        VkFormatProperties formatProperties;
        vkGetPhysicalDeviceFormatProperties(physicalDevice, format, &formatProperties);
        if ((tiling == VK_IMAGE_TILING_LINEAR && (formatProperties.linearTilingFeatures & features) == features)
            || (tiling == VK_IMAGE_TILING_OPTIMAL && (formatProperties.optimalTilingFeatures & features) == features)) {
            return format;
        }
    }
}

bool Device::hasStencilComponent(VkFormat format)
{
    return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT;
}

VkSampleCountFlagBits Device::getMaxSampleCount() const
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

VkPhysicalDeviceProperties Device::getProperties()
{
    return deviceProperties;
}

VkPhysicalDeviceFeatures Device::getFeatures()
{
    return deviceFeatures;
}

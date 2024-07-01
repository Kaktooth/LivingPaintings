// This is a personal academic project. Dear PVS-Studio, please check it.

// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com

#include "physical_device.h"
#include <algorithm>
#include <cstdint>
#include <limits>
#include <map>
#include <stdexcept>
#include <vector>

using namespace std;

void PhysicalDevice::select(VkInstance& instance, Surface& surface, QueueFamily& queueFamily)
{
    this->surface = surface;
    this->queueFamily = queueFamily;

    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);

    if (!deviceCount) {
        throw runtime_error("Failed to find any device.");
    }

    vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

    multimap<int, VkPhysicalDevice> deviceCandidates;
    for (const auto& device : devices) {
        deviceCandidates.insert({ PhysicalDevice::getDeviceScore(device), device });
    }

    physicalDevice = deviceCandidates.begin()->second;

    if (physicalDevice == VK_NULL_HANDLE) {
        throw runtime_error("Failed to find a suitible device.");
    }
}

int PhysicalDevice::getDeviceScore(VkPhysicalDevice device)
{
    int score = 0;
    VkPhysicalDeviceProperties deviceProperties;
    VkPhysicalDeviceFeatures deviceFeatures;
    vkGetPhysicalDeviceProperties(device, &deviceProperties);
    vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

    auto usedSurface = surface.get();
    QueueFamily::Indices indicies = queueFamily.findQueueFamilies(device, usedSurface);
    bool extensionsSupported = checkDeviceExtensionSupport(device);
    auto swapChainDetails = SwapChain::getDetails(device, usedSurface);
    bool swapChainSupported = !swapChainDetails.formats.empty() && !swapChainDetails.presentationModes.empty();

    if (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
        score += 500;
    }

    score += deviceProperties.limits.maxImageDimension2D;

    if (!(deviceFeatures.geometryShader || indicies.isAvailable() || swapChainSupported || extensionsSupported)) {
        return 0;
    }

    return score;
}

bool PhysicalDevice::checkDeviceExtensionSupport(VkPhysicalDevice device)
{
    uint32_t extensionCount;
    vkEnumerateDeviceExtensionProperties(
        device, nullptr, &extensionCount, nullptr);

    vector<VkExtensionProperties> availableExtensions(extensionCount);
    vkEnumerateDeviceExtensionProperties(
        device, nullptr, &extensionCount, availableExtensions.data());

    set<string> requiredExtensions(Constants::DEVICE_EXTENTIONS.begin(),
        Constants::DEVICE_EXTENTIONS.end());

    for (const auto& extension : availableExtensions) {
        requiredExtensions.erase(extension.extensionName);
    }

    return requiredExtensions.empty();
}

VkPhysicalDeviceFeatures PhysicalDevice::selectedDeviceFeatures()
{
    VkPhysicalDeviceFeatures deviceFeatures;
    vkGetPhysicalDeviceFeatures(physicalDevice, &deviceFeatures);
    return deviceFeatures;
}

VkPhysicalDevice PhysicalDevice::get()
{
    return physicalDevice;
}

Surface PhysicalDevice::usedSurface()
{
    return surface;
}

// This is a personal academic project. Dear PVS-Studio, please check it.

// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com

#include "swapchain.h"
#include <stdexcept>
#include <vector>

using namespace std;

void SwapChain::create(VkDevice device, VkPhysicalDevice physicalDevice, Surface surface, QueueFamily::Indices indicies)
{
    SwapChain::Details swapchainDetails = getDetails(physicalDevice, surface.get());

    surfaceFormat = chooseSurfaceFormat(swapchainDetails.formats);
    presentationMode = choosePresentationMode(swapchainDetails.presentationModes);
    extent = surface.chooseResolution(swapchainDetails.capabilities);
    imageFormat = surfaceFormat.format;

    minImageCount = swapchainDetails.capabilities.minImageCount + 1;
    if (swapchainDetails.capabilities.maxImageCount > 0 && minImageCount > swapchainDetails.capabilities.maxImageCount) {
        minImageCount = swapchainDetails.capabilities.maxImageCount;
    }

    VkSwapchainCreateInfoKHR swapchainInfo {};
    swapchainInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    swapchainInfo.surface = surface.get();
    swapchainInfo.minImageCount = minImageCount;
    swapchainInfo.imageFormat = surfaceFormat.format;
    swapchainInfo.imageColorSpace = surfaceFormat.colorSpace;
    swapchainInfo.imageExtent = extent;
    swapchainInfo.imageArrayLayers = 1;
    swapchainInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    uint32_t queueFamilyIndicies[] = { indicies.graphicsFamily.value(), indicies.presentationFamily.value(), indicies.transferFamily.value() };
    if (indicies.graphicsFamily != indicies.presentationFamily || indicies.presentationFamily != indicies.transferFamily) {
        swapchainInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        swapchainInfo.queueFamilyIndexCount = 3;
        swapchainInfo.pQueueFamilyIndices = queueFamilyIndicies;
    } else {
        swapchainInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    }
    swapchainInfo.preTransform = swapchainDetails.capabilities.currentTransform;
    swapchainInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    swapchainInfo.presentMode = presentationMode;
    swapchainInfo.clipped = VK_TRUE;
    swapchainInfo.oldSwapchain = VK_NULL_HANDLE;

    if (vkCreateSwapchainKHR(device, &swapchainInfo, nullptr, &swapChain) != VK_SUCCESS) {
        throw runtime_error("Failed to create swap chain.");
    }

    uint32_t imageCount;
    vkGetSwapchainImagesKHR(device, swapChain, &imageCount, nullptr);
    images.resize(imageCount);
    vkGetSwapchainImagesKHR(device, swapChain, &imageCount, images.data());

    currentFrame = 0;
    framebufferResized = false;
}

void SwapChain::createImageViews(VkDevice device)
{
    imageViews.resize(images.size());
    for (size_t i = 0; i < images.size(); i++) {
        VkImageViewCreateInfo createInfo {};
        createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        createInfo.image = images[i];
        createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        createInfo.format = imageFormat;
        createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        createInfo.subresourceRange.baseMipLevel = 0;
        createInfo.subresourceRange.baseArrayLayer = 0;
        createInfo.subresourceRange.levelCount = 1;
        createInfo.subresourceRange.layerCount = 1;

        if (vkCreateImageView(device, &createInfo, nullptr, &imageViews[i]) != VK_SUCCESS) {
            throw runtime_error("Failed to create image view.");
        }
    }
}

void SwapChain::createFramebuffers(VkDevice device, VkRenderPass renderPass)
{
    framebuffers.resize(imageViews.size());

    for (size_t i = 0; i < imageViews.size(); i++) {
        VkImageView attachments[] = {
            imageViews[i]
        };

        VkFramebufferCreateInfo framebufferInfo {};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = renderPass;
        framebufferInfo.attachmentCount = 1;
        framebufferInfo.pAttachments = attachments;
        framebufferInfo.width = extent.width;
        framebufferInfo.height = extent.height;
        framebufferInfo.layers = 1;

        if (vkCreateFramebuffer(device, &framebufferInfo, nullptr, &framebuffers[i]) != VK_SUCCESS) {
            throw runtime_error("Failed to create framebuffer.");
        }
    }
}

SwapChain::Details
SwapChain::getDetails(VkPhysicalDevice& device, VkSurfaceKHR& surface)
{

    SwapChain::Details swapChainDetails;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
        device, surface, &swapChainDetails.capabilities);

    uint32_t formatsCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatsCount, nullptr);

    if (formatsCount != 0) {
        swapChainDetails.formats.resize(formatsCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(
            device, surface, &formatsCount, swapChainDetails.formats.data());
    }

    uint32_t presentationModsCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(
        device, surface, &presentationModsCount, nullptr);

    if (formatsCount != 0) {
        swapChainDetails.presentationModes.resize(presentationModsCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(
            device,
            surface,
            &presentationModsCount,
            swapChainDetails.presentationModes.data());
    }
    return swapChainDetails;
}

VkSurfaceFormatKHR
SwapChain::chooseSurfaceFormat(
    const vector<VkSurfaceFormatKHR>& availableFormats)
{
    for (const auto& availableFormat : availableFormats) {
        if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            return availableFormat;
        }
    }

    return availableFormats[0];
}

VkPresentModeKHR
SwapChain::choosePresentationMode(
    const std::vector<VkPresentModeKHR>& availablePresentationModes)
{
    for (const auto& presentationMode : availablePresentationModes) {
        if (presentationMode == VK_PRESENT_MODE_MAILBOX_KHR) {
            return VK_PRESENT_MODE_MAILBOX_KHR;
        }
    }
    return VK_PRESENT_MODE_FIFO_KHR;
}

uint32_t SwapChain::asquireNextImage(VkDevice device, VkPhysicalDevice physicalDevice, VkRenderPass renderPass, Surface surface, QueueFamily::Indices indicies, Semaphore imageAvailable, GLFWwindow* window)
{
    uint32_t imageIndex;
    auto result = vkAcquireNextImageKHR(device, swapChain, UINT64_MAX, imageAvailable.get(currentFrame), VK_NULL_HANDLE, &imageIndex);

    if (result == VK_ERROR_OUT_OF_DATE_KHR) {
        recreate(device, physicalDevice, renderPass, surface, indicies, window);
        return 0;
    } else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
        throw runtime_error("Failed to acquire swap chain image.");
    }

    return imageIndex;
}

void SwapChain::presentImage(VkDevice device, VkPhysicalDevice physicalDevice, VkRenderPass renderPass, Surface surface, QueueFamily::Indices indicies, VkQueue& presentationQueue, vector<VkSemaphore> signalSemafores, GLFWwindow* window)
{
    VkSwapchainKHR swapChains[] = { swapChain };

    VkPresentInfoKHR presentationInfo {};
    presentationInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentationInfo.waitSemaphoreCount = signalSemafores.size();
    presentationInfo.pWaitSemaphores = signalSemafores.data();
    presentationInfo.swapchainCount = 1;
    presentationInfo.pSwapchains = swapChains;
    presentationInfo.pImageIndices = &currentFrame;
    presentationInfo.pResults = nullptr;

    VkResult result = vkQueuePresentKHR(presentationQueue, &presentationInfo);
    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || framebufferResized) {
        recreate(device, physicalDevice, renderPass, surface, indicies, window);
        framebufferResized = false;
        currentFrame = 0;
        return;
    } else if (result != VK_SUCCESS) {
        throw runtime_error("Failed to present swap chain image.");
    }

    nextFrame();
}

void SwapChain::nextFrame()
{
    currentFrame = (currentFrame + 1) % Constants::MAX_FRAMES_IN_FLIGHT;
}

void SwapChain::destroy(VkDevice device)
{
    for (auto imageView : imageViews) {
        vkDestroyImageView(device, imageView, nullptr);
    }

    for (const auto framebuffer : framebuffers) {
        vkDestroyFramebuffer(device, framebuffer, nullptr);
    }

    imageViews.clear();
    framebuffers.clear();
    vkDestroySwapchainKHR(device, swapChain, nullptr);
}

void SwapChain::recreate(VkDevice device, VkPhysicalDevice physicalDevice, VkRenderPass renderPass, Surface surface, QueueFamily::Indices indicies, GLFWwindow* window)
{
    int width = 0, height = 0;
    glfwGetFramebufferSize(window, &width, &height);
    while (width == 0 || height == 0) {
        if (glfwWindowShouldClose(window)) {
            return;
        }

        glfwGetFramebufferSize(window, &width, &height);
        glfwWaitEvents();
    }

    vkDeviceWaitIdle(device);

    destroy(device);

    create(device, physicalDevice, surface, indicies);
    createImageViews(device);
    createFramebuffers(device, renderPass);
}

uint32_t SwapChain::getMinImageCount()
{
    return minImageCount;
}

VkFormat& SwapChain::getImageFormat()
{
    return imageFormat;
}

VkExtent2D& SwapChain::getExtent()
{
    return extent;
}

vector<VkImageView>& SwapChain::getImageViews()
{
    return imageViews;
}

uint32_t& SwapChain::getCurrentFrame()
{
    return currentFrame;
};

vector<VkFramebuffer>& SwapChain::getFramebuffers()
{
    return framebuffers;
}

void SwapChain::resizeFramebuffer()
{
    framebufferResized = true;
}

// This is a personal academic project. Dear PVS-Studio, please check it.

// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com

#include "swapchain.h"

using namespace std;

void Swapchain::setDeviceContext(Device& _device, Surface& _surface)
{
    this->device = _device.get();
    this->physicalDevice = _device.getPhysicalDevice();
    this->surface = _surface;
    this->queueFamilyIndicies = _device.getQueueFamily().indicies;
}

void Swapchain::create()
{
    const uint32_t indicies[] = { queueFamilyIndicies.graphicsFamily.value(),
        queueFamilyIndicies.presentationFamily.value(),
        queueFamilyIndicies.transferFamily.value() };

    const VkSurfaceFormatKHR surfaceFormat = surface.chooseSurfaceFormat();
    const VkPresentModeKHR presentMode = surface.choosePresentationMode();
    imageFormat = surfaceFormat.format;
    extent = surface.chooseResolution();
    minImageCount = surface.details.capabilities.minImageCount + 1;
    if (surface.details.capabilities.maxImageCount > 0 && minImageCount > surface.details.capabilities.maxImageCount) {
        minImageCount = surface.details.capabilities.maxImageCount;
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

    if (queueFamilyIndicies.graphicsFamily != queueFamilyIndicies.presentationFamily || queueFamilyIndicies.presentationFamily != queueFamilyIndicies.transferFamily) {
        swapchainInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        swapchainInfo.queueFamilyIndexCount = 3;
        swapchainInfo.pQueueFamilyIndices = indicies;
    } else {
        swapchainInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    }
    swapchainInfo.preTransform = surface.details.capabilities.currentTransform;
    swapchainInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    swapchainInfo.presentMode = presentMode;
    swapchainInfo.clipped = VK_TRUE;
    swapchainInfo.oldSwapchain = VK_NULL_HANDLE;

    if (vkCreateSwapchainKHR(device, &swapchainInfo, nullptr, &swapchain) != VK_SUCCESS) {
        throw runtime_error("Failed to create swap chain.");
    }

    uint32_t imageCount;
    vkGetSwapchainImagesKHR(device, swapchain, &imageCount, nullptr);
    images.resize(imageCount);
    vkGetSwapchainImagesKHR(device, swapchain, &imageCount, images.data());

    currentFrame = 0;
    framebufferResized = false;
}

void Swapchain::createImageViews()
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

void Swapchain::createFramebuffers(VkRenderPass& renderPass)
{
    framebuffers.resize(imageViews.size());

    for (size_t i = 0; i < imageViews.size(); i++) {
        VkImageView attachments[] = { imageViews[i] };

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

uint32_t Swapchain::asquireNextImage(VkRenderPass& renderPass, VkSemaphore& imageAvailable, GLFWwindow* window)
{
    uint32_t imageIndex;
    VkResult result = vkAcquireNextImageKHR(device, swapchain, UINT64_MAX, imageAvailable, VK_NULL_HANDLE, &imageIndex);

    if (result == VK_ERROR_OUT_OF_DATE_KHR) {
        recreate(renderPass, window);
        return 0;
    } else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
        throw runtime_error("Failed to acquire swap chain image.");
    }

    return imageIndex;
}

void Swapchain::presentImage(VkRenderPass& renderPass, VkQueue& presentationQueue,
    const vector<VkSemaphore> signalSemafores, GLFWwindow* window)
{
    VkSwapchainKHR swapChains[] = { swapchain };

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
        recreate(renderPass, window);
        framebufferResized = false;
        currentFrame = 0;
        return;
    } else if (result != VK_SUCCESS) {
        throw runtime_error("Failed to present swap chain image.");
    }

    nextFrame();
}

void Swapchain::nextFrame()
{
    currentFrame = (currentFrame + 1) % Constants::MAX_FRAMES_IN_FLIGHT;
}

void Swapchain::destroy()
{
    for (VkImageView imageView : imageViews) {
        vkDestroyImageView(device, imageView, nullptr);
    }

    for (VkFramebuffer framebuffer : framebuffers) {
        vkDestroyFramebuffer(device, framebuffer, nullptr);
    }

    imageViews.clear();
    framebuffers.clear();
    vkDestroySwapchainKHR(device, swapchain, nullptr);
}

void Swapchain::recreate(VkRenderPass& renderPass, GLFWwindow* window)
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

    destroy();

    create();
    createImageViews();
    createFramebuffers(renderPass);
}

uint32_t Swapchain::getMinImageCount()
{
    return minImageCount;
}

VkFormat& Swapchain::getImageFormat()
{
    return imageFormat;
}

VkExtent2D& Swapchain::getExtent()
{
    return extent;
}

vector<VkImageView>& Swapchain::getImageViews()
{
    return imageViews;
}

uint32_t& Swapchain::getCurrentFrame()
{
    return currentFrame;
};

vector<VkFramebuffer>& Swapchain::getFramebuffers()
{
    return framebuffers;
}

void Swapchain::resizeFramebuffer()
{
    framebufferResized = true;
}

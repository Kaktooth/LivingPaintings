// This is a personal academic project. Dear PVS-Studio, please check it.

// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com

#include "swapchain.h"

using namespace std;

void Swapchain::setContext(Device& device, Surface& surface,
    VkCommandPool commandPool,
    VkSampleCountFlagBits samples,
    std::vector<VkFormat> depthFormatCandidates)
{
    this->device = device.get();
    this->physicalDevice = device.getPhysicalDevice();
    this->surface = surface;
    this->queueFamilyIndicies = device.getQueueFamily().indicies;
    this->commandPool = commandPool;
    this->samples = samples;

    const VkSurfaceFormatKHR surfaceFormat = surface.chooseSurfaceFormat();
    colorSpace = surfaceFormat.colorSpace;
    imageFormat = surfaceFormat.format;
    extent = surface.chooseResolution();
    depthFormat = device.findSupportedFormat(
        depthFormatCandidates, VK_IMAGE_TILING_OPTIMAL,
        VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
    aspectFlags = device.hasStencilComponent(depthFormat)
        ? VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT
        : VK_IMAGE_ASPECT_DEPTH_BIT;

    createSpecializedImages(device);
}

void Swapchain::createSpecializedImages(Device& device)
{
    depthImage.imageDetails.createImageInfo(
        "", extent.width, extent.height, 4,
        VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, VK_IMAGE_VIEW_TYPE_2D,
        depthFormat, VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
        VK_IMAGE_TILING_OPTIMAL, aspectFlags, samples);
    depthImage.create(device, commandPool,
        VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    colorImage.imageDetails.createImageInfo(
        "", extent.width, extent.height, 4, VK_IMAGE_LAYOUT_UNDEFINED,
        VK_IMAGE_VIEW_TYPE_2D, imageFormat, VK_SHADER_STAGE_FRAGMENT_BIT,
        VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_ASPECT_COLOR_BIT, samples);
    colorImage.create(device, commandPool,
        VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
}

void Swapchain::create()
{
    const uint32_t indicies[] = {
        queueFamilyIndicies.graphicsFamily.value(),
        queueFamilyIndicies.presentationFamily.value(),
        queueFamilyIndicies.transferFamily.value(),
        queueFamilyIndicies.computeFamily.value()
    };

    const VkPresentModeKHR presentMode = surface.choosePresentationMode();

    minImageCount = surface.details.capabilities.minImageCount + 1;
    if (surface.details.capabilities.maxImageCount > 0 && minImageCount > surface.details.capabilities.maxImageCount) {
        minImageCount = surface.details.capabilities.maxImageCount;
    }

    VkSwapchainCreateInfoKHR swapchainInfo {};
    swapchainInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    swapchainInfo.surface = surface.get();
    swapchainInfo.minImageCount = minImageCount;
    swapchainInfo.imageFormat = imageFormat;
    swapchainInfo.imageColorSpace = colorSpace;
    swapchainInfo.imageExtent = extent;
    swapchainInfo.imageArrayLayers = 1;
    swapchainInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    if (queueFamilyIndicies.graphicsFamily != queueFamilyIndicies.presentationFamily
        || queueFamilyIndicies.presentationFamily != queueFamilyIndicies.transferFamily
        || queueFamilyIndicies.graphicsFamily != queueFamilyIndicies.computeFamily) {
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
    for (int i = 0; i < images.size(); i++) {
        imageViews[i] = createImageView(images[i], imageFormat, VK_IMAGE_ASPECT_COLOR_BIT);
    }
}

VkImageView& Swapchain::createImageView(VkImage& image, VkFormat& format,
    VkImageAspectFlags aspectFlags)
{
    VkImageViewCreateInfo imageViewInfo {};
    imageViewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    imageViewInfo.image = image;
    imageViewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    imageViewInfo.format = format;
    imageViewInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
    imageViewInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
    imageViewInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
    imageViewInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
    imageViewInfo.subresourceRange.aspectMask = aspectFlags;
    imageViewInfo.subresourceRange.baseMipLevel = 0;
    imageViewInfo.subresourceRange.baseArrayLayer = 0;
    imageViewInfo.subresourceRange.levelCount = 1;
    imageViewInfo.subresourceRange.layerCount = 1;

    VkImageView imageView;
    if (vkCreateImageView(device, &imageViewInfo, nullptr, &imageView) != VK_SUCCESS) {
        throw runtime_error("Failed to create image view.");
    }
    return imageView;
}

void Swapchain::createFramebuffers(VkRenderPass& renderPass)
{
    framebuffers.resize(imageViews.size());

    VkImageView& depthImageView = depthImage.getView();
    VkImageView& colorImageView = colorImage.getView();
    for (int i = 0; i < imageViews.size(); i++) {
        std::vector<VkImageView> attachments = { colorImageView, depthImageView,
            imageViews[i] };

        VkFramebufferCreateInfo framebufferInfo {};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = renderPass;
        framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
        framebufferInfo.pAttachments = attachments.data();
        framebufferInfo.width = extent.width;
        framebufferInfo.height = extent.height;
        framebufferInfo.layers = 1;

        if (vkCreateFramebuffer(device, &framebufferInfo, nullptr, &framebuffers[i]) != VK_SUCCESS) {
            throw runtime_error("Failed to create framebuffer.");
        }
    }
}

uint32_t Swapchain::asquireNextImage(Device& device, VkRenderPass& renderPass, VkSemaphore& imageAvailable, GLFWwindow* pWindow)
{
    uint32_t imageIndex;
    VkResult result = vkAcquireNextImageKHR(this->device, swapchain, UINT64_MAX, imageAvailable, VK_NULL_HANDLE, &imageIndex);

    if (result == VK_ERROR_OUT_OF_DATE_KHR) {
        recreate(device, renderPass, pWindow);
        return 0;
    } else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
        throw runtime_error("Failed to acquire swap chain image.");
    }

    return imageIndex;
}

void Swapchain::presentImage(Device& device, VkRenderPass& renderPass, VkQueue& presentationQueue,
    const vector<VkSemaphore> signalSemafores, GLFWwindow* pWindow)
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
        recreate(device, renderPass, pWindow);
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

    depthImage.destroy();
    colorImage.destroy();
}

void Swapchain::recreate(Device& device, VkRenderPass& renderPass,
    GLFWwindow* pWindow)
{
    int width = 0, height = 0;
    glfwGetFramebufferSize(pWindow, &width, &height);
    while (width == 0 || height == 0) {
        if (glfwWindowShouldClose(pWindow)) {
            return;
        }

        glfwGetFramebufferSize(pWindow, &width, &height);
        glfwWaitEvents();
    }

    glfwSetWindowSize(pWindow, width, height);
    vkDeviceWaitIdle(this->device);

    surface.findSurfaceDetails(physicalDevice);
    extent.width = clamp(extent.width,
        surface.details.capabilities.minImageExtent.width,
        surface.details.capabilities.maxImageExtent.width);
    extent.height = clamp(extent.height,
        surface.details.capabilities.minImageExtent.height,
        surface.details.capabilities.maxImageExtent.height);

    destroy();

    create();
    createImageViews();
    createSpecializedImages(device);
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

VkFormat& Swapchain::getDepthFormat()
{
    return depthFormat;
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

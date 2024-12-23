#include "swapchain.h"

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

	Queue& graphicsQueue = device.getGraphicsQueue();
	createSpecializedImages(graphicsQueue);
}

void Swapchain::createSpecializedImages(Queue& graphicsQueue)
{
	depthImage.imageDetails.createImageInfo(
		"", extent.width, extent.height, 1,
		VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, VK_IMAGE_VIEW_TYPE_2D,
		depthFormat, VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
		VK_IMAGE_TILING_OPTIMAL, aspectFlags, samples);
	depthImage.create(device, physicalDevice, commandPool,
		VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, graphicsQueue);

	colorImage.imageDetails.createImageInfo(
		"", extent.width, extent.height, 4, VK_IMAGE_LAYOUT_UNDEFINED,
		VK_IMAGE_VIEW_TYPE_2D, imageFormat, VK_SHADER_STAGE_FRAGMENT_BIT,
		VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_ASPECT_COLOR_BIT, samples);
	colorImage.create(device, physicalDevice, commandPool,
		VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, graphicsQueue);
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

	VkSwapchainCreateInfoKHR swapchainInfo{};
	swapchainInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	swapchainInfo.surface = surface.get();
	swapchainInfo.minImageCount = minImageCount;
	swapchainInfo.imageFormat = imageFormat;
	swapchainInfo.imageColorSpace = colorSpace;
	swapchainInfo.imageExtent = extent;
	swapchainInfo.imageArrayLayers = 1;
	swapchainInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;

	if (queueFamilyIndicies.graphicsFamily != queueFamilyIndicies.presentationFamily
		|| queueFamilyIndicies.presentationFamily != queueFamilyIndicies.transferFamily
		|| queueFamilyIndicies.graphicsFamily != queueFamilyIndicies.computeFamily) {
		swapchainInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		swapchainInfo.queueFamilyIndexCount = 3;
		swapchainInfo.pQueueFamilyIndices = indicies;
	}
	else {
		swapchainInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
	}
	swapchainInfo.preTransform = surface.details.capabilities.currentTransform;
	swapchainInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	swapchainInfo.presentMode = presentMode;
	swapchainInfo.clipped = VK_TRUE;
	swapchainInfo.oldSwapchain = VK_NULL_HANDLE;

	if (vkCreateSwapchainKHR(device, &swapchainInfo, nullptr, &swapchain) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create swap chain.");
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
	size_t imageViewsSize = images.size();
	imageViews.resize(imageViewsSize);
	for (size_t i = 0; i < imageViewsSize; i++) {
		imageViews[i] = createImageView(images[i], imageFormat, VK_IMAGE_ASPECT_COLOR_BIT);
	}
}

VkImageView& Swapchain::createImageView(VkImage& image, VkFormat& format,
	VkImageAspectFlags aspectFlags)
{
	VkImageViewCreateInfo imageViewInfo{};
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
		throw std::runtime_error("Failed to create image view.");
	}
	return imageView;
}

void Swapchain::createFramebuffers(VkRenderPass& renderPass)
{
	framebuffers.resize(imageViews.size());

	VkImageView& depthImageView = depthImage.getView();
	VkImageView& colorImageView = colorImage.getView();
	for (size_t i = 0; i < imageViews.size(); i++) {
		std::vector<VkImageView> attachments = { colorImageView, depthImageView, imageViews[i] };

		VkFramebufferCreateInfo framebufferInfo{};
		framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferInfo.renderPass = renderPass;
		framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
		framebufferInfo.pAttachments = attachments.data();
		framebufferInfo.width = extent.width;
		framebufferInfo.height = extent.height;
		framebufferInfo.layers = 1;

		if (vkCreateFramebuffer(device, &framebufferInfo, nullptr, &framebuffers[i]) != VK_SUCCESS) {
			throw std::runtime_error("Failed to create framebuffer.");
		}
	}
}

uint32_t Swapchain::asquireNextImage(Queue& graphicsQueue, VkRenderPass& renderPass, VkSemaphore& imageAvailable, GLFWwindow* pWindow)
{
	uint32_t imageIndex;
	VkResult result = vkAcquireNextImageKHR(this->device, swapchain, UINT64_MAX, imageAvailable, VK_NULL_HANDLE, &imageIndex);

	if (result == VK_ERROR_OUT_OF_DATE_KHR) {
		recreate(graphicsQueue, renderPass, pWindow);
		return 0;
	}
	else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
		throw std::runtime_error("Failed to acquire swap chain image.");
	}

	return imageIndex;
}

void Swapchain::presentImage(Queue& graphicsQueue, VkRenderPass& renderPass, VkQueue& presentationQueue,
	std::vector<VkSemaphore> signalSemafores,
	GLFWwindow* pWindow)
{
	VkSwapchainKHR swapChains[] = { swapchain };

	VkPresentInfoKHR presentationInfo{};
	presentationInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentationInfo.waitSemaphoreCount = signalSemafores.size();
	presentationInfo.pWaitSemaphores = signalSemafores.data();
	presentationInfo.swapchainCount = 1;
	presentationInfo.pSwapchains = swapChains;
	presentationInfo.pImageIndices = &currentFrame;
	presentationInfo.pResults = nullptr;

	VkResult result = vkQueuePresentKHR(presentationQueue, &presentationInfo);
	if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || framebufferResized) {
		recreate(graphicsQueue, renderPass, pWindow);
		framebufferResized = false;
		currentFrame = 0;
		return;
	}
	else if (result != VK_SUCCESS) {
		throw std::runtime_error("Failed to present swap chain image.");
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

void Swapchain::recreate(Queue& graphicsQueue, VkRenderPass& renderPass,
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
	extent.width = glm::clamp(extent.width,
		surface.details.capabilities.minImageExtent.width,
		surface.details.capabilities.maxImageExtent.width);
	extent.height = glm::clamp(extent.height,
		surface.details.capabilities.minImageExtent.height,
		surface.details.capabilities.maxImageExtent.height);

	destroy();

	create();
	createImageViews();
	createSpecializedImages(graphicsQueue);
	createFramebuffers(renderPass);
}

unsigned char* Swapchain::writeFrameToBuffer(VkCommandBuffer cmds, Queue transferQueue, uint8_t currentFrame)
{
	VkOffset3D offset = VkOffset3D{ 0, 0, 0 };
	VkCommandBuffer cmd = CommandBuffer::beginSingleTimeCommands(device, commandPool);
	Image colorImageCopy;
	Buffer buffer;

	size_t size = extent.height * extent.width * sizeof(char) * 4;
	unsigned char* imageCopy = (unsigned char*)malloc(size);

	buffer.create(device, physicalDevice, size,
		VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_SHARING_MODE_EXCLUSIVE,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

	VkImageMemoryBarrier presentToTransferBarrier{};
	presentToTransferBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	presentToTransferBarrier.oldLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
	presentToTransferBarrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
	presentToTransferBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	presentToTransferBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	presentToTransferBarrier.image = images[currentFrame];
	presentToTransferBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	presentToTransferBarrier.subresourceRange.baseMipLevel = 0;
	presentToTransferBarrier.subresourceRange.levelCount = 1;
	presentToTransferBarrier.subresourceRange.baseArrayLayer = 0;
	presentToTransferBarrier.subresourceRange.layerCount = 1;
	presentToTransferBarrier.srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
	presentToTransferBarrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

	vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, &presentToTransferBarrier);

	VkBufferImageCopy bufferImageCopy;
	bufferImageCopy.bufferOffset = 0;
	bufferImageCopy.bufferRowLength = extent.width;
	bufferImageCopy.bufferImageHeight = extent.height;
	bufferImageCopy.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	bufferImageCopy.imageSubresource.layerCount = 1;
	bufferImageCopy.imageSubresource.baseArrayLayer = 0;
	bufferImageCopy.imageSubresource.mipLevel = 0;
	bufferImageCopy.imageExtent.width = extent.width;
	bufferImageCopy.imageExtent.height = extent.height;
	bufferImageCopy.imageExtent.depth = 1;
	bufferImageCopy.imageOffset = offset;

	vkCmdCopyImageToBuffer(
		cmd,
		images[currentFrame], VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
		buffer.get(), 1, &bufferImageCopy);

	VkImageMemoryBarrier transferToPresentBarrier{};
	transferToPresentBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	transferToPresentBarrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
	transferToPresentBarrier.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
	transferToPresentBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	transferToPresentBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	transferToPresentBarrier.image = images[currentFrame];
	transferToPresentBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	transferToPresentBarrier.subresourceRange.baseMipLevel = 0;
	transferToPresentBarrier.subresourceRange.levelCount = 1;
	transferToPresentBarrier.subresourceRange.baseArrayLayer = 0;
	transferToPresentBarrier.subresourceRange.layerCount = 1;
	transferToPresentBarrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
	transferToPresentBarrier.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;

	vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, &transferToPresentBarrier);

	CommandBuffer::endSingleTimeCommands(device, commandPool, cmd, transferQueue);

	void* mapped;
	vkMapMemory(device, buffer.getDeviceMemory(), 0, size, 0, &mapped);
	memcpy(imageCopy, mapped, size);

	buffer.destroy();
	return imageCopy;
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

std::vector<VkImageView>& Swapchain::getImageViews()
{
	return imageViews;
}

uint32_t& Swapchain::getCurrentFrame()
{
	return currentFrame;
};

std::vector<VkFramebuffer>& Swapchain::getFramebuffers()
{
	return framebuffers;
}

void Swapchain::resizeFramebuffer()
{
	framebufferResized = true;
}

VkDevice& Swapchain::getDevice()
{
	return device;
}

VkPhysicalDevice& Swapchain::getPhysicalDevice()
{
	return physicalDevice;
}

VkCommandPool& Swapchain::getCommandPool()
{
	return commandPool;
}

Image& Swapchain::getColorImage()
{
	return colorImage;
}

Image& Swapchain::getDepthImage()
{
	return depthImage;
}
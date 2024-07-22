// This is a personal academic project. Dear PVS-Studio, please check it.

// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com

#include "engine.h"

using namespace std;

void Engine::run()
{
    try {
        init();
        update();
        cleanup();
    } catch (const exception& e) {
        cerr << e.what() << endl;
    }
}

void Engine::init()
{
    initWindow(Constants::WINDOW_WIDTH, Constants::WINDOW_HEIGHT);

    VkDebugUtilsMessengerCreateInfoEXT debugInfo = debugMessenger.makeDebugMessengerCreateInfo();
    INIT(vulkan.instance, instance.create(debugInfo));
    INIT(vulkan.debugMessenger, debugMessenger.setup(vulkan.instance, debugInfo));

    INIT(vulkan.surface, surface.create(vulkan.instance, pWindow));
    INIT(vulkan.device, device.create(vulkan.instance, surface));
    INIT(vulkan.physicalDevice, device.getPhysicalDevice());
    surface.findSurfaceDetails(vulkan.physicalDevice);
    VkSampleCountFlagBits sampleCount = device.getMaxSampleCount();
    if (sampleCount < vulkan.sampleCount) {
        cout << "Configuration is invalid: device does not support selected"
             << std::to_string(vulkan.sampleCount) << "samples. "
             << "Device supports to " << std::to_string(vulkan.sampleCount)
             << "samples." << endl;

        vulkan.sampleCount = sampleCount;
    }

    inFlightFence.create(vulkan.device, true);
    imageAvailable.create(vulkan.device);
    renderFinished.create(vulkan.device);

    INIT(vulkan.commandPool, commandPool.create(device));
    graphicsCmds.create(vulkan.device, vulkan.commandPool);

    vkQueueWaitIdle(device.getComputeQueue().get());
    computeCmds.create(vulkan.device, vulkan.commandPool);

    swapchain.setDeviceContext(device, surface);
    swapchain.create();
    swapchain.createImageViews();
    const VkExtent2D swapchainExtent = swapchain.getExtent();
    const VkFormat depthFormat = device.findSupportedFormat(depthFormats, VK_IMAGE_TILING_OPTIMAL,
        VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
    const int aspectFlags = device.hasStencilComponent(depthFormat)
        ? VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT
        : VK_IMAGE_ASPECT_DEPTH_BIT;
    depthImage.imageDetails.createImageInfo(
        "", swapchainExtent.width, swapchainExtent.height, 4,
        VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, VK_IMAGE_VIEW_TYPE_2D,
        depthFormat, VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
        VK_IMAGE_TILING_OPTIMAL, aspectFlags, vulkan.sampleCount);
    depthImage.create(device, vulkan.commandPool,
        VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    const VkFormat colorFormat = swapchain.getImageFormat();
    colorImage.imageDetails.createImageInfo(
        "", swapchainExtent.width, swapchainExtent.height, 4,
        VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_VIEW_TYPE_2D, colorFormat,
        VK_SHADER_STAGE_FRAGMENT_BIT, VK_IMAGE_TILING_OPTIMAL,
        VK_IMAGE_ASPECT_COLOR_BIT, vulkan.sampleCount);
    colorImage.create(device, vulkan.commandPool,
        VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    swapchain.createSpecializedImageView(colorImage.get(), colorFormat,
        VK_IMAGE_ASPECT_COLOR_BIT);

    swapchain.createSpecializedImageView(depthImage.get(), depthFormat,
        VK_IMAGE_ASPECT_DEPTH_BIT);

    INIT(vulkan.renderPass, renderPass.create(vulkan.device, colorFormat, depthFormat, vulkan.sampleCount));
    swapchain.createFramebuffers(vulkan.renderPass);

    Queue& transferQueue = device.getTransferQueue();
    quad.constructQuadWithAspectRatio(1920, 1081);
    vertexBuffer.create(vulkan.device, vulkan.physicalDevice,
        vulkan.commandPool, quad.verticies, transferQueue);
    indexBuffer.create(vulkan.device, vulkan.physicalDevice, vulkan.commandPool,
        quad.indicies, transferQueue);

    uniformBuffers.resize(Constants::MAX_FRAMES_IN_FLIGHT);
    for (UniformBuffer& uniformBuffer : uniformBuffers) {
        uniformBuffer.create(vulkan.device, vulkan.physicalDevice, uniformSize);
    }

    paintingTexture.imageDetails.createImageInfo(texturePath.c_str(), 1920, 1081, 4,
        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_IMAGE_VIEW_TYPE_2D,
        Constants::IMAGE_TEXTURE_FORMAT,
        VK_SHADER_STAGE_FRAGMENT_BIT | VK_SHADER_STAGE_COMPUTE_BIT,
        VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_ASPECT_COLOR_BIT,
        VK_SAMPLE_COUNT_1_BIT);
    paintingTexture.create(device, vulkan.commandPool,
        VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    heightMapTexture.imageDetails.createImageInfo("", 1920, 1081, 1,
        VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_VIEW_TYPE_2D,
        Constants::BUMP_TEXTURE_FORMAT,
        VK_SHADER_STAGE_FRAGMENT_BIT | VK_SHADER_STAGE_COMPUTE_BIT,
        VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_ASPECT_COLOR_BIT,
        VK_SAMPLE_COUNT_1_BIT);
    heightMapTexture.create(device, vulkan.commandPool,
        VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_STORAGE_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    textureSampler.create(vulkan.device, vulkan.physicalDevice);

    descriptor.create(vulkan.device, uniformBuffers, paintingTexture,
        heightMapTexture, textureSampler);

    pipeline.create(vulkan.device, vulkan.renderPass, descriptor.getSetLayout(), swapchainExtent, vulkan.sampleCount);

    gui.init(vulkan.instance, device, vulkan.commandPool, renderPass, swapchain, descriptor.getPool(), pWindow);
}

void Engine::update()
{
    const vector<VkPipelineStageFlags> waitStages = {
        VK_PIPELINE_STAGE_VERTEX_INPUT_BIT,
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT
    };
    const std::vector<VkPipelineStageFlags> computeWaitStages = {
        VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT
    };
    const std::vector<VkSemaphore> computeWaitSemaphores {};
    const std::vector<VkSemaphore> computeSignalSemaphores {};
    const VkExtent2D& extent = swapchain.getExtent();

    std::vector<VkFramebuffer>& framebuffers = swapchain.getFramebuffers();
    Queue& presentationQueue = device.getPresentationQueue();
    Queue& computeQueue = device.getComputeQueue();
    Image::Details imageDetails = heightMapTexture.getDetails();

    forwardRenderAction.setContext(pipeline, extent, 0);

    // run compute only once for a frame
    for (int currentFrame = 0; currentFrame < Constants::MAX_FRAMES_IN_FLIGHT; currentFrame++) {
        VkCommandBuffer& cmdCompute = computeCmds.get(currentFrame);

        computeCmds.begin(currentFrame);
        pipeline.bind(cmdCompute, descriptor.getSet(currentFrame));
        vkCmdDispatch(cmdCompute, imageDetails.width / 16,
            imageDetails.height / 16, 1);
        computeCmds.end(currentFrame);

        computeQueue.submit(cmdCompute, inFlightFence, computeWaitSemaphores,
            computeSignalSemaphores, computeWaitStages,
            currentFrame);
    }

    while (!glfwWindowShouldClose(pWindow)) {
        glfwPollEvents();

        const uint32_t& currentFrame = swapchain.getCurrentFrame();
        VkCommandBuffer& cmdGraphics = graphicsCmds.get(currentFrame);
        VkSemaphore& currentImageAvailable = imageAvailable.get(currentFrame);
        VkSemaphore& currentRenderFinished = renderFinished.get(currentFrame);
        const std::vector<VkSemaphore> waitSemaphores { currentImageAvailable };
        const std::vector<VkSemaphore> signalSemaphores { currentRenderFinished };

        {
            ObjectParams objectParams = gui.getObjectParams();
            CameraParams cameraParams = gui.getCameraParams();
            quad.uniform.move(objectParams);
            quad.uniform.rotate(objectParams);
            quad.uniform.scale(objectParams);
            quad.uniform.transform(gui.getAnimatedObjectParams(),
                gui.getAnimationParams());
            quad.uniform.cameraView(cameraParams, extent);
            uniformBuffers[currentFrame].update(quad.uniform);
        }

        inFlightFence.wait(currentFrame);
        inFlightFence.reset(currentFrame);

        swapchain.asquireNextImage(vulkan.renderPass, currentImageAvailable, pWindow);

        graphicsCmds.begin(currentFrame);

        gui.draw(pipeline.getPipelineHistorySize());

        if (pipeline.recreateifShadersChanged()) {
            gui.selectPipelineindex(pipeline.getPipelineHistorySize() - 1);
        }

        forwardRenderAction.setContext(pipeline, extent,
            gui.getSelectedPipelineIndex());
        forwardRenderAction.beginRenderPass(cmdGraphics, vulkan.renderPass,
            framebuffers, currentFrame);
        forwardRenderAction.recordCommandBuffer(
            cmdGraphics, descriptor.getSet(currentFrame), vertexBuffer,
            indexBuffer, quad);
        gui.renderDrawData(cmdGraphics);
        forwardRenderAction.endRenderPass(cmdGraphics);
        graphicsCmds.end(currentFrame);

        presentationQueue.submit(cmdGraphics, inFlightFence, waitSemaphores,
            signalSemaphores, waitStages, currentFrame);
        swapchain.presentImage(vulkan.renderPass, presentationQueue.get(),
            signalSemaphores, pWindow);
    }

    vkDeviceWaitIdle(vulkan.device);
}

void Engine::cleanup()
{
    gui.destroy();

    inFlightFence.destroy();
    imageAvailable.destroy();
    renderFinished.destroy();

    pipeline.destroy();
    descriptor.destroy(vulkan.device);
    textureSampler.destroy();
    paintingTexture.destroy();
    heightMapTexture.destroy();
    depthImage.destroy();
    colorImage.destroy();

    for (UniformBuffer& uniformBuffer : uniformBuffers) {
        uniformBuffer.destroy();
    }

    vertexBuffer.destroy();
    indexBuffer.destroy();
    renderPass.destroy();
    commandPool.destroy();
    swapchain.destroy();

    device.destroy();
    surface.destory();

    debugMessenger.destroyIfLayersEnabled();
    instance.destroy();

    glfwDestroyWindow(pWindow);

    glfwTerminate();
}

void Engine::initWindow(const uint16_t width, const uint16_t height)
{
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    pWindow = glfwCreateWindow(width, height, "Living Paintings", nullptr, nullptr);
    glfwSetWindowUserPointer(pWindow, this);
    glfwMakeContextCurrent(pWindow);
    glfwSetFramebufferSizeCallback(
        pWindow, [](GLFWwindow* window, int width, int height) {
            Engine* pEngine = reinterpret_cast<Engine*>(glfwGetWindowUserPointer(window));
            pEngine->swapchain.resizeFramebuffer();
        });
}

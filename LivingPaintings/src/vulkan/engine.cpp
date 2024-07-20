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

    inFlightFence.create(vulkan.device, true);
    imageAvailable.create(vulkan.device);
    renderFinished.create(vulkan.device);
    computeIsReady.create(vulkan.device);
    graphicsIsReady.create(vulkan.device);

    swapchain.setDeviceContext(device, surface);
    swapchain.create();
    swapchain.createImageViews();

    INIT(vulkan.renderPass, renderPass.create(vulkan.device, swapchain.getImageFormat()));
    swapchain.createFramebuffers(vulkan.renderPass);

    INIT(vulkan.commandPool, commandPool.create(device));
    graphicsCmds.create(vulkan.device, vulkan.commandPool);

    vkQueueWaitIdle(device.getComputeQueue().get());
    computeCmds.create(vulkan.device, vulkan.commandPool);

    Queue& transferQueue = device.getTransferQueue();
    quad.constructQuadWithAspectRatio(1920, 1081);
    vertexBuffer.create(vulkan.device, vulkan.physicalDevice,
        vulkan.commandPool, quad.verticies, transferQueue);
    indexBuffer.create(vulkan.device, vulkan.physicalDevice, vulkan.commandPool,
        quad.indicies, transferQueue);

    uniformBuffers.resize(Constants::MAX_FRAMES_IN_FLIGHT);
    const VkDeviceSize uniformSize = sizeof(Data::GraphicsObject::UniformBufferObject);
    for (UniformBuffer& uniformBuffer : uniformBuffers) {
        uniformBuffer.create(vulkan.device, vulkan.physicalDevice, uniformSize);
    }

    // TEXTURE_FILE_PATH variable is retrieved from Cmake with macros in file config.hpp.in
    const string texturePath = RETRIEVE_STRING(TEXTURE_FILE_PATH);
    paintingTexture.imageDetails.createImageInfo(
        texturePath.c_str(), 1920, 1081, 4,
        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_IMAGE_VIEW_TYPE_2D,
        Constants::IMAGE_TEXTURE_FORMAT,
        VK_SHADER_STAGE_FRAGMENT_BIT | VK_SHADER_STAGE_COMPUTE_BIT,
        VK_IMAGE_TILING_OPTIMAL);
    paintingTexture.create(device, vulkan.commandPool,
        VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    heightMapTexture.imageDetails.createImageInfo(
        "", 1920, 1081, 1, VK_IMAGE_LAYOUT_GENERAL,
        VK_IMAGE_VIEW_TYPE_2D, Constants::BUMP_TEXTURE_FORMAT,
        VK_SHADER_STAGE_FRAGMENT_BIT | VK_SHADER_STAGE_COMPUTE_BIT,
        VK_IMAGE_TILING_OPTIMAL);
    heightMapTexture.create(device, vulkan.commandPool,
        VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_STORAGE_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    textureSampler.create(vulkan.device, vulkan.physicalDevice);

    descriptor.create(vulkan.device, uniformBuffers, paintingTexture,
        heightMapTexture, textureSampler);

    pipeline.create(vulkan.device, vulkan.renderPass, descriptor.getSetLayout(), swapchain.getExtent());

    gui.init(vulkan.instance, device, vulkan.commandPool, vulkan.renderPass, swapchain, descriptor.getPool(), pWindow);
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
    const VkExtent2D& extent = swapchain.getExtent();
    std::vector<VkFramebuffer>& framebuffers = swapchain.getFramebuffers();
    Queue& presentationQueue = device.getPresentationQueue();
    Queue& computeQueue = device.getComputeQueue();
    Image::Details imageDetails = heightMapTexture.getDetails();
    std::vector<VkSemaphore>& computeReady = computeIsReady.get();
    std::vector<VkSemaphore>& graphicsReady = graphicsIsReady.get();

    forwardRenderAction.setContext(pipeline, extent, 0);

    while (!glfwWindowShouldClose(pWindow)) {
        glfwPollEvents();

        const uint32_t& currentFrame = swapchain.getCurrentFrame();
        VkCommandBuffer& cmdGraphics = graphicsCmds.get(currentFrame);
        VkCommandBuffer& cmdCompute = computeCmds.get(currentFrame);
        VkSemaphore& currentImageAvailable = imageAvailable.get(currentFrame);
        VkSemaphore& currentRenderFinished = renderFinished.get(currentFrame);

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

        // signal semaphore that graphics is ready
        device.getComputeQueue().signal(graphicsReady[currentFrame]);

        inFlightFence.wait(currentFrame);
        inFlightFence.reset(currentFrame);

        swapchain.asquireNextImage(vulkan.renderPass, currentImageAvailable, pWindow);

        {
            computeCmds.begin(currentFrame);
            pipeline.bind(cmdCompute, descriptor.getSet(currentFrame));
            vkCmdDispatch(cmdCompute, imageDetails.width, imageDetails.height, 1);
            computeCmds.end(currentFrame);

            const std::vector<VkSemaphore> computeWaitSemaphores { graphicsReady[currentFrame] };
            const std::vector<VkSemaphore> computeSignalSemaphores { computeReady[currentFrame] };
            computeQueue.submit(cmdCompute, inFlightFence,
                computeWaitSemaphores, computeSignalSemaphores,
                computeWaitStages, currentFrame);
        }

        inFlightFence.wait(currentFrame);
        inFlightFence.reset(currentFrame);

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

        const std::vector<VkSemaphore> waitSemaphores { computeReady[currentFrame], currentImageAvailable };
        const std::vector<VkSemaphore> signalSemaphores { graphicsReady[currentFrame], currentRenderFinished };
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
    computeIsReady.destroy();
    graphicsIsReady.destroy();

    pipeline.destroy();
    descriptor.destroy(vulkan.device);
    textureSampler.destroy();
    paintingTexture.destroy();
    heightMapTexture.destroy();

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

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

    quad.constructQuad();

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

    swapchain.setDeviceContext(device, surface);
    swapchain.create();
    swapchain.createImageViews();

    INIT(vulkan.renderPass, renderPass.create(vulkan.device, swapchain.getImageFormat()));
    swapchain.createFramebuffers(vulkan.renderPass);

    INIT(vulkan.commandPool, commandPool.create(device));
    commandBuffer.create(vulkan.device, vulkan.commandPool);

    Queue& transferQueue = device.getTransferQueue();
    vertexBuffer.create(vulkan.device, vulkan.physicalDevice,
        vulkan.commandPool, quad.verticies, transferQueue);
    indexBuffer.create(vulkan.device, vulkan.physicalDevice, vulkan.commandPool,
        quad.indicies, transferQueue);

    uniformBuffers.resize(Constants::MAX_FRAMES_IN_FLIGHT);
    const VkDeviceSize size = sizeof(Data::GraphicsObject::UniformBufferObject);
    for (UniformBuffer& uniformBuffer : uniformBuffers) {
        uniformBuffer.create(vulkan.device, vulkan.physicalDevice, size);
    }

    // TEXTURE_FILE_PATH variable is retrieved from Cmake with macros in file config.hpp.in
    const string texturePath = RETRIEVE_STRING(TEXTURE_FILE_PATH);
    textureImage.imageDetails.createImageInfo(texturePath.c_str(), 1920, 1081,
        VK_IMAGE_VIEW_TYPE_2D, Constants::IMAGE_FORMAT,
        VK_IMAGE_TILING_OPTIMAL);
    textureImage.create(device, vulkan.commandPool,
        VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    textureSampler.create(vulkan.device, vulkan.physicalDevice);

    descriptor.create(vulkan.device, uniformBuffers, textureImage, textureSampler);

    graphicsPipeline.create(vulkan.device, vulkan.renderPass, descriptor.getSetLayout(), swapchain.getExtent());

    gui.init(vulkan.instance, device, vulkan.commandPool, vulkan.renderPass, swapchain, descriptor.getPool(), pWindow);
}

void Engine::update()
{
    // TODO recreate renderPass because imageFormat can also change when switching surface.

    const VkExtent2D& extent = swapchain.getExtent();
    std::vector<VkFramebuffer>& framebuffers = swapchain.getFramebuffers();
    Queue& presentationQueue = device.getPresentationQueue();
    const vector<VkPipelineStageFlags> waitStages = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };

    forwardRenderAction.setContext(graphicsPipeline, extent, 0);

    while (!glfwWindowShouldClose(pWindow)) {
        glfwPollEvents();

        const uint32_t& currentFrame = swapchain.getCurrentFrame();
        VkCommandBuffer& cmd = commandBuffer.get(currentFrame);
        VkSemaphore& currentImageAvailable = imageAvailable.get(currentFrame);
        const std::vector<VkSemaphore> waitSemaphores = vector { currentImageAvailable };
        const std::vector<VkSemaphore> signalSemaphores = vector { renderFinished.get(currentFrame) };

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

        commandBuffer.begin(currentFrame);
        gui.draw(graphicsPipeline.getPipelineHistorySize());

        if (graphicsPipeline.recreateifShadersChanged()) {
            gui.selectPipelineindex(graphicsPipeline.getPipelineHistorySize() - 1);
        }
        forwardRenderAction.setContext(graphicsPipeline, extent, gui.getSelectedPipelineIndex());
        forwardRenderAction.beginRenderPass(cmd, vulkan.renderPass, framebuffers, currentFrame);
        forwardRenderAction.recordCommandBuffer(cmd, descriptor.getSet(currentFrame),
            vertexBuffer, indexBuffer, quad);
        gui.renderDrawData(cmd);
        forwardRenderAction.endRenderPass(cmd);
        commandBuffer.end(currentFrame);

        presentationQueue.submit(cmd, inFlightFence, waitSemaphores,
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

    graphicsPipeline.destroy();
    descriptor.destroy(vulkan.device);
    textureSampler.destroy();
    textureImage.destroy();

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

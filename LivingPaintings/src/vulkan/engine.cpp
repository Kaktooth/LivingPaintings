// This is a personal academic project. Dear PVS-Studio, please check it.

// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com

#include "engine.h"
#include "../config.hpp"

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

    auto debugInfo = Engine::debugMessenger.makeDebugMessengerCreateInfo();
    Engine::instance.create(debugInfo);

    auto& instance = Engine::instance.get();
    Engine::debugMessenger.setup(instance, debugInfo);

    Engine::surface.create(instance, pWindow);

    Engine::physicalDevice.select(instance, surface, queueFamily);

    auto physicalDevice = Engine::physicalDevice.get();
    auto queueFamilyIndicies = queueFamily.findQueueFamilies(physicalDevice, surface.get());
    auto deviceFeatures = Engine::physicalDevice.selectedDeviceFeatures();
    Engine::device.create(physicalDevice, queueFamilyIndicies, deviceFeatures);

    auto device = Engine::device.get();

    inFlightFence.create(device, true);
    imageAvailable.create(device);
    renderFinished.create(device);

    Engine::swapChain.create(device, physicalDevice, surface, queueFamilyIndicies);
    Engine::swapChain.createImageViews(device);

    Engine::renderPass.create(device, swapChain.getImageFormat());

    auto renderPass = Engine::renderPass.get();
    Engine::swapChain.createFramebuffers(device, renderPass);

    Engine::commandPool.create(device, queueFamilyIndicies);

    Engine::commandBuffer.create(device, commandPool.get());

    auto extent = swapChain.getExtent();
    Engine::vertexBuffer.create(device, physicalDevice, vertexData.verticies, commandPool.get(), commandBuffer, Engine::device.getTransferQueue());
    Engine::indexBuffer.create(device, physicalDevice, vertexData.indicies, commandPool.get(), commandBuffer, Engine::device.getTransferQueue());

    uniformBuffers.resize(Constants::MAX_FRAMES_IN_FLIGHT);
    VkDeviceSize size = sizeof(VertexData::UniformBufferObject);
    for (auto& uniformBuffer : uniformBuffers) {
        uniformBuffer.create(device, physicalDevice, size);
    }

    // TEXTURE_FILE_PATH variable is retrieved from Cmake with macros in file config.hpp.in
    string texturePath = RETRIEVE_STRING(TEXTURE_FILE_PATH);
    Engine::textureImage.create(device, physicalDevice, commandPool.get(), Engine::device.getGraphicsQueue(), texturePath.c_str(), 1920, 1081, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    Engine::textureSampler.create(device, physicalDevice);

    Engine::descriptor.create(device, uniformBuffers, textureImage, textureSampler);

    auto commandBuffer = Engine::commandBuffer.get();
    Engine::graphicsPipeline.create(device, extent, renderPass, vertexData, descriptor.getSetLayout());

    gui.init(instance, Engine::device, queueFamilyIndicies, Engine::renderPass, swapChain, commandPool.get(), descriptor.getPool(), Engine::device.getGraphicsQueue(), pWindow);
}

void Engine::update()
{
    // TODO refactor each class method parameters, instead use constructors.
    // TODO recreate renderPass because imageFormat can also change when switching surface.

    auto& device = Engine::device.get();
    const auto& physicalDevice = Engine::physicalDevice.get();
    auto& surface = Engine::surface.get();
    auto& renderPass = Engine::renderPass.get();
    auto& framebuffers = Engine::swapChain.getFramebuffers();
    auto& graphicsPipeline = Engine::graphicsPipeline.get();
    auto& pipelineLayout = Engine::graphicsPipeline.getLayout();
    auto& presentationQueue = Engine::device.getPresentationQueue().get();

    auto queueFamilyIndicies = queueFamily.findQueueFamilies(physicalDevice, surface);

    while (!glfwWindowShouldClose(pWindow)) {
        glfwPollEvents();

        auto& currentFrame = swapChain.getCurrentFrame();

        inFlightFence.wait(currentFrame);

        {
            auto objectParams = gui.getObjectParams();
            auto cameraParams = gui.getCameraParams();
            uniformObject.move(objectParams);
            uniformObject.rotate(objectParams);
            uniformObject.scale(objectParams);
            uniformObject.transform(gui.getAnimatedObjectParams(), gui.getAnimationParams());
            uniformObject.cameraView(cameraParams, swapChain.getExtent());
            uniformBuffers[currentFrame].update(uniformObject);
        }

        swapChain.asquireNextImage(device, physicalDevice, renderPass, Engine::surface, queueFamilyIndicies, imageAvailable, pWindow);

        commandBuffer.begin(currentFrame);
        gui.draw();
        forwardRenderAction.beginRenderPass(commandBuffer, graphicsPipeline, framebuffers, renderPass, swapChain);
        forwardRenderAction.recordCommandBuffer(commandBuffer, swapChain, pipelineLayout, descriptor.getSet(currentFrame), vertexBuffer, indexBuffer, vertexData);
        gui.renderDrawData(commandBuffer.get(currentFrame));
        forwardRenderAction.endRenderPass(commandBuffer, swapChain);
        commandBuffer.end(currentFrame);

        inFlightFence.reset(currentFrame);

        // TODO try to sync for every command buffer
        auto waitSemaphores = vector { imageAvailable.get(currentFrame) };
        auto signalSemaphores = vector { renderFinished.get(currentFrame) };

        vector<VkPipelineStageFlags> waitStages = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
        Engine::device.getPresentationQueue()
            .submit(commandBuffer.get(currentFrame), inFlightFence, waitSemaphores, signalSemaphores, waitStages, currentFrame);
        swapChain.presentImage(device, physicalDevice, renderPass, Engine::surface, queueFamilyIndicies, presentationQueue, signalSemaphores, pWindow);
    }

    vkDeviceWaitIdle(device);
}

void Engine::cleanup()
{
    gui.destroy();

    inFlightFence.destroy();
    imageAvailable.destroy();
    renderFinished.destroy();

    {
        auto device = Engine::device.get();

        graphicsPipeline.destroy(device);
        descriptor.destroy(device);
        textureSampler.destroy(device);
        textureImage.destroy(device);

        for (auto& uniformBuffer : uniformBuffers) {
            uniformBuffer.destroy(device);
        }

        vertexBuffer.destroy(device);
        indexBuffer.destroy(device);
        renderPass.destroy(device, graphicsPipeline.getLayout());
        commandPool.destroy(device);
        swapChain.destroy(device);
    }
    Engine::device.destroy();

    {
        auto instance = Engine::instance.get();
        surface.destory(instance);
        debugMessenger.destroyIfLayersEnabled(instance);
    }
    Engine::instance.destroy();

    glfwDestroyWindow(pWindow);

    glfwTerminate();
}

static void framebufferResizeCallback(GLFWwindow* window, int width, int height)
{
    auto pEngine = reinterpret_cast<Engine*>(glfwGetWindowUserPointer(window));
    pEngine->swapChain.resizeFramebuffer();
}

void Engine::initWindow(int width, int height)
{
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    pWindow = glfwCreateWindow(width, height, "Window", nullptr, nullptr);
    glfwSetWindowUserPointer(pWindow, this);
    glfwMakeContextCurrent(pWindow);
    glfwSetFramebufferSizeCallback(pWindow, framebufferResizeCallback);
}

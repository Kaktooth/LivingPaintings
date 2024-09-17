#include "engine.h"

using Data::AlignmentProperties;
using Data::RuntimeProperties;

using Constants::WINDOW_WIDTH;
using Constants::WINDOW_HEIGHT;
using Constants::IMAGE_TEXTURE_FORMAT;
using Constants::BUMP_TEXTURE_FORMAT;
using Constants::EFFECT_MASK_TEXTURE_FORMAT;
using Constants::MAX_FRAMES_IN_FLIGHT;

using std::chrono::steady_clock;
using std::chrono::seconds;
using std::chrono::duration;

void Engine::run()
{
    try {
        init();
        update();
        cleanup();
    } catch (const std::exception& e) {
        std::cerr << e.what() << '\n';
    }
}

void Engine::init()
{
    initWindow(WINDOW_WIDTH, WINDOW_HEIGHT);

    VkDebugUtilsMessengerCreateInfoEXT debugInfo = debugMessenger.makeDebugMessengerCreateInfo();
    INIT(vulkan.instance, instance.create(debugInfo));
    INIT(vulkan.debugMessenger, debugMessenger.setup(vulkan.instance, debugInfo));

    INIT(vulkan.surface, surface.create(vulkan.instance, pWindow));
    INIT(vulkan.device, device.create(vulkan.instance, surface));
    INIT(vulkan.physicalDevice, device.getPhysicalDevice());
    surface.findSurfaceDetails(vulkan.physicalDevice);
    VkSampleCountFlagBits sampleCount = device.getMaxSampleCount();
    if (sampleCount < vulkan.sampleCount) {
        std::cout << "Configuration is invalid: device does not support selected"
                  << std::to_string(vulkan.sampleCount) << "samples. "
                  << "Device supports to " << std::to_string(vulkan.sampleCount)
                  << "samples." << '\n';

        vulkan.sampleCount = sampleCount;
    }

    inFlightFence.create(vulkan.device, true);
    imageAvailable.create(vulkan.device);
    renderFinished.create(vulkan.device);

    const QueueFamily::Indices familyQueueIndicies = device.getQueueFamily().indicies;
    INIT(vulkan.commandPool, commandPool.create(device, familyQueueIndicies.graphicsFamily.value()));
    graphicsCmds.create(vulkan.device, vulkan.commandPool);

    vkQueueWaitIdle(device.getComputeQueue().get());
    computeCmds.create(vulkan.device, vulkan.commandPool);

    swapchain.setContext(device, surface, vulkan.commandPool, vulkan.sampleCount, depthFormatCandidates);
    swapchain.create();
    swapchain.createImageViews();

    const VkFormat colorFormat = swapchain.getImageFormat();
    const VkFormat depthFormat = swapchain.getDepthFormat();
    INIT(vulkan.renderPass, renderPass.create(vulkan.device, colorFormat, depthFormat, vulkan.sampleCount));
    swapchain.createFramebuffers(vulkan.renderPass);

    Queue& transferQueue = device.getTransferQueue();

    size_t minUniformAlignment = device.getProperties().limits.minUniformBufferOffsetAlignment;
    Data::setUniformDynamicAlignments(minUniformAlignment);
    Data::GraphicsObject::instanceUniform.allocateInstances();

    graphicsObjects.resize(1);
    vertexBuffers.resize(1);
    indexBuffers.resize(1);
    graphicsObjects[0].constructQuadWithAspectRatio(TEX_WIDTH, TEX_HEIGHT, 0.0f);
    vertexBuffers[0].create(vulkan.device, vulkan.physicalDevice, vulkan.commandPool,
        graphicsObjects[0].vertices, transferQueue);
    indexBuffers[0].create(vulkan.device, vulkan.physicalDevice, vulkan.commandPool,
        graphicsObjects[0].indices, transferQueue);

    instanceUniformBuffers.resize(MAX_FRAMES_IN_FLIGHT);
    for (UniformBuffer& uniformBuffer : instanceUniformBuffers) {
        uniformBuffer.create(vulkan.device, vulkan.physicalDevice, RuntimeProperties::uboMemorySize,
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
    }

    viewUniformBuffers.resize(MAX_FRAMES_IN_FLIGHT);
    for (UniformBuffer& uniformBuffer : viewUniformBuffers) {
        uniformBuffer.create(vulkan.device, vulkan.physicalDevice, sizeof(Data::GraphicsObject::ViewUbo),
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    }

    time.create(vulkan.device, vulkan.physicalDevice, sizeof(int64_t),
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    effectsParams.create(vulkan.device, vulkan.physicalDevice, sizeof(EffectParams),
                         VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    segmentationSystem.init(device, vulkan.commandPool, pWindow,
        TEXTURE_PATH, TEX_WIDTH, TEX_HEIGHT, controls.getMouseControls());

    controls.fillInMouseControlInfo(glm::uvec2(WINDOW_WIDTH, WINDOW_HEIGHT),
        0.1f, pWindow);
    mouseControl.create(vulkan.device, vulkan.physicalDevice, mouseUniformSize,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    paintingTexture.imageDetails.createImageInfo(
        TEXTURE_PATH.c_str(), TEX_WIDTH, TEX_HEIGHT, 4,
        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_IMAGE_VIEW_TYPE_2D,
        IMAGE_TEXTURE_FORMAT,
        VK_SHADER_STAGE_FRAGMENT_BIT | VK_SHADER_STAGE_COMPUTE_BIT,
        VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_ASPECT_COLOR_BIT,
        VK_SAMPLE_COUNT_1_BIT);
    paintingTexture.create(device, vulkan.commandPool,
        VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, transferQueue);

    heightMapTexture.imageDetails.createImageInfo(
        "", TEX_WIDTH, TEX_HEIGHT, 1,
        VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_VIEW_TYPE_2D,
        BUMP_TEXTURE_FORMAT,
        VK_SHADER_STAGE_FRAGMENT_BIT | VK_SHADER_STAGE_COMPUTE_BIT,
        VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_ASPECT_COLOR_BIT,
        VK_SAMPLE_COUNT_1_BIT);
    heightMapTexture.create(device, vulkan.commandPool,
        VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_STORAGE_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, transferQueue);

    textureSampler.create(vulkan.device, vulkan.physicalDevice);

    descriptor.create(vulkan.device, instanceUniformBuffers,
                      viewUniformBuffers, paintingTexture,
                      heightMapTexture, textureSampler, mouseControl,
                      segmentationSystem.getSelectedPosMasks(),
                      time, effectsParams);

    pipeline.create(vulkan.device, vulkan.renderPass, descriptor.getSetLayout(),
        swapchain.getExtent(), vulkan.sampleCount);

    gui.init(vulkan.instance, device, vulkan.commandPool, renderPass, swapchain, descriptor.getPool(), pWindow);
}

void Engine::update()
{
    static steady_clock::time_point lastTimePoint = steady_clock::now();

    const std::vector<VkPipelineStageFlags> waitStages = {
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
    Queue& transferQueue = device.getTransferQueue();
    Queue& computeQueue = device.getComputeQueue();
    Image::Details imageDetails = heightMapTexture.getDetails();

    forwardRenderAction.setContext(pipeline, extent, 0);

    // run compute only once for a frame
    for (int currentFrame = 0; currentFrame < Constants::MAX_FRAMES_IN_FLIGHT; currentFrame++) {
        VkCommandBuffer& cmdCompute = computeCmds.get(currentFrame);

        computeCmds.begin(currentFrame);
        pipeline.bind(cmdCompute, descriptor.getSet(currentFrame));
        vkCmdDispatch(cmdCompute, imageDetails.width,
            imageDetails.height, 1);
        computeCmds.end(currentFrame);

        computeQueue.submit(cmdCompute, inFlightFence, computeWaitSemaphores,
            computeSignalSemaphores, computeWaitStages, currentFrame);
    }

    while (!glfwWindowShouldClose(pWindow)) {
        glfwPollEvents();

        glm::dvec2 cursorPos {};
        int16_t maskIndex = gui.getMouseControlParams().maskIndex;

        steady_clock::time_point currentTime = steady_clock::now();
        float timeDuration_s = duration<float, seconds::period>(currentTime.time_since_epoch()).count();
        Controls::MouseControl& mouseControls = controls.getMouseControls();
        EffectParams& effectParams = gui.getEffectParams();
        glfwGetCursorPos(pWindow, &cursorPos.x, &cursorPos.y);
        controls.updateMousePos(cursorPos);
        controls.updateMaskIndex(maskIndex);
        mouseControl.update(mouseControls);
        time.update(timeDuration_s);
        effectsParams.update(effectParams);

        segmentationSystem.updatePositionMasks(device, vulkan.commandPool, transferQueue);

        pipeline.updateExtent(swapchain.getExtent());

        const uint32_t& currentFrame = swapchain.getCurrentFrame();
        VkCommandBuffer& cmdGraphics = graphicsCmds.get(currentFrame);
        VkSemaphore& currentImageAvailable = imageAvailable.get(currentFrame);
        VkSemaphore& currentRenderFinished = renderFinished.get(currentFrame);
        const std::vector<VkSemaphore> waitSemaphores { currentImageAvailable };
        const std::vector<VkSemaphore> signalSemaphores { currentRenderFinished };

        if (gui.drawParams.clearSelectedMask) {
            segmentationSystem.removeAllMaskPositions();
            gui.drawParams.clearSelectedMask = false;
        }

        if (gui.drawParams.constructSelectedObject) {
            Data::GraphicsObject constructedObject;
            const unsigned char* mask = segmentationSystem.getSelectedPositionsMask(0);
            segmentationSystem.removeAllMaskPositions(0);
            ObjectConstructionParams objectConstructionParams = gui.getObjectConstructionParams();

            constructedObject.constructMeshFromTexture(TEX_WIDTH, TEX_HEIGHT, 0.0f, mask,
                objectConstructionParams.alphaPercentage);

            if (constructedObject.indices.size() != 0) {
                size_t lastSize = graphicsObjects.size();
                vertexBuffers.resize(lastSize + 1);
                indexBuffers.resize(lastSize + 1);
                vertexBuffers[lastSize].create(
                    vulkan.device, vulkan.physicalDevice, vulkan.commandPool,
                    constructedObject.vertices, transferQueue);
                indexBuffers[lastSize].create(
                    vulkan.device, vulkan.physicalDevice, vulkan.commandPool,
                    constructedObject.indices, transferQueue);
                graphicsObjects.push_back(constructedObject);
            }
            gui.drawParams.constructSelectedObject = false;
        }

        ObjectParams objectParams = gui.getObjectParams();
        CameraParams cameraParams = gui.getCameraParams();

        Data::GraphicsObject::instanceUniform.move(objectParams);
        Data::GraphicsObject::instanceUniform.rotate(objectParams);
        Data::GraphicsObject::instanceUniform.scale(objectParams);
        Data::GraphicsObject::instanceUniform.transform(
            gui.getAnimatedObjectParams(),
            gui.getAnimationParams());

        if (graphicsObjects.size() > 1) {
            for (size_t i = 1; i < graphicsObjects.size(); i++) {
                ObjectParams objParams {};
                objParams.index = i;
                objParams.scale[0] = 1.0f;
                objParams.scale[1] = 1.0f;
                objParams.scale[2] = 1.0f;

                Data::GraphicsObject::instanceUniform.move(objParams);
                Data::GraphicsObject::instanceUniform.rotate(objParams);
                Data::GraphicsObject::instanceUniform.scale(objParams);
            }
        }
        instanceUniformBuffers[currentFrame].update(Data::GraphicsObject::instanceUniform);

        VkMappedMemoryRange mappedMemoryRange {};
        mappedMemoryRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
        mappedMemoryRange.memory = instanceUniformBuffers[currentFrame].getDeviceMemory();
        mappedMemoryRange.size = instanceUniformBuffers[currentFrame].getMemorySize();
        vkFlushMappedMemoryRanges(vulkan.device, 1, &mappedMemoryRange);

        Data::GraphicsObject::viewUniform.cameraView(cameraParams, extent);
        viewUniformBuffers[currentFrame].update(Data::GraphicsObject::viewUniform);

        inFlightFence.wait(currentFrame);
        inFlightFence.reset(currentFrame);

        swapchain.asquireNextImage(device, vulkan.renderPass,
            currentImageAvailable, pWindow);

        graphicsCmds.begin(currentFrame);

        gui.drawParams.pipelineHistorySize = pipeline.getPipelineHistorySize();
        gui.drawParams.imageLoaded = segmentationSystem.isImageLoaded();
        gui.draw();

        if (pipeline.recreateifShadersChanged()) {
            gui.selectPipelineindex(pipeline.getPipelineHistorySize() - 1);
            VkCommandBuffer& cmdCompute = computeCmds.get(currentFrame);

            computeCmds.begin(currentFrame);
            pipeline.bind(cmdCompute, descriptor.getSet(currentFrame));
            vkCmdDispatch(cmdCompute, imageDetails.width, imageDetails.height, 1);
            computeCmds.end(currentFrame);

            computeQueue.submit(cmdCompute, inFlightFence,
                computeWaitSemaphores, computeSignalSemaphores,
                computeWaitStages, currentFrame);

            inFlightFence.wait(currentFrame);
            inFlightFence.reset(currentFrame);
        }

        forwardRenderAction.setContext(pipeline, extent,
            gui.getSelectedPipelineIndex());
        forwardRenderAction.beginRenderPass(cmdGraphics, vulkan.renderPass,
            framebuffers, currentFrame);
        for (size_t i = 0; i < graphicsObjects.size(); i++) {
            forwardRenderAction.recordCommandBuffer(cmdGraphics, descriptor.getSet(currentFrame),
                vertexBuffers[i], indexBuffers[i], graphicsObjects[i]);
        }
        gui.renderDrawData(cmdGraphics);
        forwardRenderAction.endRenderPass(cmdGraphics);
        graphicsCmds.end(currentFrame);

        presentationQueue.submit(cmdGraphics, inFlightFence, waitSemaphores,
            signalSemaphores, waitStages, currentFrame);
        swapchain.presentImage(device, vulkan.renderPass, presentationQueue.get(),
            signalSemaphores, pWindow);
    }

    vkDeviceWaitIdle(vulkan.device);
}

void Engine::cleanup()
{
    segmentationSystem.destroy();

    gui.destroy();

    inFlightFence.destroy();
    imageAvailable.destroy();
    renderFinished.destroy();

    pipeline.destroy();
    descriptor.destroy(vulkan.device);
    textureSampler.destroy();
    paintingTexture.destroy();
    heightMapTexture.destroy();

    Data::GraphicsObject::instanceUniform.destroy();
    for (size_t i = 0; i < graphicsObjects.size(); i++) {
        vertexBuffers[i].destroy();
        indexBuffers[i].destroy();
    }

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        instanceUniformBuffers[i].destroy();
        viewUniformBuffers[i].destroy();
    }
    effectsParams.destroy();
    time.destroy();
    mouseControl.destroy();
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
    glfwWindowHint(GLFW_DOUBLEBUFFER, GLFW_TRUE);
    pWindow = glfwCreateWindow(width, height, APP_NAME, nullptr, nullptr);
    glfwSetWindowUserPointer(pWindow, this);
    glfwMakeContextCurrent(pWindow);
    glfwSetFramebufferSizeCallback(
        pWindow, [](GLFWwindow* window, int width, int height) {
            glm::uvec2 windowResolution = glm::uvec2(width, height);
            Controls* pControls = reinterpret_cast<Controls*>(glfwGetWindowUserPointer(window));
            ImageSegmantationSystem* pSegmentationSystem = reinterpret_cast<ImageSegmantationSystem*>(glfwGetWindowUserPointer(window));
            pControls->updateWindowSize(windowResolution);
            pSegmentationSystem->changeWindowResolution(windowResolution);
        });
}

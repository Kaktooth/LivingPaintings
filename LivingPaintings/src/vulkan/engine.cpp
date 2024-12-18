#include "engine.h"

using Data::AlignmentProperties;
using Data::RuntimeProperties;

using Constants::WINDOW_WIDTH;
using Constants::WINDOW_HEIGHT;
using Constants::IMAGE_TEXTURE_FORMAT;
using Constants::BUMP_TEXTURE_FORMAT;
using Constants::EFFECT_MASK_TEXTURE_FORMAT;
using Constants::MAX_FRAMES_IN_FLIGHT;
using Constants::OUTPUT_FOLDER_NAME;
using Constants::EXPORT_FRAME_COUNT;

using std::chrono::steady_clock;
using std::chrono::seconds;
using std::chrono::duration;

const std::string createOutputFolder = "mkdir " + OUTPUT_FOLDER_NAME;

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
    system(createOutputFolder.c_str());

    initWindow(WINDOW_WIDTH, WINDOW_HEIGHT);

    VkDebugUtilsMessengerCreateInfoEXT debugInfo = debugMessenger.makeDebugMessengerCreateInfo();
    INIT(vulkan.instance, instance.create(debugInfo));
    INIT(vulkan.debugMessenger, debugMessenger.setup(vulkan.instance, debugInfo));

    INIT(vulkan.surface, surface.create(vulkan.instance, pWindow));
    INIT(vulkan.device, device.create(vulkan.instance, surface));
    INIT(vulkan.physicalDevice, device.getPhysicalDevice());
    surface.findSurfaceDetails(vulkan.physicalDevice);
    VkSampleCountFlagBits maxSampleCount = device.getMaxSampleCount();
    if (vulkan.sampleCount > maxSampleCount) {
        std::cout << "Configuration is invalid: device does not support selected"
                  << std::to_string(vulkan.sampleCount) << "samples. "
                  << "Device supports to " << std::to_string(vulkan.sampleCount)
                  << "samples." << '\n';

        vulkan.sampleCount = maxSampleCount;
    }

    inFlightFence.create(vulkan.device, true);
    imageAvailable.create(vulkan.device);
    renderFinished.create(vulkan.device);

    const QueueFamily::Indices familyQueueIndicies = device.getQueueFamily().indicies;
    INIT(vulkan.commandPool, commandPool.create(vulkan.device, familyQueueIndicies.graphicsFamily.value()));
    graphicsCmds.create(vulkan.device, vulkan.commandPool);

    vkQueueWaitIdle(device.getComputeQueue().get());
    computeCmds.create(vulkan.device, vulkan.commandPool);

    swapchain.setContext(device, surface, vulkan.commandPool,
        vulkan.sampleCount, depthFormatCandidates);
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
    graphicsObjects[0].constructQuadWithAspectRatio(TEX_WIDTH, TEX_HEIGHT, 0.00001f);
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
        uniformBuffer.create(vulkan.device, vulkan.physicalDevice, sizeof(Data::GraphicsObject::View),
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    }

    time.create(vulkan.device, vulkan.physicalDevice, sizeof(float),
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    effectsParams.create(vulkan.device, vulkan.physicalDevice, sizeof(EffectParams),
                         VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    lightsParams.create(vulkan.device, vulkan.physicalDevice, sizeof(LightParams),
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    segmentationSystem.init(device, vulkan.commandPool, pWindow,
        TEXTURE_PATH, TEX_WIDTH, TEX_HEIGHT, controls.getMouseControls());

    controls.fillInMouseControlInfo(glm::uvec2(WINDOW_WIDTH, WINDOW_HEIGHT),
        0.1f, pWindow);

    mouseControl.create(vulkan.device, vulkan.physicalDevice, mouseUniformSize,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    Image paintingTexture;
    paintingTexture.imageDetails.createImageInfo(
        TEXTURE_PATH.c_str(), TEX_WIDTH, TEX_HEIGHT, 4,
        VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_VIEW_TYPE_2D,
        IMAGE_TEXTURE_FORMAT,
        VK_SHADER_STAGE_FRAGMENT_BIT | VK_SHADER_STAGE_COMPUTE_BIT,
        VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_ASPECT_COLOR_BIT,
        VK_SAMPLE_COUNT_1_BIT, {}, 0);
    paintingTexture.create(vulkan.device, vulkan.physicalDevice, vulkan.commandPool,
        VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, transferQueue);
    objectsTextures.push_back(paintingTexture);

    heightMapTexture.imageDetails.createImageInfo(
        "", TEX_WIDTH, TEX_HEIGHT, 1,
        VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_VIEW_TYPE_2D,
        BUMP_TEXTURE_FORMAT,
        VK_SHADER_STAGE_FRAGMENT_BIT | VK_SHADER_STAGE_COMPUTE_BIT,
        VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_ASPECT_COLOR_BIT,
        VK_SAMPLE_COUNT_1_BIT);
    heightMapTexture.create(vulkan.device, vulkan.physicalDevice, vulkan.commandPool,
        VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_STORAGE_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, transferQueue);

    textureSampler.create(vulkan.device, vulkan.physicalDevice);

    descriptor.create(vulkan.device, instanceUniformBuffers,
                      viewUniformBuffers, paintingTexture,
                      heightMapTexture, textureSampler, mouseControl,
                      segmentationSystem.getSelectedPosMasks(),
                      time, effectsParams, lightsParams);

    std::vector<VkDescriptorSetLayout> descriptorLayouts = { descriptor.getSetLayout(), descriptor.getBindlessSetLayout() };
    pipeline.create(vulkan.device, vulkan.renderPass, descriptorLayouts,
        swapchain.getExtent(), vulkan.sampleCount);

    gui.init(vulkan.instance, device, vulkan.commandPool, renderPass, swapchain, descriptor.getPool(), pWindow);

    FileSupport::setPresentationSurfaceFormat(colorFormat);
}

void Engine::update()
{
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
    Queue& graphicsQueue = device.getGraphicsQueue();
    Queue& presentationQueue = device.getPresentationQueue();
    Queue& transferQueue = device.getTransferQueue();
    Queue& computeQueue = device.getComputeQueue();
    Image::Details imageDetails = heightMapTexture.getDetails();

    forwardRenderAction.setContext(pipeline, extent, 0);

    auto runComputeShader = [&](uint currentFrame) {
        VkCommandBuffer& cmdCompute = computeCmds.get(currentFrame);

        computeCmds.begin(currentFrame);
        pipeline.bind(cmdCompute, descriptor.getSet(currentFrame), descriptor.getBindlessSet(0));
        vkCmdDispatch(cmdCompute, imageDetails.width,
                      imageDetails.height, 1);
        computeCmds.end(currentFrame);

        computeQueue.submit(cmdCompute, inFlightFence, computeWaitSemaphores,
                            computeSignalSemaphores, computeWaitStages, currentFrame);
    };

    // run compute only once
    runComputeShader(0);

    while (!glfwWindowShouldClose(pWindow)) {
        glfwPollEvents();

        steady_clock::time_point currentTime = steady_clock::now();
        float timeDuration_s = duration<float, seconds::period>(currentTime.time_since_epoch()).count();
        Controls::MouseControl& mouseControls = controls.getMouseControls();
        EffectParams& effectParams = gui.getEffectParams();
        LightParams& lightParams = gui.getLightParams();
        int16_t maskIndex = gui.getMouseControlParams().maskIndex;
        glm::dvec2 cursorPos{};
        glfwGetCursorPos(pWindow, &cursorPos.x, &cursorPos.y);
        controls.updateMousePos(cursorPos);
        controls.updateMaskIndex(maskIndex);
        mouseControl.update(mouseControls);
        time.update(timeDuration_s);
        effectsParams.update(effectParams);
        lightsParams.update(lightParams);

        segmentationSystem.updatePositionMasks(device, vulkan.commandPool, transferQueue);

        pipeline.updateExtent(swapchain.getExtent());

        for (int32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
            const uint32_t& currentFrame = swapchain.getCurrentFrame();
            VkCommandBuffer& cmdGraphics = graphicsCmds.get(currentFrame);
            VkSemaphore& currentImageAvailable = imageAvailable.get(currentFrame);
            VkSemaphore& currentRenderFinished = renderFinished.get(currentFrame);
            const std::vector<VkSemaphore> waitSemaphores{ currentImageAvailable };
            const std::vector<VkSemaphore> signalSemaphores{ currentRenderFinished };

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

                if (constructedObject.indices.size() > 0) {
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

                    InpaintingParams inpaintingParams = gui.getInpaintingParams();
                    gui.createGraphicsObjectParams(constructedObject.instanceId);
                    if (inpaintingParams.enableInpainting) {
                        segmentationSystem.inpaintImage(inpaintingParams.patchSize, objectsTextures,
                            gui.objectsParams.back(), descriptor,
                            vulkan.commandPool, transferQueue);

                        runComputeShader(currentFrame);
                    }
                }
                gui.drawParams.constructSelectedObject = false;
            }

            gui.updateGlobalAnimationParams();
            GlobalAnimationParams globAnimParams = gui.getGlobalAnimationParams();
            for (size_t i = 0; i < graphicsObjects.size(); i++) {
                ObjectParams objectParams = gui.objectsParams[i];
                Data::GraphicsObject::instanceUniform.move(objectParams);
                Data::GraphicsObject::instanceUniform.rotate(objectParams);
                Data::GraphicsObject::instanceUniform.scale(objectParams);
            }

            for (size_t i = 0; i < gui.objectsAnimationParams.size(); i++) {
                ObjectParams objectAnimationParams = gui.objectsAnimationParams[i];
                AnimationParams animationParams = gui.animationControlParams[i];
                Data::GraphicsObject::instanceUniform.transform(globAnimParams, objectAnimationParams, animationParams);
            }

            instanceUniformBuffers[currentFrame].update(Data::GraphicsObject::instanceUniform);

            VkMappedMemoryRange mappedMemoryRange{};
            mappedMemoryRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
            mappedMemoryRange.memory = instanceUniformBuffers[currentFrame].getDeviceMemory();
            mappedMemoryRange.size = instanceUniformBuffers[currentFrame].getMemorySize();
            vkFlushMappedMemoryRanges(vulkan.device, 1, &mappedMemoryRange);

            CameraParams cameraParams = gui.getCameraParams();
            Data::GraphicsObject::viewUniform.cameraView(cameraParams, extent);
            viewUniformBuffers[currentFrame].update(Data::GraphicsObject::viewUniform);

            inFlightFence.wait(currentFrame);
            inFlightFence.reset(currentFrame);

            swapchain.asquireNextImage(graphicsQueue, vulkan.renderPass,
                currentImageAvailable, pWindow);

            graphicsCmds.begin(currentFrame);

            gui.drawParams.pipelineHistorySize = pipeline.getPipelineHistorySize();
            gui.drawParams.imageLoaded = segmentationSystem.isImageLoaded();
            if (!gui.drawParams.writeFile) {
                gui.draw();
            }

            if (pipeline.recreateifShadersChanged()) {
                gui.selectPipelineindex(pipeline.getPipelineHistorySize() - 1);
                runComputeShader(currentFrame);

                inFlightFence.wait(currentFrame);
                inFlightFence.reset(currentFrame);
            }

            forwardRenderAction.setContext(pipeline, extent, gui.getSelectedPipelineIndex());
            forwardRenderAction.beginRenderPass(cmdGraphics, vulkan.renderPass,
                framebuffers, currentFrame);
            for (size_t i = 0; i < graphicsObjects.size(); i++) {
                forwardRenderAction.recordCommandBuffer(cmdGraphics,
                    descriptor.getSet(currentFrame), descriptor.getBindlessSet(0),
                    vertexBuffers[i], indexBuffers[i], graphicsObjects[i]);
            }

            if (!gui.drawParams.writeFile) {
                gui.renderDrawData(cmdGraphics);
            }

            forwardRenderAction.endRenderPass(cmdGraphics);
            graphicsCmds.end(currentFrame);

            presentationQueue.submit(cmdGraphics, inFlightFence, waitSemaphores,
                signalSemaphores, waitStages, currentFrame);

            swapchain.presentImage(graphicsQueue, vulkan.renderPass, presentationQueue.get(),
                signalSemaphores, pWindow);

            if (gui.drawParams.writeFile) {
                unsigned char* frame = swapchain.writeFrameToBuffer(cmdGraphics, transferQueue, currentFrame);
                FileSupport::gatherFrame(frame, gui.drawParams.writeFile, gui.drawParams.fileFormat);
            } else {
                FileSupport::setExportParams(gui.drawParams.frameCount, extent.width, extent.height);
            }
        }

        vkDeviceWaitIdle(vulkan.device);
    }
}

void Engine::cleanup()
{
    segmentationSystem.destroy();

    gui.destroy();

    inFlightFence.destroy();
    imageAvailable.destroy();
    renderFinished.destroy();

    pipeline.destroy();
    descriptor.destroy();
    textureSampler.destroy();
    for (size_t i = 0; i < objectsTextures.size(); i++) {
        objectsTextures[i].destroy(); 
    }
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
    lightsParams.destroy();
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

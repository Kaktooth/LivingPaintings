#pragma once
#include "../config.hpp"
#include "../segmentation/segmentation_system.h"
#include "command_buffer.h"
#include "command_pool.h"
#include "consts.h"
#include "debug.h"
#include "descriptor.h"
#include "device.h"
#include "fence.h"
#include "forward_rendering_action.h"
#include "gui.h"
#include "image.h"
#include "instance.h"
#include "pipeline.h"
#include "queue_family.h"
#include "render_pass.h"
#include "sampler.h"
#include "semaphore.h"
#include "surface.h"
#include "../utils/frame_exporter.h"
#include "vulkan/vulkan.h"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <iostream>
#include <stdexcept>
#include <vector>

#define INIT(mainHandle, instance) (mainHandle = instance)

using Constants::APP_NAME;
using Constants::TEXTURE_PATH;
using Constants::TEX_WIDTH;
using Constants::TEX_HEIGHT;
using Constants::MAX_SAMPLE_COUNT;

class Engine {

    const std::vector<VkFormat> depthFormatCandidates = {
        //VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT,
        // use low resolution depth format
        VK_FORMAT_D16_UNORM
    };

    const VkDeviceSize mouseUniformSize = sizeof(Controls::MouseControl);

    struct {
        VkInstance instance = VK_NULL_HANDLE;
        VkDebugUtilsMessengerEXT debugMessenger = VK_NULL_HANDLE;
        VkSurfaceKHR surface = VK_NULL_HANDLE;
        VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
        VkDevice device = VK_NULL_HANDLE;
        VkRenderPass renderPass = VK_NULL_HANDLE;
        VkCommandPool commandPool = VK_NULL_HANDLE;
        VkSampleCountFlagBits sampleCount = MAX_SAMPLE_COUNT;
    } vulkan;

    ImageSegmantationSystem segmentationSystem;

    GLFWwindow* pWindow = nullptr;
    VulkanInstance instance;
    VulkanDebugMessenger debugMessenger;
    Device device;
    Surface surface;
    Swapchain swapchain;
    Pipeline pipeline;
    RenderPass renderPass;
    CommandPool commandPool;
    CommandBuffer graphicsCmds;
    CommandBuffer computeCmds;
    ForwardRenderingAction forwardRenderAction;
    Semaphore imageAvailable;
    Semaphore renderFinished;
    Fence inFlightFence;
    Descriptor descriptor;
    std::vector<Data::GraphicsObject> graphicsObjects;
    std::vector<UniformBuffer> instanceUniformBuffers;
    std::vector<UniformBuffer> viewUniformBuffers;
    std::vector<VertexBuffer> vertexBuffers;
    std::vector<IndexBuffer> indexBuffers;
    Controls controls;
    UniformBuffer mouseControl;
    UniformBuffer time;
    UniformBuffer effectsParams;
    UniformBuffer lightsParams;
    std::vector<Image> objectsTextures; // contains original image of a painting and inpainted images
    Image heightMapTexture;
    Sampler textureSampler;
    Gui gui;
    SpecificDrawParams drawParams;

    void init();
    void update();
    void cleanup();
    void initWindow(const uint16_t width, const uint16_t height);

public:
    void run();
};

// This is a personal academic project. Dear PVS-Studio, please check it.

// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com

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
#include "vulkan/vulkan.h"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <iostream>
#include <stdexcept>
#include <vector>

#define INIT(mainHandle, instance) (mainHandle = instance)

// TEXTURE_FILE_PATH variable is retrieved from Cmake with macros in file
// config.hpp.in
const std::string texturePath = RETRIEVE_STRING(TEXTURE_FILE_PATH);

class Engine {

    const std::vector<VkFormat> depthFormatCandidates = {
        VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT,
        VK_FORMAT_D24_UNORM_S8_UINT, VK_FORMAT_D16_UNORM
    };

    const VkDeviceSize mouseUniformSize = sizeof(Controls::MouseControl);
    const VkDeviceSize selectedObjectsTextureSize = 1920 * 1081 * sizeof(char);

    struct {
        VkInstance instance = VK_NULL_HANDLE;
        VkDebugUtilsMessengerEXT debugMessenger = VK_NULL_HANDLE;
        VkSurfaceKHR surface = VK_NULL_HANDLE;
        VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
        VkDevice device = VK_NULL_HANDLE;
        VkRenderPass renderPass = VK_NULL_HANDLE;
        VkCommandPool commandPool = VK_NULL_HANDLE;
        VkSampleCountFlagBits sampleCount = VK_SAMPLE_COUNT_4_BIT;
    } vulkan;

    ImageSegmantationSystem segmentationSystem;

    GLFWwindow* pWindow = NULL;
    VulkanInstance instance;
    VulkanDebugMessenger debugMessenger;
    QueueFamily queueFamily;
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
    Image paintingTexture;
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

// This is a personal academic project. Dear PVS-Studio, please check it.

// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com

#pragma once
#include "command_buffer.h"
#include "command_pool.h"
#include "consts.h"
#include "debug.h"
#include "descriptor.h"
#include "device.h"
#include "fence.h"
#include "forward_rendering_action.h"
#include "graphics_pipeline.h"
#include "gui.h"
#include "image.h"
#include "instance.h"
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

class Engine {

    struct {
        VkInstance instance = VK_NULL_HANDLE;
        VkDebugUtilsMessengerEXT debugMessenger = VK_NULL_HANDLE;
        VkSurfaceKHR surface = VK_NULL_HANDLE;
        VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
        VkDevice device = VK_NULL_HANDLE;
        VkRenderPass renderPass = VK_NULL_HANDLE;
        VkCommandPool commandPool = VK_NULL_HANDLE;
    } vulkan;

    GLFWwindow* pWindow = NULL;
    VulkanInstance instance;
    VulkanDebugMessenger debugMessenger;
    QueueFamily queueFamily;
    Device device;
    Surface surface;
    Swapchain swapchain;
    GraphicsPipeline graphicsPipeline;
    RenderPass renderPass;
    CommandPool commandPool;
    CommandBuffer commandBuffer;
    ForwardRenderingAction forwardRenderAction;
    Semaphore imageAvailable;
    Semaphore renderFinished;
    Fence inFlightFence;
    VertexBuffer vertexBuffer;
    IndexBuffer indexBuffer;
    Descriptor descriptor;
    Data::GraphicsObject quad;
    std::vector<UniformBuffer> uniformBuffers;
    Image textureImage;
    Sampler textureSampler;
    Gui gui;

    void init();
    void update();
    void cleanup();
    void initWindow(const uint16_t width, const uint16_t height);

public:
    void run();
};

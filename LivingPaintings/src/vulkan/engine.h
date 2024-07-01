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
#include "framebuffer.h"
#include "graphics_pipeline.h"
#include "gui.h"
#include "image.h"
#include "instance.h"
#include "physical_device.h"
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

class Engine {

    void init();
    void update();
    void cleanup();
    void initWindow(int width, int height);

public:
    GLFWwindow* pWindow;
    VulkanInstance instance;
    VulkanDebugMessenger debugMessenger;
    PhysicalDevice physicalDevice;
    QueueFamily queueFamily;
    Device device;
    Surface surface;
    SwapChain swapChain;
    GraphicsPipeline graphicsPipeline;
    RenderPass renderPass;
    CommandPool commandPool;
    CommandBuffer commandBuffer;
    ForwardRenderingAction forwardRenderAction;
    Semaphore imageAvailable;
    Semaphore renderFinished;
    Fence inFlightFence;
    VertexData vertexData;
    VertexBuffer vertexBuffer;
    IndexBuffer indexBuffer;
    Descriptor descriptor;
    VertexData::UniformBufferObject uniformObject;
    std::vector<UniformBuffer> uniformBuffers;
    Image textureImage;
    Sampler textureSampler;
    Gui gui;

    void run();
};

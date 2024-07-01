// This is a personal academic project. Dear PVS-Studio, please check it.

// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com

#pragma once

#include "command_buffer.h"
#include "consts.h"
#include "device.h"
#include "gui_params.h"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_vulkan.h"
#include "queue_family.h"
#include "render_pass.h"
#include "swapchain.h"
#include "vulkan/vulkan.h"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

class Gui {

    ObjectParams animatedObjectParams;

    ObjectParams objectParams = animatedObjectParams = {
        { 0.0f, 0.0f, 0.0f },
        { 0.0f, 0.0f, 0.0f },
        { 1.0f, 1.0f, 1.0f } // initial object scale
    };

    CameraParams cameraParams = {
        { 45.0f, 0.0f, 45.0f }, // init for orthogonal projection
        { 0.0f, 0.0f, 0.0f },
        { 0.0f, -1.0f, 0.0f }, // camera should look down
        false, // use orthogonal projection
        true,
        50.0f,
        0.1f,
        100.0f,
        0.5f
    };

    AnimationParams animationParams = {
        false,
        0.0f,
        0.0f,
        0.0f,
        0.0f,
        0.0f,
        false,
        { "sineIn", "sineOut", "sineInOut" },
        0
    };

    void uploadFonts(VkCommandPool commandPool, Queue queue, VkDevice device);

public:
    void init(VkInstance instance, Device device, QueueFamily::Indices queueFamilyIndicies, RenderPass renderPass, SwapChain swapChain, VkCommandPool commandPool, VkDescriptorPool descriptorPool, Queue queue, GLFWwindow* window);
    void draw();
    void renderDrawData(VkCommandBuffer commandBuffer);
    void destroy();
    ObjectParams getObjectParams();
    ObjectParams getAnimatedObjectParams();
    CameraParams getCameraParams();
    AnimationParams getAnimationParams();
};

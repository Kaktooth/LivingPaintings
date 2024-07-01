// This is a personal academic project. Dear PVS-Studio, please check it.

// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com

#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include <vulkan/vulkan.h>

struct Constants {

#ifdef NDEBUG
    static const bool ENABLE_VALIDATION_LAYERS = false;
#else
    static const bool ENABLE_VALIDATION_LAYERS = true;
#endif
    static const uint32_t WINDOW_WIDTH = 800;
    static const uint32_t WINDOW_HEIGHT = 600;

    inline static const char* APP_NAME = { "Animated Paintings" };
    inline static const char* ENGINE_NAME = { "Engine" };

    inline static const std::vector<const char*> VALIDATION_LAYERS = { "VK_LAYER_KHRONOS_validation" };
    inline static const std::vector<const char*> DEVICE_EXTENTIONS = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };

    static const int MAX_FRAMES_IN_FLIGHT = 3;
};

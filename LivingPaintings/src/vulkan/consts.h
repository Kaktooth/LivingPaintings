// This is a personal academic project. Dear PVS-Studio, please check it.

// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com

#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include <vulkan/vulkan.h>

namespace Constants {

#ifdef NDEBUG
static const bool ENABLE_VALIDATION_LAYERS = false;
#else
static const bool ENABLE_VALIDATION_LAYERS = true;
#endif
static const uint16_t WINDOW_WIDTH = 1920;
static const uint16_t WINDOW_HEIGHT = 1080;

inline static const char* APP_NAME = { "Animated Paintings" };
inline static const char* ENGINE_NAME = { "Engine" };

inline static const std::vector<const char*> VALIDATION_LAYERS = { "VK_LAYER_KHRONOS_validation" };
inline static const std::vector<const char*> DEVICE_EXTENTIONS = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };

static const uint8_t MAX_FRAMES_IN_FLIGHT = 3;

static const VkFormat IMAGE_TEXTURE_FORMAT = VK_FORMAT_R8G8B8A8_SRGB;
static const VkFormat BUMP_TEXTURE_FORMAT = VK_FORMAT_R8_UNORM;

static const VkColorSpaceKHR COLOR_SPACE = VK_COLOR_SPACE_HDR10_HLG_EXT;

// from 0 - 255
static const uint8_t SELECTED_REGION_HIGHLIGHT = 100;

static const size_t OBJECT_INSTANCES = 1000;
};

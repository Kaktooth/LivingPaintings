#pragma once
#include "../config.hpp"
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

inline static const char* APP_NAME = { "Living Paintings" };
inline static const char* ENGINE_NAME = { "Engine" };

inline static const std::vector<const char*> VALIDATION_LAYERS = {
    "VK_LAYER_KHRONOS_validation"
};
inline static const std::vector<const char*> DEVICE_EXTENTIONS = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

static const uint8_t MAX_FRAMES_IN_FLIGHT = 3;

static const VkFormat IMAGE_TEXTURE_FORMAT = VK_FORMAT_R8G8B8A8_SRGB;
static const VkFormat BUMP_TEXTURE_FORMAT = VK_FORMAT_R8_UNORM;

static const VkColorSpaceKHR COLOR_SPACE = VK_COLOR_SPACE_HDR10_HLG_EXT;

// from 0 - 255
static const uint8_t SELECTED_REGION_HIGHLIGHT = 100;

static const size_t OBJECT_INSTANCES = 1000;

// TEXTURE_FILE_PATH variable is retrieved from Cmake with macros in file
// config.hpp.in
static const std::string TEXTURE_PATH = RETRIEVE_STRING(TEXTURE_FILE_PATH);
static const uint16_t TEX_WIDTH = std::stoi(RETRIEVE_STRING(TEXTURE_WIDTH));
static const uint16_t TEX_HEIGHT = std::stoi(RETRIEVE_STRING(TEXTURE_HEIGHT));
static const std::string SHADER_PATH = RETRIEVE_STRING(RESOURCE_SHADER_PATH);
static const std::string PREPROCESS_SAM_MODEL_PATH = RETRIEVE_STRING(PREPROCESS_SAM_PATH);
static const std::string SAM_MODEL_PATH = RETRIEVE_STRING(SAM_PATH);
} // namespace Constants

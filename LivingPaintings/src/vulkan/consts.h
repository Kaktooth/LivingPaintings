#pragma once
#include "../config.hpp"
#include <cstdint>
#include <string>
#include <iostream>
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

static const uint16_t UI_WINDOW_WIDTH = 550;
static const uint16_t UI_WINDOW_HEIGHT = 900;

inline static const char* APP_NAME = { "Living Paintings" };
inline static const char* ENGINE_NAME = { "Engine" };

inline static const std::vector<const char*> VALIDATION_LAYERS = {
    "VK_LAYER_KHRONOS_validation"
};
inline static const std::vector<const char*> DEVICE_EXTENTIONS = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

static const uint8_t MAX_FRAMES_IN_FLIGHT = 3;
static const VkSampleCountFlagBits MAX_SAMPLE_COUNT = VkSampleCountFlagBits::VK_SAMPLE_COUNT_4_BIT;

static const VkFormat IMAGE_TEXTURE_FORMAT = VK_FORMAT_R8G8B8A8_SRGB;
static const VkFormat BUMP_TEXTURE_FORMAT = VK_FORMAT_R8_UNORM;
static const VkFormat EFFECT_MASK_TEXTURE_FORMAT = VK_FORMAT_R8_UNORM;

static const VkColorSpaceKHR COLOR_SPACE = VK_COLOR_SPACE_HDR10_HLG_EXT;

// from 0 - 255
static const uint8_t SELECTED_REGION_HIGHLIGHT = 70;

static const size_t OBJECT_INSTANCES = 100;
static const uint32_t MAX_BINDLESS_RESOURCES = 100;

// TEXTURE_FILE_PATH variable is retrieved from Cmake with macros in file
// config.hpp.in
static const std::string TEXTURE_PATH = RETRIEVE_STRING(TEXTURE_FILE_PATH);
static const uint16_t TEX_WIDTH = std::stoi(RETRIEVE_STRING(TEXTURE_WIDTH));
static const uint16_t TEX_HEIGHT = std::stoi(RETRIEVE_STRING(TEXTURE_HEIGHT));
static const std::string SHADER_PATH = RETRIEVE_STRING(RESOURCE_SHADER_PATH);
static const std::string PREPROCESS_SAM_MODEL_PATH = RETRIEVE_STRING(PREPROCESS_SAM_PATH);
static const std::string SAM_MODEL_PATH = RETRIEVE_STRING(SAM_PATH);

static const uint16_t EFFECTS_COUNT = 3;
static constexpr uint16_t MASKS_COUNT = EFFECTS_COUNT + 1;

static const uint8_t DEFAULT_PATCH_SIZE = 20;
static const std::string INPAINTING_HISTORY_FOLDER_NAME = "InpaintingHistory";

static const std::string OUTPUT_FOLDER_NAME = "Output";

static const uint32_t STREAM_FRAME_RATE = 25;
static const uint32_t EXPORT_FRAME_COUNT = 200;
} // namespace Constants

#pragma once
#include "inpaint/criminisi_inpainter.h"
#include "../include/sam/sam.h"
#include "../vulkan/controls.h"
#include "../vulkan/image.h"
#include "../vulkan/consts.h"
#include "../vulkan/descriptor.h"
#include "glm/glm.hpp"
#include "glm/gtx/hash.hpp"
#include <chrono>
#include <iostream>
#include <map>
#include <opencv2/opencv.hpp>
#include <queue>
#include <thread>
#include <unordered_set>
#include <sstream>

using Constants::MASKS_COUNT;

class ImageSegmantationSystem {

    VkDevice device = VK_NULL_HANDLE;
    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
    std::array<Image, MASKS_COUNT> selectedPosMasks;

    bool callbackIsSet = false;

    cv::Mat segmentImage(Sam const* sam);

public:
    void runObjectSegmentationTask();

    void init(Device& _device, VkCommandPool& _commandPool, GLFWwindow* pWindow,
              const std::string& imagePath, uint32_t width, uint32_t height,
              Controls::MouseControl& mouseControl);
    void destroy();

    void changeWindowResolution(glm::uvec2& windowResolution);
    void removeAllMaskPositions();
    void removeAllMaskPositions(uint16_t maskIndex);
    bool selectedObjectSizeChanged();
    bool selectedObjectSizeChanged(uint16_t maskIndex);
    void updatePositionMasks(Device& device, VkCommandPool& commandPool, Queue& transferQueue);
    void inpaintImage(uint8_t patchSize, std::vector<Image>& objectsTextures, Descriptor& descriptor, VkCommandPool& commandPool, Queue& transferQueue);
    uchar* getLatestBackground();
    bool& isImageLoaded();
    uchar* getSelectedPositionsMask();
    uchar* getSelectedPositionsMask(uint16_t maskIndex);
    std::array<Image, MASKS_COUNT>& getSelectedPosMasks();
};

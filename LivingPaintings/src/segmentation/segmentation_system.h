#pragma once
#include "../include/sam/sam.h"
#include "../vulkan/controls.h"
#include "../vulkan/image.h"
#include "../vulkan/consts.h"
#include "glm/glm.hpp"
#include "glm/gtx/hash.hpp"
#include "opencv2/xphoto/inpainting.hpp"
#include <chrono>
#include <iostream>
#include <map>
#include <opencv2/opencv.hpp>
#include <queue>
#include <thread>
#include <unordered_set>

using Constants::MASKS_COUNT;

class ImageSegmantationSystem {

    VkDevice device = VK_NULL_HANDLE;
    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
    VkImageCopy imageCopy;
    std::array<Image, MASKS_COUNT> selectedPosMasks;
    Image selectedPosMaskTemp;

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
    bool& isImageLoaded();
    unsigned char* getSelectedPositionsMask();
    unsigned char* getSelectedPositionsMask(uint16_t maskIndex);
    std::array<Image, MASKS_COUNT>& getSelectedPosMasks();
};

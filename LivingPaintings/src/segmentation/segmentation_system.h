#pragma once
#include "../include/sam/sam.h"
#include "../vulkan/controls.h"
#include "../vulkan/image.h"
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

class ImageSegmantationSystem {

    VkDevice device = VK_NULL_HANDLE;
    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
    VkImageCopy imageCopy;
    Image selectedPositionsMask;
    Image selectedPositionsMaskTemp;

    cv::Mat segmentImage(Sam const* sam);

public:
    void runObjectSegmentationTask();

    void init(Device& _device, VkCommandPool& _commandPool, GLFWwindow* pWindow,
              const std::string& imagePath, uint32_t width, uint32_t height,
        Controls::MouseControl& mouseControl);
    void destroy();

    void changeWindowResolution(glm::uvec2& windowResolution);
    void removeAllPositions();
    bool selectedObjectSizeChanged();
    void updateSelectedImageMask(Device& device, VkCommandPool& commandPool, Queue& transferQueue);
    void clearSelectedPixels();
    bool& isImageLoaded();
    unsigned char* getSelectedPositionsMask();
    Image& getSelectedObjectsMask();
};

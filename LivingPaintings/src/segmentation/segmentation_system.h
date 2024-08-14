// This is a personal academic project. Dear PVS-Studio, please check it.

// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com

#pragma once
#include <thread>
#include "glm/glm.hpp"
#include "glm/gtx/hash.hpp"
#include <map>
#include <unordered_set>
#include <queue>
#include <iostream>
#include <chrono>
#include <opencv2/opencv.hpp>
#include "opencv2/xphoto/inpainting.hpp"
#include "../include/sam/sam.h"
#include "../vulkan/controls.h"
#include "../vulkan/image.h"

class ImageSegmantationSystem {
  
    VkDevice device = VK_NULL_HANDLE;
    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
    VkImageCopy imageCopy;
    Image selectedPositionsMask;
    Image selectedPositionsMaskTemp;

	cv::Mat ImageSegmantationSystem::segmentImage(Sam const* sam);

public:
    // TODO make vertecies for selected object creation
    void runObjectSegmentationTask();

	void init(Device& _device, VkCommandPool& _commandPool, GLFWwindow* pWindow,
              std::string imagePath, uint32_t width, uint32_t height,
              Controls::MouseControl& mouseControl);	
	void destroy();

    void changeWindowResolution(glm::uvec2& windowResolution);
    void removeAllPositions();
    bool selectedObjectSizeChanged();
    void updateSelectedImageMask(Device& device, VkCommandPool&, Queue& transferQueue);
    bool& isImageLoaded();
    unsigned char* getSelectedPositionsMask();
    Image& getSelectedObjectsMask();
};
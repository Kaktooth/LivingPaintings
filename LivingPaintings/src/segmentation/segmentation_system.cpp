// This is a personal academic project. Dear PVS-Studio, please check it.

// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com

#include "segmentation_system.h"
#include "../config.hpp"
#include "../vulkan/consts.h"

using namespace Constants;

const std::string preprocessModelPath = RETRIEVE_STRING(PREPROCESS_SAM_PATH);
const std::string modelPath = RETRIEVE_STRING(SAM_PATH);
const int THREAD_NUMBER = std::thread::hardware_concurrency();

bool runningSegmentation = true;
bool imageLoaded = false;
std::string imagePath;

GLFWwindow* pWindow = NULL;
Queue transferQueue;
Controls::MouseControl* mouseControl = NULL;
std::unique_ptr<Sam> samModel = NULL;
glm::uvec2 modelResolution;

cv::Mat imageTexture;
glm::uvec2 imageResolution;
glm::uvec2 windowResolution;
// Resize point using old (imageResolution) and new (windowResolution) resolutions.
#define RESIZE_POINT_POSITION(pointPos) ((pointPos / glm::vec2(windowResolution)) * glm::vec2(imageResolution))

std::unordered_set<glm::uvec2> brushPositions { { 0, 0 }, { 1, 0 }, { -1, 0 }, { 0, 1 },
    { 0, -1 }, { 1, 1 }, { -1, -1 }, { 1, -1 },
    { -1, 1 } };

std::unordered_map<glm::uvec2, glm::uvec2> objectPositions {};
unsigned char* selectedPosMask;
uint32_t selectedObjectsSize = 0;
uint32_t currentSelectedObjectsSize = 0;
cv::Mat maskedImage;

std::queue<glm::uvec2> inputPositions {};
std::thread objectSelectionThread;

// 1 - creating button, 2 - removing button
std::pair<bool, bool> buttonHeld;

static Sam::Parameter getSamParam(std::string const& preprocessModel,
    std::string const& segmentModel,
    int preprocessDevice, int segmentDevice)
{
    Sam::Parameter param(preprocessModel, segmentModel, THREAD_NUMBER);
    param.providers[0].deviceType = preprocessDevice;
    param.providers[1].deviceType = segmentDevice;
    return param;
}

Sam::Parameter paramSam = getSamParam(preprocessModelPath.c_str(), modelPath.c_str(), 0, 0);

static void loadImage(Sam* sam, std::string const& inputImage)
{
    cv::Size inputSize = sam->getInputSize();
    std::cout << "Model resolution: " << std::endl
              << "  width: " << inputSize.width << std::endl
              << "  height: " << inputSize.height << std::endl;

    modelResolution = glm::uvec2(inputSize.width, inputSize.height);

    if (inputSize.empty()) {
        std::cout << "Sam initialization failed" << std::endl;
    }

    cv::Mat image = cv::imread(inputImage, -1);
    if (image.empty()) {
        std::cout << "Image is empty, image loading failed" << std::endl;
    }

    imageResolution = glm::uvec2(image.cols, image.rows);
    std::cout << "Loading image with resolution: " << std::endl
              << "  width: " << image.cols << std::endl
              << "  height: " << image.rows << std::endl;

    windowResolution = imageResolution;
    sam->setWindowResolution(windowResolution.x, windowResolution.y);

    resize(image, image, inputSize);
    if (!sam->loadImage(image)) {
        std::cout << "Image loading failed" << std::endl;
    } else {
        std::cout << "Image is loaded!" << std::endl;
    }
    imageTexture = image.clone();
}

cv::Mat ImageSegmantationSystem::segmentImage(Sam const* sam)
{
    double xpos;
    double ypos;
    glfwGetCursorPos(pWindow, &xpos, &ypos);
    cv::Point point(xpos, ypos);
    return sam->getMask(point);
}

void useBrush(glm::uvec2 pos)
{
    if (buttonHeld.first) {
        std::cout << "Holding. Pixel position: " << pos.x << " " << pos.y
                  << std::endl;
        for (glm::uvec2 brushPos : brushPositions) {
            objectPositions.emplace(pos + brushPos, pos + brushPos);
        }
    }
    if (buttonHeld.second) {
        for (glm::uvec2 brushPos : brushPositions) {
            if (objectPositions.contains(pos + brushPos)) {
                std::cout << "Removing selected pixel: " << pos.x << " " << pos.y
                          << std::endl;
                objectPositions.erase(pos + brushPos);
            }
        }
    }
}

void cursor_position_callback(GLFWwindow* window, double xpos, double ypos)
{

    if (buttonHeld.first || buttonHeld.second) {
        glm::uvec2 pos = RESIZE_POINT_POSITION(glm::vec2(xpos, ypos));
        useBrush(pos);
    }
}

void mouse_buttons_callback(GLFWwindow* window, int button, int action,
    int mods)
{
    if (imageLoaded) {
        if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE) {
            currentSelectedObjectsSize = objectPositions.size();
            buttonHeld.first = false;
        }
        if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_RELEASE) {
            currentSelectedObjectsSize = objectPositions.size();
            buttonHeld.second = false;
        }
        if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS && mods == GLFW_MOD_CONTROL) {
            glm::dvec2 cursorPos {};
            glfwGetCursorPos(window, &cursorPos.x, &cursorPos.y);
            glm::uvec2 pos = glm::uvec2(cursorPos);
            std::cout << "Selected pixel position: " << pos.x << " " << pos.y
                      << std::endl;
            if (mouseControl->pixelScaling) {
                buttonHeld.first = true;
                useBrush(pos);
            } else {
                inputPositions.push(pos);
            }
        }
        if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS && mods == GLFW_MOD_CONTROL) {
            glm::dvec2 cursorPos {};
            glfwGetCursorPos(window, &cursorPos.x, &cursorPos.y);
            glm::uvec2 pos = RESIZE_POINT_POSITION(glm::vec2(cursorPos));

            if (mouseControl->pixelScaling) {
                buttonHeld.second = true;
                useBrush(pos);
            } else {
                if (objectPositions.contains(pos)) {
                    std::cout << "Removing region that contains pixel: " << pos.x << " "
                              << pos.y << std::endl;
                    glm::uvec2 pressedKeyPos = objectPositions.at(pos);
                    std::erase_if(objectPositions,
                        [&](const std::pair<glm::uvec2, glm::uvec2>& entry) {
                            return entry.second == pressedKeyPos;
                        });
                    currentSelectedObjectsSize = objectPositions.size();
                }
            }
        }
    }
}

void ImageSegmantationSystem::runObjectSegmentationTask()
{

    std::cout << "Starting to build Segment Anything model... " << std::endl;
    samModel = std::make_unique<Sam>(paramSam);
    std::cout << "Building is finished!" << std::endl;

    loadImage(samModel.get(), imagePath);
    imageLoaded = true;

    const VkDeviceSize maskSize = imageResolution.x * imageResolution.y * sizeof(char);
    while (runningSegmentation) {
        bool inputPositionsEmpty = inputPositions.empty();
        if (!inputPositionsEmpty) {
            glm::uvec2 pos = inputPositions.front();
            inputPositions.pop();

            cv::Mat mask = segmentImage(samModel.get());
            uint8_t* pMaskPixels = (uint8_t*)mask.data;
            uint8_t channels = imageTexture.channels();

            cv::imwrite("D:\\Downloads\\mask.jpg", mask);
            maskedImage = imageTexture.clone();
            uint8_t* pPixels = (uint8_t*)maskedImage.data;
            for (uint32_t h = 0; h < imageTexture.rows; h++) {
                for (uint32_t w = 0; w < imageTexture.cols; w++) {
                    uint8_t r = pMaskPixels[h * mask.cols + w + 2];
                    uint8_t g = pMaskPixels[h * mask.cols + w + 1];
                    uint8_t b = pMaskPixels[h * mask.cols + w];
                    if (r == 255 && g == 255 && b == 255) {
                        pPixels[h * imageTexture.cols * channels + w * channels + 2] = 0;
                        pPixels[h * imageTexture.cols * channels + w * channels + 1] = 0;
                        pPixels[h * imageTexture.cols * channels + w * channels] = 0;
                    }
                }
            }
            // cv::Mat inpaintedImage;
            // cv::xphoto::inpaint(maskedImage, ~mask, inpaintedImage,
            //                     cv::xphoto::INPAINT_FSR_FAST);
            // cv::imwrite("D:\\Downloads\\inpaintedImage.jpg", inpaintedImage);

            cv::resize(mask, mask, cv::Size(imageResolution.x, imageResolution.y));
            uint8_t* pResisedMaskPixels = (uint8_t*)mask.data;
            cv::resize(maskedImage, maskedImage,
                cv::Size(imageResolution.x, imageResolution.y));
            cv::imwrite("D:\\Downloads\\maskedImage.jpg", maskedImage);

            for (uint32_t h = 0; h < imageResolution.y; h++) {
                for (uint32_t w = 0; w < imageResolution.x; w++) {
                    uint8_t r = pResisedMaskPixels[h * mask.cols + w + 2];
                    uint8_t g = pResisedMaskPixels[h * mask.cols + w + 1];
                    uint8_t b = pResisedMaskPixels[h * mask.cols + w];
                    if (r == 255 && g == 255 && b == 255) {
                        objectPositions.emplace(glm::ivec2(w, h), pos);
                    }
                }
            }

            std::cout << "objects selected " << std::endl;
            currentSelectedObjectsSize = objectPositions.size();
        }
    }
}

void ImageSegmantationSystem::init(Device& _device, VkCommandPool& _commandPool,
    GLFWwindow* _pWindow, std::string _imagePath,
    uint32_t imageWidth, uint32_t imageHeight,
    Controls::MouseControl& _mouseControl)
{

    pWindow = _pWindow;
    imagePath = _imagePath;
    mouseControl = &_mouseControl;
    device = _device.get();
    physicalDevice = _device.getPhysicalDevice();
    transferQueue = _device.getTransferQueue();

    std::cout << "___ Initialization Phase ___ " << std::endl;
    std::cout << "Threads: " << THREAD_NUMBER << std::endl;

    imageResolution = glm::uvec2(imageWidth, imageHeight);

    selectedPositionsMask.imageDetails.createImageInfo(
        "", imageWidth, imageHeight, 1,
        VK_IMAGE_LAYOUT_GENERAL,
        VK_IMAGE_VIEW_TYPE_2D,
        BUMP_TEXTURE_FORMAT, VK_SHADER_STAGE_FRAGMENT_BIT,
        VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_ASPECT_COLOR_BIT, VK_SAMPLE_COUNT_1_BIT);
    selectedPositionsMask.create(_device, _commandPool,
        VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, transferQueue);

    VkImageSubresourceLayers imageSubresource {};
    imageSubresource.aspectMask = selectedPositionsMask.imageDetails.aspectFlags;
    imageSubresource.mipLevel = 0;
    imageSubresource.baseArrayLayer = 0;
    imageSubresource.layerCount = 1;

    imageCopy.srcSubresource = imageSubresource;
    imageCopy.srcOffset = { 0, 0, 0 };
    imageCopy.dstSubresource = imageSubresource;
    imageCopy.dstOffset = { 0, 0, 0 };
    imageCopy.extent = { imageResolution.x, imageResolution.y, 1 };

    glfwSetMouseButtonCallback(pWindow, mouse_buttons_callback);
    glfwSetCursorPosCallback(pWindow, cursor_position_callback);
    objectSelectionThread = std::thread(&ImageSegmantationSystem::runObjectSegmentationTask, *this);
}

void ImageSegmantationSystem::destroy()
{
    runningSegmentation = false;
    if (objectSelectionThread.joinable()) {
        objectSelectionThread.join();
    }
    selectedPositionsMask.destroy();
}

void ImageSegmantationSystem::changeWindowResolution(glm::uvec2& _windowResolution)
{
    windowResolution = _windowResolution;
    samModel->setWindowResolution(windowResolution.x, windowResolution.y);
}

void ImageSegmantationSystem::removeAllPositions() { objectPositions.clear(); }

bool ImageSegmantationSystem::selectedObjectSizeChanged()
{
    if (selectedObjectsSize != currentSelectedObjectsSize) {
        selectedObjectsSize = currentSelectedObjectsSize;
        return true;
    }

    return false;
}

void ImageSegmantationSystem::updateSelectedImageMask(
    Device& device, VkCommandPool& commandPool, Queue& transferQueue)
{
    if (selectedObjectSizeChanged()) {
        unsigned char* selectedPosMask = getSelectedPositionsMask();

        selectedPositionsMaskTemp.imageDetails.createImageInfo(
            "", imageResolution.x, imageResolution.y, 1, VK_IMAGE_LAYOUT_GENERAL,
            VK_IMAGE_VIEW_TYPE_2D,
            BUMP_TEXTURE_FORMAT, VK_SHADER_STAGE_FRAGMENT_BIT,
            VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_ASPECT_COLOR_BIT,
            VK_SAMPLE_COUNT_1_BIT, selectedPosMask);
        selectedPositionsMaskTemp.create(device, commandPool,
            VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, transferQueue);

        VkCommandBuffer cmd = CommandBuffer::beginSingleTimeCommands(device.get(), commandPool);

        vkCmdCopyImage(cmd, selectedPositionsMaskTemp.get(),
            VK_IMAGE_LAYOUT_GENERAL,
            selectedPositionsMask.get(),
            VK_IMAGE_LAYOUT_GENERAL, 1, &imageCopy);

        CommandBuffer::endSingleTimeCommands(device.get(), commandPool, cmd,
            transferQueue);

        selectedPositionsMaskTemp.destroy();
    }
}

bool& ImageSegmantationSystem::isImageLoaded() { return imageLoaded; }

unsigned char* ImageSegmantationSystem::getSelectedPositionsMask()
{
    unsigned char* selectedPositionsMask = (unsigned char*)calloc(
        1, imageResolution.x * imageResolution.y);
    for (uint32_t w = 0; w < imageResolution.x; w++) {
        for (uint32_t h = 0; h < imageResolution.y; h++) {
            unsigned char* offset = selectedPositionsMask + w + (imageResolution.x * h);
            glm::uvec2 pos = glm::uvec2(w, h);
            if (objectPositions.contains(pos)) {
                offset[0] = SELECTED_REGION_HIGHLIGHT;
            } else {
                offset[0] = 0;
            }
        }
    }
    return selectedPositionsMask;
}

Image& ImageSegmantationSystem::getSelectedObjectsMask()
{
    return selectedPositionsMask;
}

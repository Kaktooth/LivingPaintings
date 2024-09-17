#include "segmentation_system.h"
#include "../config.hpp"
#include "../vulkan/consts.h"

using Constants::PREPROCESS_SAM_MODEL_PATH;
using Constants::SAM_MODEL_PATH;
using Constants::BUMP_TEXTURE_FORMAT;
using Constants::SELECTED_REGION_HIGHLIGHT;
using Constants::MASKS_COUNT;
using Constants::WINDOW_WIDTH;
using Constants::WINDOW_HEIGHT;

const int THREAD_NUMBER = std::thread::hardware_concurrency();

bool runningSegmentation = true;
bool imageLoaded = false;
std::string imagePath;

GLFWwindow* pWindow = nullptr;
Controls::MouseControl* mouseControl = nullptr;
std::unique_ptr<Sam> samModel = nullptr;
glm::uvec2 modelResolution;

cv::Mat imageTexture;
glm::uvec2 imageResolution;
glm::uvec2 windowResolution;

// Resize point using old (imageResolution) and new (windowResolution) resolutions.
#define RESIZE_POINT_POSITION(pointPos) (((pointPos) / glm::vec2(windowResolution)) * glm::vec2(imageResolution))

std::unordered_set<glm::uvec2> brushPositions { { 0, 0 }, { 1, 0 }, { -1, 0 }, { 0, 1 },
    { 0, -1 }, { 1, 1 }, { -1, -1 }, { 1, -1 },
    { -1, 1 } };

std::array<std::unordered_map<glm::uvec2, glm::uvec2>, MASKS_COUNT> objectPositions {};
std::array<uint32_t, MASKS_COUNT> selectedObjectsSize{};
std::array<uint32_t, MASKS_COUNT> currentSelectedObjectsSize{};
unsigned char* selectedPosMask;

cv::Mat maskedImage;

std::queue<glm::uvec2> inputPositions {};
std::thread objectSelectionThread;

// First value is for condition when user selecting pixels and second value is for unselecting pixels.
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

Sam::Parameter paramSam = getSamParam(PREPROCESS_SAM_MODEL_PATH, SAM_MODEL_PATH, 0, 0);

static void loadImage(Sam* sam, std::string const& inputImage)
{
    cv::Size inputSize = sam->getInputSize();
    std::cout << "Model resolution: " << '\n'
              << "  width: " << inputSize.width << '\n'
              << "  height: " << inputSize.height << '\n';

    modelResolution = glm::uvec2(inputSize.width, inputSize.height);

    if (inputSize.empty()) {
        std::cout << "Sam initialization failed" << '\n';
    }

    cv::Mat image = cv::imread(inputImage, -1);
    if (image.empty()) {
        std::cout << "Image is empty, image loading failed" << '\n';
    }

    imageResolution = glm::uvec2(image.cols, image.rows);
    std::cout << "Loading image with resolution: " << '\n'
              << "  width: " << image.cols << '\n'
              << "  height: " << image.rows << '\n';

    sam->setWindowResolution(windowResolution.x, windowResolution.y);

    resize(image, image, inputSize);
    if (!sam->loadImage(image)) {
        std::cout << "Image loading failed" << '\n';
    } else {
        std::cout << "Image is loaded!" << '\n';
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

static void useBrush(glm::uvec2 pos)
{
    if (buttonHeld.first) {
        std::cout << "Holding. Pixel position: " << pos.x << " " << pos.y << '\n';
        for (glm::uvec2 brushPos : brushPositions) {
            objectPositions[mouseControl->maskIndex].emplace(pos + brushPos, pos + brushPos);
        }
    }
    if (buttonHeld.second) {
        for (glm::uvec2 brushPos : brushPositions) {
            if (objectPositions[mouseControl->maskIndex].contains(pos + brushPos)) {
                std::cout << "Removing selected pixel: " << pos.x << " " << pos.y
                          << '\n';
                objectPositions[mouseControl->maskIndex].erase(pos + brushPos);
            }
        }
    }
}

static void cursor_position_callback(GLFWwindow* window, double xpos, double ypos)
{
    if (buttonHeld.first || buttonHeld.second) {
        glm::uvec2 pos = RESIZE_POINT_POSITION(glm::vec2(xpos, ypos));
        useBrush(pos);
    }
}

static void mouse_buttons_callback(GLFWwindow* window, int button, int action,
    int mods)
{
    if (imageLoaded) {
        if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE) {
            currentSelectedObjectsSize[mouseControl->maskIndex] = objectPositions[mouseControl->maskIndex].size();
            buttonHeld.first = false;
        }
        if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_RELEASE) {
            currentSelectedObjectsSize[mouseControl->maskIndex] = objectPositions[mouseControl->maskIndex].size();
            buttonHeld.second = false;
        }
        if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS && mods == GLFW_MOD_CONTROL) {
            glm::dvec2 cursorPos {};
            glfwGetCursorPos(window, &cursorPos.x, &cursorPos.y);
            glm::uvec2 pos = RESIZE_POINT_POSITION(glm::vec2(cursorPos));
            std::cout << "Selected pixel position: " << pos.x << " " << pos.y << '\n';
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
                if (objectPositions[mouseControl->maskIndex].contains(pos)) {
                    std::cout << "Removing region that contains pixel: " << pos.x << " " << pos.y << '\n';
                    glm::uvec2 pressedKeyPos = objectPositions[mouseControl->maskIndex].at(pos);
                    std::erase_if(objectPositions[mouseControl->maskIndex],
                        [&](const std::pair<glm::uvec2, glm::uvec2>& entry) {
                            return entry.second == pressedKeyPos;
                        });
                    currentSelectedObjectsSize[mouseControl->maskIndex] = objectPositions[mouseControl->maskIndex].size();
                }
            }
        }
    }
}

void ImageSegmantationSystem::runObjectSegmentationTask()
{

    std::cout << "Starting to build Segment Anything model... " << '\n';
    samModel = std::make_unique<Sam>(paramSam);
    std::cout << "Building is finished!" << '\n';

    loadImage(samModel.get(), imagePath);
    imageLoaded = true;

    const VkDeviceSize maskSize = static_cast<size_t>(imageResolution.x) 
                                * static_cast<size_t>(imageResolution.y) * sizeof(char);
    while (runningSegmentation) {
        bool inputPositionsEmpty = inputPositions.empty();
        if (!inputPositionsEmpty) {
            glm::uvec2 pos = inputPositions.front();
            inputPositions.pop();

            cv::Mat mask = segmentImage(samModel.get());
            auto pMaskPixels = static_cast<uint8_t*>(mask.data);
            uint8_t channels = imageTexture.channels();

            cv::imwrite("D:\\Downloads\\mask.jpg", mask);
            maskedImage = imageTexture.clone();
            auto pPixels = static_cast<uint8_t*>(maskedImage.data);
            for (int height = 0; height < imageTexture.rows; height++) {
                for (int width = 0; width < imageTexture.cols; width++) {
                    uint8_t red = pMaskPixels[height * mask.cols + width + 2];
                    uint8_t green = pMaskPixels[height * mask.cols + width + 1];
                    uint8_t blue = pMaskPixels[height * mask.cols + width];
                    if (red == 255 && green == 255 && blue == 255) {
                        pPixels[height * imageTexture.cols * channels + width * channels + 2] = 0;
                        pPixels[height * imageTexture.cols * channels + width * channels + 1] = 0;
                        pPixels[height * imageTexture.cols * channels + width * channels] = 0;
                    }
                }
            }
            // cv::Mat inpaintedImage;
            // cv::xphoto::inpaint(maskedImage, ~mask, inpaintedImage,
            //                     cv::xphoto::INPAINT_FSR_FAST);
            // cv::imwrite("D:\\Downloads\\inpaintedImage.jpg", inpaintedImage);

            cv::resize(mask, mask, cv::Size(imageResolution.x, imageResolution.y));
            auto pResisedMaskPixels = static_cast<uint8_t*>(mask.data);
            cv::resize(maskedImage, maskedImage, cv::Size(imageResolution.x, imageResolution.y));
            cv::imwrite("D:\\Downloads\\maskedImage.jpg", maskedImage);

            for (uint32_t height = 0; height < imageResolution.y; height++) {
                for (uint32_t width = 0; width < imageResolution.x; width++) {
                    uint8_t red = pResisedMaskPixels[height * mask.cols + width + 2];
                    uint8_t green = pResisedMaskPixels[height * mask.cols + width + 1];
                    uint8_t blue = pResisedMaskPixels[height * mask.cols + width];
                    if (red == 255 && green == 255 && blue == 255) {
                        objectPositions[mouseControl->maskIndex].emplace(glm::ivec2(width, height), pos);
                    }
                }
            }

            std::cout << "objects selected " << '\n';
            currentSelectedObjectsSize[mouseControl->maskIndex] = objectPositions[mouseControl->maskIndex].size();
        }
    }
}

void ImageSegmantationSystem::init(Device& _device, VkCommandPool& _commandPool,
    GLFWwindow* _pWindow, const std::string& _imagePath,
    uint32_t imageWidth, uint32_t imageHeight,
    Controls::MouseControl& _mouseControl)
{
    const glm::uvec2 windowSize = glm::uvec2(WINDOW_WIDTH, WINDOW_HEIGHT);

    pWindow = _pWindow;
    windowResolution = windowSize;
    imagePath = _imagePath;
    mouseControl = &_mouseControl;
    device = _device.get();
    physicalDevice = _device.getPhysicalDevice();
    Queue& transferQueue = _device.getTransferQueue();

    std::cout << "___ Initialization Phase ___ " << '\n';
    std::cout << "Threads: " << THREAD_NUMBER << '\n';

    imageResolution = glm::uvec2(imageWidth, imageHeight);

    for (uint16_t maskIndex = 0; maskIndex < MASKS_COUNT; maskIndex++) {
        selectedPosMasks[maskIndex].imageDetails.createImageInfo(
            "", imageWidth, imageHeight, 1,
            VK_IMAGE_LAYOUT_GENERAL,
            VK_IMAGE_VIEW_TYPE_2D,
            BUMP_TEXTURE_FORMAT, VK_SHADER_STAGE_FRAGMENT_BIT,
            VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_ASPECT_COLOR_BIT, VK_SAMPLE_COUNT_1_BIT);
        selectedPosMasks[maskIndex].create(_device, _commandPool,
                                     VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
                                     VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, transferQueue);
    }

    VkImageSubresourceLayers imageSubresource {};
    imageSubresource.aspectMask = selectedPosMasks[0].imageDetails.aspectFlags;
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
    for (uint16_t maskIndex = 0; maskIndex < MASKS_COUNT; maskIndex++) {
        selectedPosMasks[maskIndex].destroy();
    }
}

void ImageSegmantationSystem::changeWindowResolution(glm::uvec2& _windowResolution)
{
    windowResolution = _windowResolution;
    samModel->setWindowResolution(windowResolution.x, windowResolution.y);
}

void ImageSegmantationSystem::removeAllMaskPositions() { objectPositions[mouseControl->maskIndex].clear(); }

void ImageSegmantationSystem::removeAllMaskPositions(uint16_t maskIndex)
{
    objectPositions[maskIndex].clear();
}

bool ImageSegmantationSystem::selectedObjectSizeChanged()
{
    return selectedObjectSizeChanged(mouseControl->maskIndex);
}

bool ImageSegmantationSystem::selectedObjectSizeChanged(uint16_t maskIndex)
{
    if (selectedObjectsSize[maskIndex] != currentSelectedObjectsSize[maskIndex]) {
        selectedObjectsSize[maskIndex] = currentSelectedObjectsSize[maskIndex];
        return true;
    }

    return false;
}

void ImageSegmantationSystem::updatePositionMasks(Device& device, VkCommandPool& commandPool, Queue& transferQueue)
{
    if (selectedObjectSizeChanged()) {
        unsigned char* selectedPosMask = getSelectedPositionsMask();

        selectedPosMaskTemp.imageDetails.createImageInfo(
            "", imageResolution.x, imageResolution.y, 1, VK_IMAGE_LAYOUT_GENERAL,
            VK_IMAGE_VIEW_TYPE_2D,
            BUMP_TEXTURE_FORMAT, VK_SHADER_STAGE_FRAGMENT_BIT,
            VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_ASPECT_COLOR_BIT,
            VK_SAMPLE_COUNT_1_BIT, selectedPosMask);
        selectedPosMaskTemp.create(device, commandPool,
            VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, transferQueue);

        VkCommandBuffer cmd = CommandBuffer::beginSingleTimeCommands(device.get(), commandPool);

        vkCmdCopyImage(cmd, selectedPosMaskTemp.get(),
            VK_IMAGE_LAYOUT_GENERAL,
                       selectedPosMasks[mouseControl->maskIndex].get(),
            VK_IMAGE_LAYOUT_GENERAL, 1, &imageCopy);

        CommandBuffer::endSingleTimeCommands(device.get(), commandPool, cmd, transferQueue);

        selectedPosMaskTemp.destroy();
    }
}

bool& ImageSegmantationSystem::isImageLoaded() { return imageLoaded; }

unsigned char* ImageSegmantationSystem::getSelectedPositionsMask()
{
    return getSelectedPositionsMask(mouseControl->maskIndex);
}

unsigned char* ImageSegmantationSystem::getSelectedPositionsMask(uint16_t maskIndex)
{
   auto pixelSize = static_cast<size_t>(imageResolution.x) * static_cast<size_t>(imageResolution.y);
   auto selectedPositionsMask = static_cast<unsigned char*>(calloc(1, pixelSize));
    for (uint32_t w = 0; w < imageResolution.x; w++) {
        for (uint32_t h = 0; h < imageResolution.y; h++) {
            unsigned char* offset = selectedPositionsMask + w + (imageResolution.x * h);
            auto pos = glm::uvec2(w, h);
            if (objectPositions[maskIndex].contains(pos)) {
                offset[0] = SELECTED_REGION_HIGHLIGHT;
            } else {
                offset[0] = 0;
            }
        }
    }
    return selectedPositionsMask;
}

std::array<Image, MASKS_COUNT>& ImageSegmantationSystem::getSelectedPosMasks()
{
    return selectedPosMasks;
}
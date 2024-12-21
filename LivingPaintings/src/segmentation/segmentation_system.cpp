#include "segmentation_system.h"
#include "../config.hpp"
#include "../vulkan/consts.h"
#include "inpaint/criminisi_inpainter.h"

using Constants::PREPROCESS_SAM_MODEL_PATH;
using Constants::SAM_MODEL_PATH;
using Constants::BUMP_TEXTURE_FORMAT;
using Constants::SELECTED_REGION_HIGHLIGHT;
using Constants::MASKS_COUNT;
using Constants::WINDOW_WIDTH;
using Constants::WINDOW_HEIGHT;
using Constants::INPAINTING_HISTORY_FOLDER_NAME;
using Constants::IMAGE_TEXTURE_FORMAT;

const int THREAD_NUMBER = std::thread::hardware_concurrency();

bool runningSegmentation = true;
bool imageLoaded = false;
std::string imagePath;

GLFWwindow* pWindow = nullptr;
Controls::MouseControl* mouseControl = nullptr;
std::unique_ptr<Sam> samModel = nullptr;
glm::uvec2 modelResolution;

cv::Mat latestImageTexture;
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

cv::Mat image;
cv::Mat selectedPosMask;

std::queue<glm::uvec2> inputPositions {};
std::thread objectSelectionThread;

// First value is for condition when user selecting pixels and second value is for unselecting pixels.
std::pair<bool, bool> buttonHeld;

Inpaint::CriminisiInpainter inpainter;

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

    cv::Mat image = cv::imread(inputImage, cv::IMREAD_UNCHANGED);
    latestImageTexture = image.clone();
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

    //load image again because previous image has lost resolution after resize
    image = latestImageTexture;
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

    while (runningSegmentation) {
        bool inputPositionsEmpty = inputPositions.empty();
        if (!inputPositionsEmpty) {
            //TODO pos to mouse
            glm::uvec2 pos = inputPositions.front();
            inputPositions.pop();

            selectedPosMask = segmentImage(samModel.get());
            image = latestImageTexture.clone();
   
            cv::resize(selectedPosMask, selectedPosMask, cv::Size(imageResolution.x, imageResolution.y));
            auto pResisedMaskPixels = static_cast<uint8_t*>(selectedPosMask.data);

            for (uint32_t height = 0; height < imageResolution.y; height++) {
                for (uint32_t width = 0; width < imageResolution.x; width++) {
                    uint8_t red = pResisedMaskPixels[height * selectedPosMask.cols + width + 2];
                    uint8_t green = pResisedMaskPixels[height * selectedPosMask.cols + width + 1];
                    uint8_t blue = pResisedMaskPixels[height * selectedPosMask.cols + width];
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
    const std::string createFolder = "mkdir " + INPAINTING_HISTORY_FOLDER_NAME;
    system(createFolder.c_str());

    const glm::uvec2 windowSize = glm::uvec2(WINDOW_WIDTH, WINDOW_HEIGHT);
    imageResolution = glm::uvec2(imageWidth, imageHeight);

    pWindow = _pWindow;
    windowResolution = windowSize;
    imagePath = _imagePath;
    mouseControl = &_mouseControl;
    device = _device.get();
    physicalDevice = _device.getPhysicalDevice();
    Queue& transferQueue = _device.getTransferQueue();

    std::cout << "___ Initialization Phase ___ " << '\n';
    std::cout << "Threads: " << THREAD_NUMBER << '\n';

    for (uint16_t maskIndex = 0; maskIndex < MASKS_COUNT; maskIndex++) {
        selectedPosMasks[maskIndex].imageDetails.createImageInfo(
            "", imageWidth, imageHeight, 1,
            VK_IMAGE_LAYOUT_GENERAL,
            VK_IMAGE_VIEW_TYPE_2D,
            BUMP_TEXTURE_FORMAT, VK_SHADER_STAGE_FRAGMENT_BIT,
            VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_ASPECT_COLOR_BIT, VK_SAMPLE_COUNT_1_BIT);
        selectedPosMasks[maskIndex].create(device, physicalDevice, _commandPool,
                                     VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
                                     VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, transferQueue);
    }

    if (!callbackIsSet) {
        glfwSetMouseButtonCallback(pWindow, mouse_buttons_callback);
        glfwSetCursorPosCallback(pWindow, cursor_position_callback);
        callbackIsSet = true;
    }

    objectSelectionThread = std::thread(&ImageSegmantationSystem::runObjectSegmentationTask, *this);
}

void ImageSegmantationSystem::destroy()
{
    runningSegmentation = false;
    imageLoaded = false;
    if (objectSelectionThread.joinable()) {
        objectSelectionThread.join();
    }
    for (uint16_t maskIndex = 0; maskIndex < MASKS_COUNT; maskIndex++) {
        selectedPosMasks[maskIndex].destroy();
    }
    selectedPosMasks = std::array<Image, MASKS_COUNT>();
    runningSegmentation = true;
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
        selectedPosMasks[mouseControl->maskIndex].copyBufferToImage(transferQueue, selectedPosMask);
    }
}

/* Method inpaints chosen pixel mask with already existing image patches using Criminisi method.
   Firstly, area and side of square area will be found using mask countours. Center point will be 
   found using moments of pixel colors of the image that is used to align inpainted area to the center.
   To inpaint the image there will be used square area patch of the image instead of whole image to 
   speed up method excecution time for larger images.  */
void ImageSegmantationSystem::inpaintImage(uint8_t patchSize, std::vector<Image>& objectsTextures, ObjectParams& objectParams, Descriptor& descriptor, VkCommandPool& commandPool, Queue& transferQueue)
{
    std::vector<std::vector<cv::Point>> contours;
    cv::findContours(selectedPosMask, contours, cv::RETR_TREE, cv::CHAIN_APPROX_SIMPLE);
    uint32_t biggestArea = 0;
    for (auto contour : contours) {
        auto area = cv::contourArea(contour);
        if (biggestArea < area) {
            biggestArea = area;
        }
    }
    int side = cv::sqrt(biggestArea);
    cv::Moments m = cv::moments(selectedPosMask, true);
    cv::Point pointNearCenter(m.m10 / m.m00, m.m01 / m.m00);
    uint32_t negativeLength_x = glm::clamp(pointNearCenter.x - side , 0, image.cols);
    uint32_t negativeLength_y = glm::clamp(pointNearCenter.y - side, 0, image.rows);
    uint32_t positiveLength_x = glm::clamp(pointNearCenter.x + side, 0, image.cols);
    uint32_t positiveLength_y = glm::clamp(pointNearCenter.y + side, 0, image.rows);
    auto border_w = cv::Range(negativeLength_x, positiveLength_x);
    auto border_h = cv::Range(negativeLength_y, positiveLength_y);
    cv::Mat resisedImage = image(border_h, border_w);
    cv::Mat selectedPosMaskR = selectedPosMask(border_h, border_w);
    cv::Mat maskRectangle = selectedPosMaskR.clone();
    cv::rectangle(maskRectangle, cv::Point(patchSize), cv::Point(selectedPosMaskR.cols - patchSize, selectedPosMaskR.rows - patchSize), (0, 0, 0), -1);
    cv::bitwise_xor(selectedPosMaskR, maskRectangle, selectedPosMaskR);
    inpainter.setSourceImage(resisedImage);
    inpainter.setTargetMask(selectedPosMaskR);
    inpainter.setSourceMask(~selectedPosMaskR);
    inpainter.setPatchSize(patchSize);
    inpainter.initialize();

    while (inpainter.hasMoreSteps()) {
        inpainter.step();
    }

    cv::Mat resImage;
    cv::Mat inpaintedImage;
    inpainter.image().copyTo(inpaintedImage);
    uint32_t top = border_h.start;
    uint32_t bottom = image.rows - border_h.end;
    uint32_t left = border_w.start;
    uint32_t right = image.cols - border_w.end;
    cv::copyMakeBorder(inpaintedImage, resImage, top, bottom, left, right, cv::BorderTypes::BORDER_CONSTANT, 0);
    cv::copyTo(resImage, image, selectedPosMask);

    //TODO implement inpainted image history
    std::stringstream filePath;
    filePath << INPAINTING_HISTORY_FOLDER_NAME << "/" << "latest.png";
    std::vector<int> imageWriteParams;
    imageWriteParams.push_back(cv::IMWRITE_PNG_COMPRESSION);
    imageWriteParams.push_back(0);
    cv::imwrite(filePath.str(), image, imageWriteParams);
    latestImageTexture = image.clone();
    std::cout << "Background is inpainted" << '\n';

    Image::Details imageDetails = objectsTextures.front().getDetails();
    int w, h, channels;
    uchar* inpaintedTextureBuffer = stbi_load(filePath.str().c_str(), &w, &h, &channels, STBI_rgb_alpha);
    Image inpaintImage;
    inpaintImage.imageDetails.createImageInfo(
        "", imageDetails.width, imageDetails.height, imageDetails.channels,
        VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_VIEW_TYPE_2D,
        IMAGE_TEXTURE_FORMAT,
        VK_SHADER_STAGE_FRAGMENT_BIT | VK_SHADER_STAGE_COMPUTE_BIT,
        VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_ASPECT_COLOR_BIT,
        VK_SAMPLE_COUNT_1_BIT, inpaintedTextureBuffer, imageDetails.bindingId);
    inpaintImage.create(this->device, physicalDevice, commandPool,
        VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, transferQueue);
    objectsTextures.push_back(inpaintImage);

    uint16_t maxBinding = Image::bindingIdToImageArrayElementId[imageDetails.bindingId];
    descriptor.updateBindlessTexture(objectsTextures.back(), 0);
    uint16_t arrayElementId = 1;
    for (uint16_t i = maxBinding; i >= 1; i--) {
        descriptor.updateBindlessTexture(objectsTextures[maxBinding - i], arrayElementId++);
    }
    float objectMaxWidth = imageDetails.width - right - left;
    float width = glm::sqrt(objectMaxWidth + side);
    float objectMaxHeight = imageDetails.height - top - bottom;
    float hight = glm::sqrt(objectMaxHeight + side);
    glm::vec2 objectCenter = glm::vec2(pointNearCenter.x + width, pointNearCenter.y - hight);

    objectParams.position[0] = -0.5f * ((float)imageDetails.width / imageDetails.height);
    objectParams.position[1] = -0.5f;
}

bool& ImageSegmantationSystem::isImageLoaded() { return imageLoaded; }

uchar* ImageSegmantationSystem::getSelectedPositionsMask()
{
    return getSelectedPositionsMask(mouseControl->maskIndex);
}

uchar* ImageSegmantationSystem::getSelectedPositionsMask(uint16_t maskIndex)
{
   auto pixelSize = static_cast<size_t>(imageResolution.x) * static_cast<size_t>(imageResolution.y);
   auto selectedPositionsMask = static_cast<unsigned char*>(calloc(1, pixelSize));
    for (uint32_t width = 0; width < imageResolution.x; width++) {
        for (uint32_t height = 0; height < imageResolution.y; height++) {
            unsigned char* offset = selectedPositionsMask + width + (imageResolution.x * height);
            auto pos = glm::uvec2(width, height);
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
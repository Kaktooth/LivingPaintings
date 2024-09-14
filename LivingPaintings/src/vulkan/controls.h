#pragma once
#include "glm/glm.hpp"
#include <GLFW/glfw3.h>

class Controls {

public:
    struct MouseControl {
        glm::dvec2 mousePos;
        glm::uvec2 windowSize;
        float squareSize;
        bool pixelScaling = false;
    };

    void fillInMouseControlInfo(glm::uvec2 windowSize, float squareSize,
        GLFWwindow* pWindow);
    void updateMousePos(const glm::dvec2& mousePos);
    void updateWindowSize(const glm::uvec2& windowSize);
    void setPixelScaling(bool pixelScaling);
    bool getPixelScaling();
    MouseControl& getMouseControls();
};

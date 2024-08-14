#include "controls.h"

Controls::MouseControl mouseControl;

void key_callback(GLFWwindow* pWindow, int key, int scancode, int action,
                  int mods) {
  if (key == GLFW_KEY_I && action == GLFW_PRESS) {
    mouseControl.pixelScaling = !mouseControl.pixelScaling;
    if (mouseControl.pixelScaling) {
      glfwSetInputMode(pWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    } else {
      glfwSetInputMode(pWindow, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    }
  }
}

void Controls::fillInMouseControlInfo(glm::uvec2 windowSize, float squareSize,
                               GLFWwindow* pWindow) {
  mouseControl.windowSize = windowSize;
  mouseControl.squareSize = squareSize;
  glfwSetKeyCallback(pWindow, key_callback);
}

void Controls::updateMousePos(const glm::dvec2& mousePos) {
  mouseControl.mousePos = mousePos;
}

void Controls::updateWindowSize(const glm::uvec2& windowSize) {
  mouseControl.windowSize = windowSize;
}

void Controls::setPixelScaling(bool pixelScaling) {
  mouseControl.pixelScaling = pixelScaling;
}

bool Controls::getPixelScaling() { return mouseControl.pixelScaling; }

Controls::MouseControl& Controls::getMouseControls() 
{ 
    return mouseControl;
}
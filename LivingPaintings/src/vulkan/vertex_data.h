// This is a personal academic project. Dear PVS-Studio, please check it.

// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com

#pragma once

#include "gui_params.h"
#include "vulkan/vulkan.h"
#include <chrono>
// #define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/glm.hpp>
#include <vector>

namespace Data {
struct GraphicsObject {

    struct UniformBufferObject {
        glm::mat4 model;
        glm::mat4 view;
        glm::mat4 proj;

        void move(ObjectParams params);
        void rotate(ObjectParams params);
        void scale(ObjectParams params);
        void transform(ObjectParams params, AnimationParams animationParams);
        void move(ObjectParams params, float time);
        void rotate(ObjectParams params, float time);
        void scale(ObjectParams params, float time);
        void cameraView(CameraParams& params, VkExtent2D extent);
    } uniform;

    struct Vertex {
        glm::vec2 pos;
        glm::vec3 color;
        glm::vec2 texCoord;

        static VkVertexInputBindingDescription getBindingDescription();
        static std::vector<VkVertexInputAttributeDescription>
        getAttributeDescriptions();
    } vert;

    std::vector<Vertex> verticies;
    std::vector<uint16_t> indicies;

    void constructQuad();
};
} // namespace Data

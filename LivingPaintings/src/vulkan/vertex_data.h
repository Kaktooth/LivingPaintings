#pragma once

#include "../utils/alloc.h"

#include "consts.h"
#include "gui_params.h"
#include "vulkan/vulkan.h"
#include <algorithm>
#include <chrono>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/glm.hpp>
#include <glm/gtx/hash.hpp>
#include <queue>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace Data {

struct AlignmentProperties {
    static size_t minUniformAlignment;
    static size_t dynamicUniformAlignment_mat4;
};

struct RuntimeProperties {
    static size_t uboMemorySize;
    static float time;
};

struct GraphicsObject {
    static uint16_t s_instanceId;

    static struct Instance {
        static const glm::mat3 IDENTITY_MAT_3;
        static const glm::mat4 IDENTITY_MAT_4;

        glm::mat4* model { nullptr };

        void allocateInstances();
        void move(ObjectParams params, const glm::mat4 translationMatrix = IDENTITY_MAT_4);
        void rotate(ObjectParams params);
        void scale(ObjectParams params);
        void transform(GlobalAnimationParams globAnimParams, ObjectParams params, AnimationParams animationParams) const;
        void move(ObjectParams params, float time);
        void rotate(ObjectParams params, float time);
        void scale(ObjectParams params, float time);
        glm::mat4* translationMatrix(uint16_t instanceIndex) const;
        void destroy();
    } instanceUniform;

    static struct View {
        glm::mat4 view;
        glm::mat4 proj;

        void cameraView(CameraParams& params, VkExtent2D extent);
    } viewUniform;

    struct Vertex {
        glm::vec3 pos;
        glm::vec2 texCoord;

        bool operator>(const Vertex& other) const { return pos == other.pos; }

        static VkVertexInputBindingDescription getBindingDescription();
        static std::vector<VkVertexInputAttributeDescription>
        getAttributeDescriptions();
    };

    uint16_t instanceId = s_instanceId++;
    std::vector<Vertex> vertices;
    std::vector<uint16_t> indices;

    void constructQuad();
    void constructQuadWithAspectRatio(uint16_t width, uint16_t height,
        float depth);
    void constructQuadsWithAspectRatio(uint16_t width, uint16_t height);
    void constructMeshFromTexture(uint16_t width, uint16_t height,
        float selectedDepth, const unsigned char* pixels,
        uint16_t alphaPercentage);
};

void setUniformDynamicAlignments(size_t minUboAlignment);
} // namespace Data

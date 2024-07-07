// This is a personal academic project. Dear PVS-Studio, please check it.

// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com

#include "vertex_data.h"

using namespace std;
using std::chrono::steady_clock;

VkVertexInputBindingDescription Data::GraphicsObject::Vertex::getBindingDescription()
{
    VkVertexInputBindingDescription bindingDescription {};
    bindingDescription.binding = 0;
    bindingDescription.stride = sizeof(Vertex);
    bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
    return bindingDescription;
}

vector<VkVertexInputAttributeDescription> Data::GraphicsObject::Vertex::getAttributeDescriptions()
{
    vector<VkVertexInputAttributeDescription> attributeDescriptions(3);

    attributeDescriptions[0].binding = 0;
    attributeDescriptions[0].location = 0;
    attributeDescriptions[0].format = VK_FORMAT_R32G32_SFLOAT;
    attributeDescriptions[0].offset = offsetof(Vertex, pos);

    attributeDescriptions[1].binding = 0;
    attributeDescriptions[1].location = 1;
    attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
    attributeDescriptions[1].offset = offsetof(Vertex, color);

    attributeDescriptions[2].binding = 0;
    attributeDescriptions[2].location = 2;
    attributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
    attributeDescriptions[2].offset = offsetof(Vertex, texCoord);

    return attributeDescriptions;
}

void Data::GraphicsObject::UniformBufferObject::move(ObjectParams params)
{
    model = glm::translate(glm::mat4(1.0f), glm::vec3(params.position[0], params.position[1], params.position[2]));
}

void Data::GraphicsObject::UniformBufferObject::rotate(ObjectParams params)
{
    model = glm::rotate(model, glm::radians(params.rotation[0]), glm::vec3(1.0f, 0.0f, 0.0f));
    model = glm::rotate(model, glm::radians(params.rotation[1]), glm::vec3(0.0f, 1.0f, 0.0f));
    model = glm::rotate(model, glm::radians(params.rotation[2]), glm::vec3(0.0f, 0.0f, 1.0f));
}

void Data::GraphicsObject::UniformBufferObject::scale(ObjectParams params)
{
    model = glm::scale(model, glm::vec3(params.scale[0], params.scale[1], params.scale[2]));
}

void Data::GraphicsObject::UniformBufferObject::transform(
    ObjectParams params, AnimationParams animationParams)
{
    if (animationParams.play) {
        static auto startTime = chrono::high_resolution_clock::now();

        auto currentTime = chrono::high_resolution_clock::now();
        float duration_ms = chrono::duration<float, chrono::milliseconds::period>(currentTime - startTime).count();

        if (duration_ms > animationParams.end_ms) {
            startTime = currentTime;
        }

        // get normilized time in percentage for compliting object transformation in animation
        animationParams.t = (duration_ms - animationParams.start_ms) / (animationParams.end_ms - animationParams.start_ms);

        float equation = 0;
        if (animationParams.useEasingFunction) {
            if (animationParams.selectedEasingEquation == 0) {
                equation = 1 - glm::cos((animationParams.t * glm::pi<float>()) / 2);
            } else if (animationParams.selectedEasingEquation == 1) {
                equation = glm::sin((animationParams.t * glm::pi<float>()) / 2);
            } else if (animationParams.selectedEasingEquation == 2) {
                equation = -(glm::cos(animationParams.t * glm::pi<float>()) - 1) / 2;
            }

            animationParams.tranformModifier = lerp(0, 1, equation);
        } else {
            animationParams.tranformModifier = animationParams.t;
        }

        if (animationParams.play_ms >= animationParams.end_ms) {
            animationParams.play_ms = animationParams.start_ms;
        }

        animationParams.play_ms += duration_ms;

        if (animationParams.start_ms <= animationParams.play_ms && animationParams.play_ms <= animationParams.end_ms) {
            // use normilized time for object transformations
            move(params, animationParams.tranformModifier);
            rotate(params, animationParams.tranformModifier);
            /*  scale(params, animationParams.tranformModifier);*/
        }
    }
}

void Data::GraphicsObject::UniformBufferObject::move(ObjectParams params,
    float time)
{
    model = glm::translate(model, time * glm::vec3(params.position[0], params.position[1], params.position[2]));
}

void Data::GraphicsObject::UniformBufferObject::rotate(ObjectParams params,
    float time)
{
    model = glm::rotate(model, time * glm::radians(params.rotation[0]), glm::vec3(1.0f, 0.0f, 0.0f));
    model = glm::rotate(model, time * glm::radians(params.rotation[1]), glm::vec3(0.0f, 1.0f, 0.0f));
    model = glm::rotate(model, time * glm::radians(params.rotation[2]), glm::vec3(0.0f, 0.0f, 1.0f));
}

void Data::GraphicsObject::UniformBufferObject::scale(ObjectParams params,
    float time)
{
    model = glm::scale(model, time * glm::vec3(params.scale[0], params.scale[1], params.scale[2]));
}

void Data::GraphicsObject::UniformBufferObject::cameraView(CameraParams& params, VkExtent2D extent)
{
    if (params.lookMode) {
        view = glm::lookAt(glm::vec3(params.cameraPos[0], params.cameraPos[1], params.cameraPos[2]),
            glm::vec3(params.cameraTarget[0], params.cameraTarget[1], params.cameraTarget[2]),
            glm::vec3(params.upVector[0], params.upVector[1], params.upVector[2]));
    } else {
        auto cameraPos = glm::vec3(params.cameraPos[0], params.cameraPos[1], params.cameraPos[2]);
        float dist = glm::distance(cameraPos, glm::vec3(0, 0, 0));
        view = glm::lookAt(cameraPos, glm::vec3(dist, 0, dist),
            glm::vec3(params.upVector[0], params.upVector[1], params.upVector[2]));
    }

    auto aspectRatio = extent.width / (float)extent.height;
    if (params.perspectiveMode) {
        proj = glm::perspective(glm::radians(params.fieldOfView), aspectRatio, params.nearClippingPlane, params.farClippingPlane);
    } else {
        proj = glm::ortho(-params.orthoSize, params.orthoSize, -params.orthoSize, params.orthoSize, params.nearClippingPlane, params.farClippingPlane);
    }
    proj[1][1] *= -1;
}

void Data::GraphicsObject::constructQuad()
{
    verticies = { { { -0.5f, -0.5f }, { 1.0f, 1.0f, 0.0f }, { 1.0f, 0.0f } },
        { { 0.5f, -0.5f }, { 0.0f, 1.0f, 1.0f }, { 0.0f, 0.0f } },
        { { 0.5f, 0.5f }, { 0.0f, 1.0f, 0.0f }, { 0.0f, 1.0f } },
        { { -0.5f, 0.5f }, { 1.0f, 0.0f, 1.0f }, { 1.0f, 1.0f } } };

    indicies = { 0, 1, 2, 2, 3, 0 };
}

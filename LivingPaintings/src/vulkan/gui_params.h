#include "consts.h"
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/glm.hpp>
#include <glm/gtx/hash.hpp>
#include <string>
#include <vector>
#ifndef GUI_PARAMS_H
#define GUI_PARAMS_H

using Constants::EFFECTS_COUNT;

static const uint16_t EFFECTS_ENABLED_SIZE = (EFFECTS_COUNT + 3) / 4;

struct ObjectParams {
    uint16_t index = 0;
    float position[3];
    float rotation[3];
    float scale[3];
};

struct AnimationParams {
    uint16_t objIndex;
    bool play;
    float play_ms;
    float start_ms;
    float end_ms;
    float t; // time percentage parameter that specifies animation completion
    float tranformModifier;
    bool useEasingFunction;
    std::vector<std::string> easingEquations;
    int selectedEasingEquation;
};

struct GlobalAnimationParams {
    float start_ms;
    float end_ms;
    bool showObjectPosStart;
};

struct EffectParams
{
    glm::uvec4 enabledEffects[EFFECTS_ENABLED_SIZE]; // boolean values packed into uvec4
    unsigned int highlightSelectedPixels; // uint aligned to 16 bytes
    float heightRange;
    float noiseScale;
    float distortionModifier;
    float parallaxHeightScale;
    float amplifyFlickeringLight;
    float amplifyHighlight;
};

struct CameraParams {
    float cameraPos[3];
    float cameraTarget[3];
    float upVector[3];
    bool perspectiveMode;
    bool lookMode;
    float fieldOfView;
    float nearClippingPlane;
    float farClippingPlane;
    float orthoSize;
};

//Todo LightParams

struct SpecificDrawParams {
    size_t pipelineHistorySize;
    bool imageLoaded;
    bool constructSelectedObject;
    bool clearSelectedMask;
};

struct ObjectConstructionParams {
    /* Alpha value is a square radius by which triangles will be removed
       near specified points of selected object. */
    int alphaPercentage;
};

struct MouseControlParams
{
    int maskIndex;
};

struct InpaintingParams {
    bool enableInpainting;
    int patchSize;
};
#endif

#include <string>
#include <vector>
#ifndef GUI_PARAMS_H
#define GUI_PARAMS_H

struct ObjectParams {
    uint16_t index = 0;
    float position[3];
    float rotation[3];
    float scale[3];
};

struct AnimationParams {
    bool play;
    float play_ms;
    float start_ms;
    float end_ms;
    float t; // percentage parameter that specifies animation completion
    float tranformModifier; // interpolated or not interpolated value
    bool useEasingFunction;
    std::vector<std::string> easingEquations;
    int selectedEasingEquation;
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

struct SpecificDrawParams {
    size_t pipelineHistorySize;
    bool imageLoaded;
    bool constructSelectedObject;
};

struct ObjectConstructionParams {
    /* Alpha value is a square radius by which triangles will be removed
       near specified points of selected object. */
    int alphaPercentage;
};
#endif

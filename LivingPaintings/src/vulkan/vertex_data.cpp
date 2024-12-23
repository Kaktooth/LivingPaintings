#include "vertex_data.h"
#include "consts.h"

#include <CGAL/Alpha_shape_2.h>
#include <CGAL/Alpha_shape_face_base_2.h>
#include <CGAL/Alpha_shape_vertex_base_2.h>
#include <CGAL/Delaunay_triangulation_2.h>
#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/algorithm.h>

#include <algorithm>
#include <cmath>
#include <fstream>
#include <functional>
#include <iostream>
#include <list>
#include <unordered_map>
#include <unordered_set>
#include <vector>

using std::chrono::steady_clock;

typedef std::chrono::duration<float, std::chrono::milliseconds::period> duration_ms;

typedef CGAL::Exact_predicates_inexact_constructions_kernel K;

typedef K::FT FT;
typedef K::Point_2 Point;
typedef K::Segment_2 Segment;
typedef CGAL::Alpha_shape_vertex_base_2<K> Vb;
typedef CGAL::Alpha_shape_face_base_2<K> Fb;
typedef CGAL::Triangulation_data_structure_2<Vb, Fb> Tds;
typedef CGAL::Delaunay_triangulation_2<K, Tds> Triangulation_2;
typedef CGAL::Alpha_shape_2<Triangulation_2> Alpha_shape_2;
typedef Alpha_shape_2::All_faces_iterator Alpha_shape_faces_iterator;
typedef Alpha_shape_2::Alpha_shape_edges_iterator Alpha_shape_edges_iterator;

const glm::mat3 Data::GraphicsObject::Instance::IDENTITY_MAT_3 = glm::identity<glm::mat3>();
const glm::mat4 Data::GraphicsObject::Instance::IDENTITY_MAT_4 = glm::identity<glm::mat4>();

const std::map<uint16_t, uint8_t> CHECKED_REGIONS_TO_POINT_GRANULARITY {
    { 1000, 50 }, { 300, 20 }, { 150, 15 }, { 100, 10 }, { 80, 5 }, { 40, 4 }
};

size_t Data::AlignmentProperties::minUniformAlignment = 0;
size_t Data::AlignmentProperties::dynamicUniformAlignment_mat4 = 0;

size_t Data::RuntimeProperties::uboMemorySize = 0;

uint16_t Data::GraphicsObject::s_instanceId = 0;
Data::GraphicsObject::Instance Data::GraphicsObject::instanceUniform {};
Data::GraphicsObject::View Data::GraphicsObject::viewUniform {};

VkVertexInputBindingDescription
Data::GraphicsObject::Vertex::getBindingDescription()
{
    VkVertexInputBindingDescription bindingDescription {};
    bindingDescription.binding = 0;
    bindingDescription.stride = sizeof(Vertex);
    bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
    return bindingDescription;
}

std::vector<VkVertexInputAttributeDescription>
Data::GraphicsObject::Vertex::getAttributeDescriptions()
{
    std::vector<VkVertexInputAttributeDescription> attributeDescriptions(2);

    attributeDescriptions[0].binding = 0;
    attributeDescriptions[0].location = 0;
    attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
    attributeDescriptions[0].offset = offsetof(Vertex, pos);

    attributeDescriptions[1].binding = 0;
    attributeDescriptions[1].location = 1;
    attributeDescriptions[1].format = VK_FORMAT_R32G32_SFLOAT;
    attributeDescriptions[1].offset = offsetof(Vertex, texCoord);

    return attributeDescriptions;
}

void Data::GraphicsObject::Instance::allocateInstances()
{
    RuntimeProperties::uboMemorySize = Constants::OBJECT_INSTANCES * AlignmentProperties::dynamicUniformAlignment_mat4;
    model = (glm::mat4*)alignedAlloc(RuntimeProperties::uboMemorySize, AlignmentProperties::dynamicUniformAlignment_mat4);
}

void Data::GraphicsObject::Instance::move(ObjectParams params, glm::mat4 mat)
{
    glm::mat4* modelMat = translationMatrix(params.index);
    *modelMat = glm::translate(mat, { params.position[0], params.position[1], params.position[2] });
}

void Data::GraphicsObject::Instance::rotate(ObjectParams params)
{
    glm::mat4* modelMat = translationMatrix(params.index);
    *modelMat = glm::rotate(*modelMat, glm::radians(params.rotation[0]), IDENTITY_MAT_3[0]);
    *modelMat = glm::rotate(*modelMat, glm::radians(params.rotation[1]), IDENTITY_MAT_3[1]);
    *modelMat = glm::rotate(*modelMat, glm::radians(params.rotation[2]), IDENTITY_MAT_3[2]);
}

void Data::GraphicsObject::Instance::scale(ObjectParams params)
{
    glm::mat4* modelMat = translationMatrix(params.index);
    *modelMat = glm::scale(*modelMat, glm::vec3(params.scale[0], params.scale[1], params.scale[2]));
}

void Data::GraphicsObject::Instance::transform(GlobalAnimationParams globAnimParams, ObjectParams params, AnimationParams animationParams) const
{
    static steady_clock::time_point startTime = std::chrono::steady_clock::now();

    steady_clock::time_point currentTime = std::chrono::steady_clock::now();
    float duration = duration_ms(currentTime - startTime).count();

    if (animationParams.play) {
        if (duration > globAnimParams.end_ms) {
            startTime = currentTime;
        }
    }

    // Get normilized time in percentage for compliting object transformation in animation.
    animationParams.t = (duration - animationParams.start_ms) / (animationParams.end_ms - animationParams.start_ms);

    float equation = 0;
    if (animationParams.useEasingFunction) {
        if (animationParams.selectedEasingEquation == 0) {
            equation = 1 - glm::cos((animationParams.t * glm::pi<float>()) / 2);
        }
        else if (animationParams.selectedEasingEquation == 1) {
            equation = glm::sin((animationParams.t * glm::pi<float>()) / 2);
        }
        else if (animationParams.selectedEasingEquation == 2) {
            equation = -(glm::cos(animationParams.t * glm::pi<float>()) - 1) / 2;
        }

        animationParams.tranformModifier = std::lerp(0, 1, equation);
    }
    else {
        animationParams.tranformModifier = animationParams.t;
    }

    animationParams.play_ms += duration;

    if (animationParams.start_ms <= animationParams.play_ms && animationParams.play_ms <= animationParams.end_ms) {
        // Use normilized time for object transformations
        instanceUniform.move(params, animationParams.tranformModifier);
        instanceUniform.rotate(params, animationParams.tranformModifier);
        /*  scale(params, animationParams.tranformModifier);*/
    }
    else if (animationParams.play_ms > animationParams.end_ms && globAnimParams.showObjectPosStart) {
        instanceUniform.move(params, 1.0f);
        instanceUniform.rotate(params, 1.0f);
    }
}

void Data::GraphicsObject::Instance::move(ObjectParams params, float time)
{
    glm::mat4* modelMat = translationMatrix(params.index);
    *modelMat = glm::translate(*modelMat, time * glm::vec3(params.position[0], params.position[1], params.position[2]));
}

void Data::GraphicsObject::Instance::rotate(ObjectParams params, float time)
{
    glm::mat4* modelMat = translationMatrix(params.index);
    *modelMat = glm::rotate(*modelMat, time * glm::radians(params.rotation[0]), IDENTITY_MAT_3[0]);
    *modelMat = glm::rotate(*modelMat, time * glm::radians(params.rotation[1]), IDENTITY_MAT_3[1]);
    *modelMat = glm::rotate(*modelMat, time * glm::radians(params.rotation[2]), IDENTITY_MAT_3[2]);
}

void Data::GraphicsObject::Instance::scale(ObjectParams params, float time)
{
    glm::mat4* modelMat = translationMatrix(params.index);
    *modelMat = glm::scale(*modelMat, time * glm::vec3(params.scale[0], params.scale[1], params.scale[2]));
}

glm::mat4* Data::GraphicsObject::Instance::translationMatrix(uint16_t instanceIndex) const
{
    uint64_t mstartaddr = reinterpret_cast<uint64_t>(model);
    size_t uboOffset = instanceIndex * AlignmentProperties::dynamicUniformAlignment_mat4;
    glm::mat4* modelMat = reinterpret_cast<glm::mat4*>(mstartaddr + uboOffset);
    return modelMat;
}

void Data::GraphicsObject::Instance::destroy()
{
    alignedFree(instanceUniform.model);
}

void Data::GraphicsObject::View::cameraView(CameraParams& params,
    VkExtent2D extent)
{
    if (params.lookMode) {
        view = glm::lookAt(
            glm::vec3(params.cameraPos[0], params.cameraPos[1],
                params.cameraPos[2]),
            glm::vec3(params.cameraTarget[0], params.cameraTarget[1],
                params.cameraTarget[2]),
            glm::vec3(params.upVector[0], params.upVector[1], params.upVector[2]));
    } else {
        glm::vec3 cameraPos = glm::vec3(params.cameraPos[0], params.cameraPos[1],
            params.cameraPos[2]);
        float dist = glm::distance(cameraPos, glm::vec3(0.0f));
        view = glm::lookAt(
            cameraPos, glm::vec3(dist, 0, dist),
            glm::vec3(params.upVector[0], params.upVector[1], params.upVector[2]));
    }

    auto aspectRatio = static_cast<float>(extent.width) / static_cast<float>(extent.height);
    if (params.perspectiveMode) {
        proj = glm::perspective(glm::radians(params.fieldOfView), aspectRatio,
            params.nearClippingPlane, params.farClippingPlane);
    } else {
        proj = glm::ortho(-params.orthoSize, params.orthoSize, -params.orthoSize,
            params.orthoSize, params.nearClippingPlane,
            params.farClippingPlane);
    }
    proj[1][1] *= -1;
}

void Data::GraphicsObject::constructQuad()
{
    vertices = { { { -0.5f, -0.5f, 0.0f }, { 1.0f, 0.0f } },
        { { 0.5f, -0.5f, 0.0f }, { 0.0f, 0.0f } },
        { { 0.5f, 0.5f, 0.0f }, { 0.0f, 1.0f } },
        { { -0.5f, 0.5f, 0.0f }, { 1.0f, 1.0f } } };

    indices = { 0, 1, 2, 2, 3, 0 };
}

void Data::GraphicsObject::constructQuadWithAspectRatio(uint16_t width, uint16_t height,
    float depth)
{
    // Find ratio to calculate verticies position.
    float ratio = float(width) / height;

    vertices = { { { -0.5f * ratio, -0.5f, depth }, { 1.0f, 0.0f } },
        { { 0.5f * ratio, -0.5f, depth }, { 0.0f, 0.0f } },
        { { 0.5f * ratio, 0.5f, depth }, { 0.0f, 1.0f } },
        { { -0.5f * ratio, 0.5f, depth }, { 1.0f, 1.0f } } };

    indices = { 0, 1, 2, 2, 3, 0 };
}

void Data::GraphicsObject::constructQuadsWithAspectRatio(uint16_t width,
    uint16_t height)
{
    float ratio = float(width) / height;

    vertices = { { { -0.5f * ratio, -0.5f, 0.0f }, { 1.0f, 0.0f } },
        { { 0.5f * ratio, -0.5f, 0.0f }, { 0.0f, 0.0f } },
        { { 0.5f * ratio, 0.5f, 0.0f }, { 0.0f, 1.0f } },
        { { -0.5f * ratio, 0.5f, 0.0f }, { 1.0f, 1.0f } },

        { { -0.5f, -0.5f, -0.5f }, { 1.0f, 0.0f } },
        { { 0.5f, -0.5f, -0.5f }, { 0.0f, 0.0f } },
        { { 0.5f, 0.5f, -0.5f }, { 0.0f, 1.0f } },
        { { -0.5f, 0.5f, -0.5f }, { 1.0f, 1.0f } } };

    indices = { 0, 1, 2, 2, 3, 0, 4, 5, 6, 6, 7, 4 };
}

/* Set uniform dynamic properties for dynamic buffers.
   minUboAlignment - parameter must be taken from device properties.
   */
void Data::setUniformDynamicAlignments(size_t minUboAlignment)
{
    AlignmentProperties::minUniformAlignment = minUboAlignment;
    AlignmentProperties::dynamicUniformAlignment_mat4 = sizeof(glm::mat4);
    if (minUboAlignment > 0) {
        AlignmentProperties::dynamicUniformAlignment_mat4 = (AlignmentProperties::dynamicUniformAlignment_mat4 + minUboAlignment - 1) & ~(minUboAlignment - 1);
    }
}

/* Constructs mesh from mask texture by gathering points from mask texture and using Alpha shape method to build
   mesh from specified points. When gathering point, point granularity in the mask texture is checked. In accordance
   with point granularity, points will be added to array if they was selected and they dont have all neighbouring
   pixels selected. Next use points for calculating Alpha shape and push indicies of face with valid alpha parameter.
   Implementation of the CGAL library for the Alpha form is used. https://doc.cgal.org/latest/Alpha_shapes_2/index.html
   width and height - specify resolution of texture.
   selectedDepth - used for creating mesh for specified depth.
   pixels - mask texture that is used for selected objects.
   alpha - value (1 - 10000) that used to calculate alpha parameter for Alpha shape method.
   */
void Data::GraphicsObject::constructMeshFromTexture(uint16_t width, uint16_t height,
    float selectedDepth, const unsigned char* pixels, uint16_t alpha)
{
    float ratio = float(width) / height;
    double alphaPercentage = static_cast<double>(alpha) / 10000;
    std::unordered_map<glm::vec2, uint16_t> verticesToIndices {};
    uint16_t updatePointGranularityRate = 20;
    uint16_t pointGranularity = 5;
    uint32_t regionsChecked = 0;
    int foundRegionIndex = -1;
    bool vertexSizeFounded = false;

    for (int w = 0; w < width; w++) {
        regionsChecked = 0;
        if (w % updatePointGranularityRate == 0) {
            pointGranularity = CHECKED_REGIONS_TO_POINT_GRANULARITY.upper_bound(regionsChecked)->second;
        }

        for (int h = 0; h < height; h++) {
            // Find highlighted regions of the image and gather points from the mask.
            const unsigned char* pixelOffset = pixels + w + (width * h);
            bool regionFound = pixelOffset[0] == Constants::SELECTED_REGION_HIGHLIGHT;
            if (regionFound) {
                uint32_t rightOffset = w + 1 <= width ? w + 1 : w;
                uint32_t upOffset = h + 1 < height ? h + 1 : h;
                uint32_t leftOffset = w - 1 >= 0 ? w - 1 : w;
                uint32_t downOffset = h - 1 > 0 ? h - 1 : h;
                const unsigned char* rightPixel = pixels + rightOffset + (width * h);
                const unsigned char* highPixel = pixels + w + (width * upOffset);
                const unsigned char* leftPixel = pixels + leftOffset + (width * h);
                const unsigned char* lowPixel = pixels + w + (width * downOffset);
                bool rightRegionExists = rightPixel[0] == Constants::SELECTED_REGION_HIGHLIGHT;
                bool upRegionExists = highPixel[0] == Constants::SELECTED_REGION_HIGHLIGHT;
                bool leftRegionExists = leftPixel[0] == Constants::SELECTED_REGION_HIGHLIGHT;
                bool downRegionExists = lowPixel[0] == Constants::SELECTED_REGION_HIGHLIGHT;
                regionsChecked++;

                bool edgeDirectionsValid = rightRegionExists && upRegionExists && leftRegionExists && downRegionExists;
                bool objectEdgeFound = edgeDirectionsValid && foundRegionIndex != -1;
                bool pointGranularityComplies = (w % pointGranularity == 0 && h % pointGranularity == 0);
                if (objectEdgeFound && !pointGranularityComplies) {
                    continue;
                }

                float vertPosX = static_cast<float>(w) / width;
                float vertPosY = static_cast<float>(h) / height;

                size_t lastSize = vertices.size();
                size_t nextSize = lastSize + 1;
                glm::vec3 pos = { (0.5f - vertPosX) * ratio, vertPosY - 0.5f, selectedDepth };
                vertices.resize(nextSize);
                vertices[lastSize] = { pos, { vertPosX, vertPosY } };
                verticesToIndices.insert({ glm::vec2(pos), lastSize });

                foundRegionIndex = w;
            } else {
                foundRegionIndex = -1;
            }
        }
        if (w == 0 && !vertexSizeFounded) {
            vertexSizeFounded = true;
            w--;
        }
        foundRegionIndex = -1;
    }

    std::list<Point> points;
    for (const Vertex& vertex : vertices) {
        points.push_back(Point(vertex.pos.x, vertex.pos.y));
    }

    Alpha_shape_2 alphaShape(points.begin(), points.end(), FT(10000), Alpha_shape_2::GENERAL);

    for (Alpha_shape_faces_iterator it = alphaShape.all_faces_begin(); it != alphaShape.all_faces_end(); ++it) {
        double faceAlpha = it->get_alpha();
        if (faceAlpha < alphaPercentage) {
            Point p1 = it->vertex(0)->point();
            Point p2 = it->vertex(1)->point();
            Point p3 = it->vertex(2)->point();

            bool verticesExists = verticesToIndices.find(glm::vec2(p1.x(), p1.y())) != verticesToIndices.end() 
                && verticesToIndices.find(glm::vec2(p2.x(), p2.y())) != verticesToIndices.end() 
                && verticesToIndices.find(glm::vec2(p3.x(), p3.y())) != verticesToIndices.end();

            if (verticesExists) {
                indices.push_back(verticesToIndices.at(glm::vec2(p1.x(), p1.y())));
                indices.push_back(verticesToIndices.at(glm::vec2(p2.x(), p2.y())));
                indices.push_back(verticesToIndices.at(glm::vec2(p3.x(), p3.y())));
            }
        }
    }
}

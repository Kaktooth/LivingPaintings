#pragma once

#include "../segmentation/segmentation_system.h"
#include "command_buffer.h"
#include "consts.h"
#include "device.h"
#include "gui_params.h"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_vulkan.h"
#include "queue_family.h"
#include "render_pass.h"
#include "swapchain.h"
#include "vulkan/vulkan.h"
#include <stdexcept>
#include <algorithm>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include "../vulkan/render_pass.h"
#include "../vulkan/swapchain.h"

using Constants::MASKS_COUNT;
using Constants::DEFAULT_PATCH_SIZE;
using Constants::EXPORT_FRAME_COUNT;

class Gui {

	ObjectParams objectParams = {
		0,
		{ 0.0f, 0.0f, 0.0f },
		{ 0.0f, 0.0f, 0.0f },
		{ 1.0f, 1.0f, 1.0f } // initial object scale
	};

	AnimationParams animationParams = {
		0,
		false,
		0.0f,
		0.0f,
		0.0f,
		0.0f,
		0.0f,
		false,
		{ "sineIn", "sineOut", "sineInOut" },
		0
	};

	GlobalAnimationParams globalAnimationParams = {
		0.0f,
		0.0f,
		true
	};

	CameraParams cameraParams = {
		{ 45.0f, 0.0f, 45.0f }, // init for orthogonal projection
		{ 0.0f, 0.0f, 0.0f },
		{ 0.0f, -1.0f, 0.0f }, // camera should look down
		false, // use orthogonal projection
		true,
		50.0f,
		0.1f,
		100.0f,
		0.5f
	};

	EffectParams effectsParams = {
		{},
		true,
		1.0f,
		50.0f,
		0.005f,
		0.003f,
		0.4f,
		0.2f
	};

	LightParams lightParams = {
		{0.0f, 10.0f, 5.0f},
		0.35f
	};

	ObjectConstructionParams objectConstructionParams = {
		1
	};

	MouseControlParams mouseControlParams = {
		0 // 0 to use object selection mask, other non-negative number select masks for effects.
	};

	InpaintingParams inpaintingParams = {
		true,
		DEFAULT_PATCH_SIZE
	};

	size_t selectedPipelineIndex = 0;

	VkDevice device = VK_NULL_HANDLE;
	VkCommandPool commandPool = VK_NULL_HANDLE;

	void uploadFonts(Queue queue);

public:
	std::vector<ObjectParams> objectsParams{ objectParams };
	std::vector<ObjectParams> objectsAnimationParams{ objectParams };
	std::vector<AnimationParams> animationControlParams{ animationParams };

	SpecificDrawParams drawParams = { 1, false, false, false, false, "", EXPORT_FRAME_COUNT };

	uint16_t animIndex = 0;
	uint16_t objIndex = 0;

	void init(VkInstance& instance, Device& device, VkCommandPool& commandPool,
		RenderPass& renderPass, Swapchain& swapChain,
		VkDescriptorPool& descriptorPool, GLFWwindow* window);
	void ShowEventsOverlay(bool* p_open) const;
	void ShowControls(bool* p_open);
	void draw();
	void renderDrawData(VkCommandBuffer& commandBuffer);
	void createGraphicsObjectParams(uint16_t objIndex);
	void createObjectAnimationParams(uint16_t objIndex);
	void updateGlobalAnimationParams();
	void destroy();
	ObjectParams getObjectParams();
	AnimationParams getAnimationParams();
	GlobalAnimationParams& getGlobalAnimationParams();
	CameraParams& getCameraParams();
	EffectParams& getEffectParams();
	LightParams& getLightParams();
	ObjectConstructionParams& getObjectConstructionParams();
	MouseControlParams& getMouseControlParams();
	InpaintingParams& getInpaintingParams();
	size_t getSelectedPipelineIndex() const;
	void selectPipelineindex(const size_t pipelineIndex);
};

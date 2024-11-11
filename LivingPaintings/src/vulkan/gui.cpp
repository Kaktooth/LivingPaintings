#include "gui.h"

using Constants::APP_NAME;
using Constants::MAX_FRAMES_IN_FLIGHT;
using Constants::EFFECTS_COUNT;

const float PAD = 10.0f;
const ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoMove;

void Gui::init(VkInstance& instance, Device& _device, VkCommandPool& commandPool, RenderPass& renderPass, Swapchain& swapChain, VkDescriptorPool& descriptorPool, GLFWwindow* pWindow)
{
    for (uint16_t maskIndex = 0; maskIndex < EFFECTS_ENABLED_SIZE; maskIndex++) {
        effectsParams.enabledEffects[maskIndex] = glm::uvec4(1);
    }

    Queue& graphicsQueue = _device.getGraphicsQueue();

    this->device = _device.get();
    this->commandPool = commandPool;

    ImGui::CreateContext();

    ImGui_ImplGlfw_InitForVulkan(pWindow, true);

    ImGui_ImplVulkan_InitInfo initInfo {};
    initInfo.Instance = instance;
    initInfo.PhysicalDevice = _device.getPhysicalDevice();
    initInfo.Device = device;
    initInfo.QueueFamily = _device.getQueueFamily().indicies.graphicsFamily.value();
    initInfo.Queue = graphicsQueue.get();
    initInfo.DescriptorPool = descriptorPool;
    initInfo.MinImageCount = swapChain.getMinImageCount();
    initInfo.ImageCount = MAX_FRAMES_IN_FLIGHT;
    initInfo.MSAASamples = renderPass.getSampleCount();
    initInfo.ColorAttachmentFormat = swapChain.getImageFormat();

    ImGui_ImplVulkan_Init(&initInfo, renderPass.get());

    uploadFonts(graphicsQueue);
}

void Gui::uploadFonts(Queue queue)
{
    VkCommandBuffer cmd = CommandBuffer::beginSingleTimeCommands(device, commandPool);
    ImGui_ImplVulkan_CreateFontsTexture(cmd);
    CommandBuffer::endSingleTimeCommands(device, commandPool, cmd, queue);
    vkDeviceWaitIdle(device);
    ImGui_ImplVulkan_DestroyFontUploadObjects();
}

void Gui::ShowEventsOverlay(bool* p_open) const
{
    const ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImVec2 work_pos = viewport->WorkPos;
    ImVec2 work_size = viewport->WorkSize;
    ImVec2 window_pos, window_pos_pivot;
    window_pos.x = work_pos.x + PAD;
    window_pos.y = work_pos.y + work_size.y - PAD;
    window_pos_pivot = ImVec2(0.0f, 1.0f);
    ImGui::SetNextWindowPos(window_pos, ImGuiCond_Always, window_pos_pivot);
    ImGui::SetNextWindowViewport(viewport->ID);

    ImGui::SetNextWindowBgAlpha(0.35f);
    if (ImGui::Begin("Events", p_open, window_flags)) {
        if (drawParams.imageLoaded) {
            ImGui::Text("Image is loaded!");
        } else {
            ImGui::Text("Loading image for segmentation... Cant select objects right now.");
        }
    }

    if (ImGui::BeginPopupContextWindow()) {
        if (p_open && ImGui::MenuItem("Close")) {
            *p_open = false;
        }
        ImGui::EndPopup();
    }

    ImGui::End();
}

void Gui::ShowControls(bool* p_open)
{
    const ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImVec2 work_pos = viewport->WorkPos;
    ImVec2 window_pos, window_pos_pivot;
    window_pos.x = work_pos.x + PAD;
    window_pos.y = work_pos.y + PAD;
    window_pos_pivot = ImVec2();
    ImGui::SetNextWindowPos(window_pos, ImGuiCond_Always, window_pos_pivot);
    ImGui::SetNextWindowViewport(viewport->ID);

    ImGui::SetNextWindowBgAlpha(0.35f);
    if (ImGui::Begin("Controls", p_open, window_flags)) {
        ImGui::Text("Constrols: ");
        ImGui::Separator;
        ImGui::Text("Ctrl + Left Mouse Click: Select object region");
        ImGui::Text("Ctrl + Right Mouse Click: Unselect object region");
        ImGui::Text("I: Zoom in. In zoom in state hold mouse button to select(or unselect) the pixels and release the button when done.");
    }

    if (ImGui::BeginPopupContextWindow()) {
        if (p_open && ImGui::MenuItem("Close")) {
            *p_open = false;
        }
        ImGui::EndPopup();
    }

    ImGui::End();
}

void Gui::draw()
{
    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
    {
        bool windowCreated = ImGui::Begin(APP_NAME);

        ShowEventsOverlay(&windowCreated);
        ShowControls(&windowCreated);

        if (ImGui::BeginMenu("File")) {
            if (ImGui::MenuItem("New")) { }
            if (ImGui::MenuItem("Open", "Ctrl+O")) { }
            if (ImGui::MenuItem("Save", "Ctrl+S")) { }
            if (ImGui::MenuItem("Save As..")) { }
            if (ImGui::MenuItem("Export")) { }
            ImGui::Separator();
                if (ImGui::BeginMenu("Options")) {
                    ImGui::DragFloat3("Object position", objectsParams[0].position, 0.1f);
                    ImGui::DragFloat3("Object rotation", objectsParams[0].rotation, 0.1f);
                    ImGui::DragFloat3("Object scale", objectsParams[0].scale, 0.1f);
                    ImGui::DragFloat3("Camera position", cameraParams.cameraPos, 0.1f);
                    ImGui::DragFloat3("Camera target", cameraParams.cameraTarget, 0.1f);
                    ImGui::DragFloat3("Up vector", cameraParams.upVector,
                        0.1f);
                    ImGui::Checkbox("Look mode", &cameraParams.lookMode);
                    ImGui::Checkbox("Perspective mode", &cameraParams.perspectiveMode);
                    if (cameraParams.perspectiveMode) {
                        if (cameraParams.cameraPos[0] == 45 && cameraParams.cameraPos[2] == 45) {
                            cameraParams.cameraPos[0] = 0;
                            cameraParams.cameraPos[2] = 1;
                        }
                        ImGui::SliderFloat("Field of View", &cameraParams.fieldOfView, 0.0f, 1000.0f);
                        ImGui::InputFloat("Field of View", &cameraParams.fieldOfView, 0.01f);
                        ImGui::SliderFloat("Near Clipping Plane", &cameraParams.nearClippingPlane, 0.0f, 1.0f);
                        ImGui::InputFloat("Near Clipping Plane", &cameraParams.nearClippingPlane, 0.01f);
                        ImGui::SliderFloat("Far Clipping Plane", &cameraParams.farClippingPlane, 0.0f, 400.0f);
                        ImGui::InputFloat("Far Clipping Plane", &cameraParams.farClippingPlane, 0.01f);
                    } else {
                        if (cameraParams.cameraPos[0] == 0 && cameraParams.cameraPos[2] == 1) {
                            cameraParams.cameraPos[0] = 45;
                            cameraParams.cameraPos[2] = 45;
                        }
                        ImGui::InputFloat("Ortho Left", &cameraParams.orthoSize, 0.25f);
                    }
                    ImGui::EndMenu();
                }
            if (ImGui::MenuItem("Quit", "Alt+F4")) { }
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Animation")) {
            ImGui::SeparatorText("Animation controls");
            if (ImGui::Button("Play animation")) {
                animationParams.play = true;
            }
            if (ImGui::Button("Stop animation")) {
                animationParams.play = false;
            }
            ImGui::DragFloat("Play (ms)", &animationControlParams[0].play_ms);
            ImGui::DragFloat("Start (ms)", &animationControlParams[0].start_ms);
            ImGui::DragFloat("End (ms)", &animationControlParams[0].end_ms);

            ImGui::SeparatorText("Object properties");
            ImGui::DragFloat3("Object position", objectsAnimationParams[0].position, 0.1f);
            ImGui::DragFloat3("Object rotation", objectsAnimationParams[0].rotation, 0.1f);
            ImGui::DragFloat3("Object scale", objectsAnimationParams[0].scale, 0.1f);

            ImGui::SeparatorText("Easing equetions");
            ImGui::Checkbox("Use Easing Function", &animationControlParams[0].useEasingFunction);
            if (ImGui::TreeNode("List of easing equetions")) {
                for (size_t i = 0; i < animationControlParams[0].easingEquations.size(); i++) {
                    if (ImGui::Selectable(animationControlParams[0].easingEquations[i].c_str(),
                        i == animationControlParams[0].selectedEasingEquation)) {
                        animationControlParams[0].selectedEasingEquation = i;
                    }
                }
                ImGui::TreePop();
            }
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Debug")) {
            if (ImGui::TreeNode("Pipeline History")) {
                for (size_t i = 0; i < drawParams.pipelineHistorySize; i++) {
                    std::string name = "Graphics Pipeline " + std::to_string(i);
                    if (ImGui::Selectable(name.c_str(), i == selectedPipelineIndex)) {
                        selectedPipelineIndex = i;
                    }
                }
                ImGui::TreePop();
            }
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Create")) {
            if (ImGui::BeginMenu("Objects")) {
                ImGui::DragInt("alpha (1 - 10000)", &objectConstructionParams.alphaPercentage, 10.0f, 1, 10000, "%d%%");
                if (ImGui::Button("Construct")) {
                    drawParams.constructSelectedObject = true;
                }
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("Effects")) {
                ImGui::CheckboxFlags("Highlight selected pixels", &effectsParams.highlightSelectedPixels, 1);
                ImGui::Separator();
                for(uint16_t maskIndex = 0; maskIndex < EFFECTS_COUNT; maskIndex++) {
                    uint16_t vecIndex = maskIndex / 4;
                    uint16_t vecMaskIndex = maskIndex - vecIndex * 4;
                    std::string effectCheckboxName = "Effect: " + std::to_string(maskIndex + 1);
                    ImGui::CheckboxFlags(effectCheckboxName.c_str(), &effectsParams.enabledEffects[vecIndex][vecMaskIndex], 1);
                }
                ImGui::DragFloat("Height Range", &effectsParams.heightRange, 0.01f, 0.1, 1.0f);
                ImGui::DragFloat("Noise Scale", &effectsParams.noiseScale, 1.0f, 0.01f, 1000.0f);
                ImGui::DragFloat("Distortion", &effectsParams.distortionModifier, 0.001f, 0.0001, 1.0f);
                // TODO format to 5 numbers in fraction part
                ImGui::DragFloat("Parallax Height Scale", &effectsParams.parallaxHeightScale, 0.00001f, 0.00001, 0.001f);
                ImGui::DragFloat("Flickering Light", &effectsParams.amplifyFlickeringLight, 0.01f, 0.0001, 1.0f);
                ImGui::DragFloat("Highlight", &effectsParams.amplifyHighlight, 0.01f, 0.0001, 1.0f);
                ImGui::EndMenu();
            }

            ImGui::SeparatorText("Mask Options");
            ImGui::DragInt("Choose mask to edit", &mouseControlParams.maskIndex, 1, 0, EFFECTS_COUNT);
            if (ImGui::Button("Clear")) {
                drawParams.clearSelectedMask = true;
            }

            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Inpainting")) {
            ImGui::Checkbox("Enable", &inpaintingParams.enableInpainting);
            ImGui::DragInt("Patch size", &inpaintingParams.patchSize, 1, 5, 200);
            ImGui::EndMenu();
        }
        ImGui::End();
    }

    ImGui::Render();
}

void Gui::renderDrawData(VkCommandBuffer& commandBuffer)
{
    ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), commandBuffer);
}

void Gui::createGraphicsObjectParams(uint16_t objIndex)
{
    ObjectParams objectParams = getObjectParams();
    ObjectParams objectAnimationParams = getObjectParams();
    AnimationParams animationParams = getAnimationParams();
    objectParams.index = objIndex;
    objectAnimationParams.index = objIndex;
    animationParams.objIndex = objIndex;
    objectsParams.push_back(objectParams);
    objectsAnimationParams.push_back(objectAnimationParams);
    animationControlParams.push_back(animationParams);
}

void Gui::destroy()
{
    ImGui_ImplVulkan_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}

ObjectParams Gui::getObjectParams()
{
    return objectParams;
}

AnimationParams Gui::getAnimationParams()
{
    return animationParams;
}

CameraParams& Gui::getCameraParams()
{
    return cameraParams;
}

EffectParams& Gui::getEffectParams()
{
    return effectsParams;
}

ObjectConstructionParams& Gui::getObjectConstructionParams()
{
    return objectConstructionParams;
}

MouseControlParams& Gui::getMouseControlParams()
{
    return mouseControlParams;
}

InpaintingParams& Gui::getInpaintingParams()
{
    return inpaintingParams;
}

size_t Gui::getSelectedPipelineIndex() const
{
    return selectedPipelineIndex;
}

void Gui::selectPipelineindex(const size_t pipelineIndex)
{
    selectedPipelineIndex = pipelineIndex;
}

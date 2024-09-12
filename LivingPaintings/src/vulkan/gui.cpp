// This is a personal academic project. Dear PVS-Studio, please check it.

// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com

#include "gui.h"

using namespace std;

const float PAD = 10.0f;

const ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoMove;

void Gui::init(VkInstance& instance, Device& _device, VkCommandPool& commandPool, RenderPass& renderPass, Swapchain& swapChain, VkDescriptorPool& descriptorPool, GLFWwindow* pWindow)
{
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
    initInfo.ImageCount = Constants::MAX_FRAMES_IN_FLIGHT;
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

void Gui::ShowEventsOverlay(bool* p_open)
{
    const ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImVec2 work_pos = viewport->WorkPos;
    ImVec2 work_size = viewport->WorkSize;
    ImVec2 window_pos, window_pos_pivot;
    window_pos.x = work_pos.x + PAD;
    window_pos.y = work_pos.y + work_size.y - PAD;
    window_pos_pivot.x = 0.0f;
    window_pos_pivot.y = 1.0f;
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
        if (p_open && ImGui::MenuItem("Close"))
            *p_open = false;
        ImGui::EndPopup();
    }

    ImGui::End();
}

void Gui::ShowControls(bool* p_open)
{
    const ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImVec2 work_pos = viewport->WorkPos;
    ImVec2 work_size = viewport->WorkSize;
    ImVec2 window_pos, window_pos_pivot;
    window_pos.x = work_pos.x + PAD;
    window_pos.y = work_pos.y + PAD;
    window_pos_pivot.x = 0.0f;
    window_pos_pivot.y = 0.0f;
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
        if (p_open && ImGui::MenuItem("Close"))
            *p_open = false;
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
        bool windowCreated = ImGui::Begin("Living Paintings");

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
                ImGui::DragFloat3("Object position", objectParams.position, 0.1f);
                ImGui::DragFloat3("Object rotation", objectParams.rotation, 0.1f);
                ImGui::DragFloat3("Object scale", objectParams.scale, 0.1f);
                ImGui::DragFloat3("Camera position", cameraParams.cameraPos, 0.1f);
                ImGui::DragFloat3("Camera target", cameraParams.cameraTarget, 0.1f);
                ImGui::DragFloat3("Up vector", cameraParams.upVector, 0.1f);
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
            ImGui::DragFloat("Play (ms)", &animationParams.play_ms);
            ImGui::DragFloat("Start (ms)", &animationParams.start_ms);
            ImGui::DragFloat("End (ms)", &animationParams.end_ms);

            ImGui::SeparatorText("Object properties");
            ImGui::DragFloat3("Object position", animatedObjectParams.position, 0.1f);
            ImGui::DragFloat3("Object rotation", animatedObjectParams.rotation, 0.1f);
            ImGui::DragFloat3("Object scale", animatedObjectParams.scale, 0.1f);

            ImGui::SeparatorText("Easing equetions");
            ImGui::Checkbox("Use Easing Function", &animationParams.useEasingFunction);
            if (ImGui::TreeNode("List of easing equetions")) {
                for (int i = 0; i < (int)animationParams.easingEquations.size(); i++) {
                    if (ImGui::Selectable(animationParams.easingEquations[i].c_str(), i == animationParams.selectedEasingEquation)) {
                        animationParams.selectedEasingEquation = i;
                    }
                }
                ImGui::TreePop();
            }
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Debug")) {
            if (ImGui::TreeNode("Pipeline History")) {
                for (int i = 0; i < drawParams.pipelineHistorySize; i++) {
                    std::string name = "Graphics Pipeline " + std::to_string(i);
                    if (ImGui::Selectable(name.c_str(), i == selectedPipelineIndex)) {
                        selectedPipelineIndex = i;
                    }
                }
                ImGui::TreePop();
            }
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Construct Object")) {
            ImGui::DragInt("alpha (1 - 10000)", &objectConstructionParams.alphaPercentage, 10.0f, 1, 10000, "%d%%");
            if (ImGui::Button("Construct")) {
                drawParams.constructSelectedObject = true;
            }
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

void Gui::destroy()
{
    ImGui_ImplVulkan_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}

ObjectParams& Gui::getObjectParams()
{
    return objectParams;
}

ObjectParams& Gui::getAnimatedObjectParams()
{
    return animatedObjectParams;
}

CameraParams& Gui::getCameraParams()
{
    return cameraParams;
}

AnimationParams& Gui::getAnimationParams()
{
    return animationParams;
}

ObjectConstructionParams& Gui::getObjectConstructionParams()
{
    return objectConstructionParams;
}

size_t Gui::getSelectedPipelineIndex()
{
    return selectedPipelineIndex;
}

void Gui::selectPipelineindex(const size_t pipelineIndex)
{
    selectedPipelineIndex = pipelineIndex;
}

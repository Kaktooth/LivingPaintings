// This is a personal academic project. Dear PVS-Studio, please check it.

// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com

#include "gui.h"
#include <stdexcept>

using namespace std;

void Gui::init(VkInstance& instance, Device& _device, VkCommandPool& commandPool, VkRenderPass& renderPass, Swapchain& swapChain, VkDescriptorPool& descriptorPool, GLFWwindow* window)
{
    Queue& graphicsQueue = _device.getGraphicsQueue();

    this->device = _device.get();
    this->commandPool = commandPool;

    ImGui::CreateContext();

    ImGui_ImplGlfw_InitForVulkan(window, true);

    ImGui_ImplVulkan_InitInfo initInfo {};
    initInfo.Instance = instance;
    initInfo.PhysicalDevice = _device.getPhysicalDevice();
    initInfo.Device = device;
    initInfo.QueueFamily = _device.getQueueFamily().indicies.graphicsFamily.value();
    initInfo.Queue = graphicsQueue.get();
    initInfo.DescriptorPool = descriptorPool;
    initInfo.MinImageCount = swapChain.getMinImageCount();
    initInfo.ImageCount = Constants::MAX_FRAMES_IN_FLIGHT;
    initInfo.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
    initInfo.ColorAttachmentFormat = swapChain.getImageFormat();

    ImGui_ImplVulkan_Init(&initInfo, renderPass);

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

void Gui::draw()
{
    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
    {
        ImGui::Begin("DeepPicture");
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
            if (ImGui::TreeNode("list of easing equetions")) {
                for (int i = 0; i < (int)animationParams.easingEquations.size(); i++) {
                    if (ImGui::Selectable(animationParams.easingEquations[i].c_str(), i == animationParams.selectedEasingEquation)) {
                        animationParams.selectedEasingEquation = i;
                    }
                }
                ImGui::TreePop();
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

ObjectParams Gui::getObjectParams()
{
    return objectParams;
}

ObjectParams Gui::getAnimatedObjectParams()
{
    return animatedObjectParams;
}

CameraParams Gui::getCameraParams()
{
    return cameraParams;
}

AnimationParams Gui::getAnimationParams()
{
    return animationParams;
}
